#include "virtual_memory.cuh"
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdlib.h>
#include <stdio.h>
__device__ int vm_LRU(VirtualMemory *vm);
__device__ inline void save2memory(VirtualMemory *vm, int index, uchar value);
__device__ void swap_out(VirtualMemory *vm, int page_num, int frame_num);
__device__ void swap_in(VirtualMemory *vm, int mem_num, int origin_num, int disk_num);

//This variable save the LRU index.
__device__ static int index;

__device__ void init_invert_page_table(VirtualMemory *vm) {

    for (int i = 0; i < vm->PAGE_ENTRIES; i++) {

        /* Use the first number as the invalid-valid checker
         * If any data is saved to the memory and changed the page table,
         * The MSB will not be 1 and become valid.
         */
        // 0-1023: Record the frame number in the physical memory and
        vm->invert_page_table[i] = 0x80000000; // invalid := MSB is 1
        // Page Entries: 1024 entries in the memory and the page table.
        // 1024-2047: Record the frame number in the secondary memory
        vm->invert_page_table[i + vm->PAGE_ENTRIES] = 0x80000000;
        // 2048-3071: Count the used numbers to manipulate LRU algorithm.
        vm -> invert_page_table[i + 2 * vm->PAGE_ENTRIES] = 0;
        /* 3072-4096: Modified Bit: 0 -> Not Modified, can access it directly from the memory
         * 1 -> Modified, should raise the page fault and let the OS to get it from the disk.
         */
        vm -> invert_page_table[i + 3 * vm->PAGE_ENTRIES] = 0;
    }
}

__device__ void vm_init(VirtualMemory *vm, uchar *buffer, uchar *storage,
                        u32 *invert_page_table, int *pagefault_num_ptr,
                        int PAGESIZE, int INVERT_PAGE_TABLE_SIZE,
                        int PHYSICAL_MEM_SIZE, int STORAGE_SIZE,
                        int PAGE_ENTRIES) {
    // init variables
    vm->buffer = buffer;
    vm->storage = storage;
    vm->invert_page_table = invert_page_table;
    vm->pagefault_num_ptr = pagefault_num_ptr;

    // init constants
    vm->PAGESIZE = PAGESIZE;
    vm->INVERT_PAGE_TABLE_SIZE = INVERT_PAGE_TABLE_SIZE;
    vm->PHYSICAL_MEM_SIZE = PHYSICAL_MEM_SIZE;
    vm->STORAGE_SIZE = STORAGE_SIZE;
    vm->PAGE_ENTRIES = PAGE_ENTRIES;

    // before first vm_write or vm_read
    // Initialize the page table.
    init_invert_page_table(vm);
}



__device__ void vm_write(VirtualMemory *vm, u32 addr, uchar value) {
    /* Complete vm_write function to write value into data buffer */
    u32 page_number = addr / 32;
    // This remainder's value is the first goal of the memory paging.
    u32 remainder = page_number % 1024;
//    printf("%d",int(remainder));
    // Only raise the page fault and do the swap out at the first byte of the page.
    if(addr % 32 == 0){
        // Two situations:
        // 1. Empty Page Table, Empty memory
        if(vm->invert_page_table[remainder] == 0x80000000){
            *(vm->pagefault_num_ptr) += 1;
            // Update the page table:
            vm->invert_page_table[remainder] = remainder;
            vm->invert_page_table[remainder + vm->PAGE_ENTRIES] = page_number;
            //vm->invert_page_table[remainder + 2 * vm->PAGE_ENTRIES] = addr;
            vm->invert_page_table[remainder + 2 * vm->PAGE_ENTRIES] = page_number;

            save2memory(vm, 32 * vm->invert_page_table[remainder], value);
            index = remainder;

        }
        else{
            // 2. The page table (memory) is full.
            *(vm->pagefault_num_ptr) += 1;
            // Use LRU to find the position we want to replace.
//            printf("PAGE:%d", page_number);

            index = vm_LRU(vm);
            // Change the page number in the page table.
            // Do the swap out.
//            printf("INDEX: %d", index);

            swap_out(vm, vm->invert_page_table[index], vm->invert_page_table[index + vm->PAGE_ENTRIES]);
            // Then, update the page table.
            vm->invert_page_table[index] = remainder;
            vm->invert_page_table[index + vm->PAGE_ENTRIES] = page_number;
            vm->invert_page_table[index + 2 * vm->PAGE_ENTRIES] = page_number;
            // Set the modified bit to show that it has been modified.
            vm->invert_page_table[index + 3 * vm->PAGE_ENTRIES] = 1;
            save2memory(vm, 32 * vm->invert_page_table[index], value);
        }

    }
    else{
        // Deal with the bytes in the page:
        save2memory(vm, (32 * vm->invert_page_table[index]) + (addr % 32), value);
    }

    // For the other
    // Overwrite, 如果已存在自己到最常用，其它的往后掉。
    // New: 如果不存在，则放到最后一位，常用度最高。
    // 被访问：放到最后一位，常用度最高。
}

// This function will read data from the input buffer.
__device__ uchar vm_read(VirtualMemory *vm, u32 addr, int & max_count) {
    u32 page_number = addr / 32;
    u32 remainder = page_number % 1024;

    for (int i = 0; i < vm->PAGE_ENTRIES; ++i) {
        // Situation 1: Get the data from the memory directly.
        if (vm->invert_page_table[i + vm->PAGE_ENTRIES] == page_number) {
            index = i;
            printf("INDEX: %d\n", index);
            vm->invert_page_table[remainder + 2 * vm->PAGE_ENTRIES] = ++max_count;
            return vm->buffer[32 * index + (addr % 32)];
        }
    }


    //Situation 2: The data we need is not in the memory. We should get the result from the disk by swapping in.
    printf("SWAP");
    index = vm_LRU(vm);
    printf("INDEX: %d", index);
    //Update the page fault information
    (*vm->pagefault_num_ptr) += 1;

    //Swap in the data
    int origin_number = vm->invert_page_table[index + vm->PAGE_ENTRIES];
    //Save data to the disk[origin_number], and load disk[page_number] to mem[index]
    swap_in(vm, vm->invert_page_table[index],page_number, origin_number);
    // Update the page table
    vm->invert_page_table[index] = index;
    vm->invert_page_table[index + vm->PAGE_ENTRIES] = page_number;
    vm->invert_page_table[index + 2 * vm->PAGE_ENTRIES] = max_count + 1;
    return vm->buffer[32 * vm->invert_page_table[index] + (addr % 32)];

    // Look through all over the page table to find whether the data we need is in the memory.

}


__device__ void vm_snapshot(VirtualMemory *vm, uchar *results, int offset,
                            int input_size, int & max_count) {
    /* Complete snapshot function togther with vm_read to load elements from data
     * to result buffer */
    for (int i = 0; i < input_size; ++i){
        int value = vm_read(vm, i, max_count);
        results[offset + i] = value;
    }
}

// This function return the appropriate index to replace.
__device__ int vm_LRU(VirtualMemory *vm){
    // This two integers help to find the least used entity.
    int min_index = 0;
    int min = INT_MAX;
    int temp = 0;
    for (int i = 0; i < vm->PAGE_ENTRIES; ++i) {
        temp = vm->invert_page_table[i + 2 * vm->PAGE_ENTRIES];
        if (temp == 0) {
            return i;
        }
        if (temp <= min){
            min = temp;
            min_index = i;
        }
    }
    return min_index;
}

__device__ inline void save2memory(VirtualMemory *vm, int index, uchar value){
    vm->buffer[index] = value;
}

__device__ void swap_out(VirtualMemory *vm, int page_num, int frame_num){
    for (int i = 0; i < vm->PAGESIZE; ++i){
        vm->storage[32 * frame_num + i] = vm->buffer[32 * page_num + i];
        vm->buffer[32 * page_num + i] = NULL;
    }

}
__device__  void swap_in(VirtualMemory *vm, int mem_num, int disk_num, int origin_num){
    for (int i = 0; i < vm->PAGESIZE; ++i){
        vm->storage[32 * origin_num + i] = vm->buffer[32 * mem_num + i];
    }
    for (int i = 0; i < vm->PAGESIZE; ++i) {
        vm->buffer[32 * mem_num + i] = vm->storage[32 * disk_num + i];
    }

}