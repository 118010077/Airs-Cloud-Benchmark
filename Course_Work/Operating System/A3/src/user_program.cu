#include "virtual_memory.cuh"
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>
__device__ void user_program(VirtualMemory *vm, uchar *input, uchar *results,
                             int input_size) {
    int max_count = 0;
    for (int i = 0; i < input_size; i++){
        vm_write(vm, i, input[i]);
    }

//    vm_read(vm, input_size);
    for(int i = 0; i < vm->PAGE_ENTRIES; ++ i){
        if(vm->invert_page_table[i + 2 * vm->PAGE_ENTRIES] > max_count){
            max_count = vm->invert_page_table[i + 2 * vm->PAGE_ENTRIES];
        }
    }

    for (int i = input_size - 1; i >= input_size - 32769; i--){
        int value = vm_read(vm, i, max_count);
    }

    // Read the data to the result buffer, and this buffer is exactly the output buffer.
    vm_snapshot(vm, results, 0, input_size, max_count);

}
