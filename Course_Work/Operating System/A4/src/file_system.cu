#include "file_system.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#define BYTE_TO_BINARY_PATTERN "0b%c%c%c%c%c%c%c\n"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')


__device__ __managed__ u32 gtime = 0;
//We have a 128 bytes TempArray for sorting.
__device__ __managed__ uchar tempArray[128];
__device__ void fs_gsys(FileSystem *fs, int op, char *s);

__device__ void init_volume(FileSystem *fs) {
    for (int i = 0; i < fs->SUPERBLOCK_SIZE; ++i){
        fs->volume[i] = 0b11111111;
    }
    for (int i = fs->SUPERBLOCK_SIZE; i < fs->STORAGE_SIZE;++i){
        fs->volume[i] = 0;
    }

}

// This function translate 4 u8 chars to u32.
__device__ inline u32 u8tou32(FileSystem * fs, u32 i){
    // 1st byte + 2nd byte * 2^8 + 3rd byte * 2^16 + 4th byte * 2^24
    return u32(fs->volume[i]) + u32(fs->volume[i + 1] * 256) + u32(fs->volume[i + 2] * 65536)
    + u32(fs->volume[i + 3] * 16777216);
}

// This function translate a u32 number to 4 u8 chars.
__device__ void u32tou8(FileSystem * fs, u32 index, u32 value){
    fs->volume[index] = value & 0x000000FFu;
    fs->volume[index+1] = (value & 0x0000FF00u) >> 8u;
    fs->volume[index+2] = (value & 0x00FF0000u) >> 16u;
    fs->volume[index+3] = (value & 0xFF000000u) >> 24u;
}

// This function update the super block (bit map).
//Start: First byte address of the file.
__device__ void updateSuperBlock(FileSystem * fs, u32 start, u32 size, int mode = 0){
    u32 block_index = (start - fs->FILE_BASE_ADDRESS)  / 32 + mode; // Block number of this file (0-1023)
    u32 super_index = block_index / 8; //Which entry this block belongs to in the super block.
    //Offset to find it in bit-map (0 - 7). 0: First bit, 7: Last bit
    u32 super_offset = block_index % 8;
    //Number of blocks this file need.
    u32 total_block;
    if(size % 32 == 0) total_block = size / 32;
    else total_block = (size / 32 + 1);
    //After the initialization, we update the Bit-Map
    //Mode == 1 -> Free the bit-map (modify 0 -> 1)
    if(mode == 1){
        for(u32 k = super_index + 1; k < fs->SUPERBLOCK_SIZE; ++k){
            fs->volume[k] = 0b11111111;
        }
        //Then change the information of super_index:
        switch(super_offset){
            case 0:
                fs->volume[super_index] = 0b11111111;
                break;
            case 1:
                fs->volume[super_index] = 0b01111111;
                break;
            case 2:
                fs->volume[super_index] = 0b00111111;
                break;
            case 3:
                fs->volume[super_index] = 0b00011111;
                break;
            case 4:
                fs->volume[super_index] = 0b00001111;
                break;
            case 5:
                fs->volume[super_index] = 0b00000111;
                break;
            case 6:
                fs->volume[super_index] = 0b00000011;
                break;
            case 7:
                fs->volume[super_index] = 0b00000001;
                break;
        }
        return;
    }
    //Mode == 0 -> Take up the bit-map (modify 1 -> 0)
    //Case 1: Just modify one super_index
    if(super_offset + total_block <= 8){
        switch(super_offset){
            case 0:
                switch(total_block){
                    case 1:
                        //Offset = 0 and only need 1 block. Change the first block to 0
                        fs->volume[super_index] = fs->volume[super_index] & 0b01111111u;
                        break;
                    case 2:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00111111u;
                        break;
                    case 3:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00011111u;
                        break;
                    case 4:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00001111u;
                        break;
                    case 5:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00000111u;
                        break;
                    case 6:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00000011u;
                        break;
                    case 7:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00000001u;
                        break;
                    case 8:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00000000u;
                        break;
                }
                break;
            case 1:
                switch(total_block){
                    case 1:
                        //Offset = 0 and only need 1 block. Change the first block to 0
                        fs->volume[super_index] = fs->volume[super_index] & 0b10111111u;
                        break;
                    case 2:
                        fs->volume[super_index] = fs->volume[super_index] & 0b10011111u;
                        break;
                    case 3:
                        fs->volume[super_index] = fs->volume[super_index] & 0b10001111u;
                        break;
                    case 4:
                        fs->volume[super_index] = fs->volume[super_index] & 0b10000111u;
                        break;
                    case 5:
                        fs->volume[super_index] = fs->volume[super_index] & 0b10000011u;
                        break;
                    case 6:
                        fs->volume[super_index] = fs->volume[super_index] & 0b10000001u;
                        break;
                    case 7:
                        fs->volume[super_index] = fs->volume[super_index] & 0b10000000u;
                        break;
                }
                break;
            case 2:
                switch(total_block){
                    case 1:
                        //Offset = 0 and only need 1 block. Change the first block to 0
                        fs->volume[super_index] = fs->volume[super_index] & 0b11011111u;
                        break;
                    case 2:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11001111u;
                        break;
                    case 3:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11000111u;
                        break;
                    case 4:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11000011u;
                        break;
                    case 5:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11000001u;
                        break;
                    case 6:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11000000u;
                        break;
                }
                break;
            case 3:
                switch(total_block){
                    case 1:
                        //Offset = 0 and only need 1 block. Change the first block to 0
                        fs->volume[super_index] = fs->volume[super_index] & 0b11101111u;
                        break;
                    case 2:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11100111u;
                        break;
                    case 3:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11100011u;
                        break;
                    case 4:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11100001u;
                        break;
                    case 5:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11100000u;
                        break;
                }
                break;
            case 4:
                switch(total_block){
                    case 1:
                        //Offset = 0 and only need 1 block. Change the first block to 0
                        fs->volume[super_index] = fs->volume[super_index] & 0b11110111u;
                        break;
                    case 2:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11110011u;
                        break;
                    case 3:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11110001u;
                        break;
                    case 4:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11110000u;
                        break;
                }
                break;
            case 5:
                switch(total_block){
                    case 1:
                        //Offset = 0 and only need 1 block. Change the first block to 0
                        fs->volume[super_index] = fs->volume[super_index] & 0b11111011u;
                        break;
                    case 2:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11111001u;
                        break;
                    case 3:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11111000u;
                        break;
                }
                break;
            case 6:
                switch(total_block){
                    case 1:
                        //Offset = 0 and only need 1 block. Change the first block to 0
                        fs->volume[super_index] = fs->volume[super_index] & 0b11111101u;
                        break;
                    case 2:
                        fs->volume[super_index] = fs->volume[super_index] & 0b11111100u;
                        break;
                }
                break;
            case 7:
                fs->volume[super_index] = fs->volume[super_index] & 0b11111110u;
                break;
        }
    }
    else{
        int remain;
        remain = total_block - (8 - super_offset);
        //Case 2: Modify more super_index
        //Change the first index firstly.
        switch(super_offset){
            case 0:
                fs->volume[super_index] = fs->volume[super_index] & 0b00000000u;
                break;
            case 1:
                fs->volume[super_index] = fs->volume[super_index] & 0b10000000u;
                break;
            case 2:
                fs->volume[super_index] = fs->volume[super_index] & 0b11000000u;
                break;
            case 3:
                fs->volume[super_index] = fs->volume[super_index] & 0b11100000u;
                break;
            case 4:
                fs->volume[super_index] = fs->volume[super_index] & 0b11110000u;
                break;
            case 5:
                fs->volume[super_index] = fs->volume[super_index] & 0b11111000u;
                break;
            case 6:
                fs->volume[super_index] = fs->volume[super_index] & 0b11111100u;
                break;
            case 7:
                fs->volume[super_index] = fs->volume[super_index] & 0b11111110u;
                break;
        }
        //Change other bits.
        for(int i = 0; ;++i){
            if(remain / 8 == 0){
                switch(remain % 8){
                    case 1:
                        fs->volume[super_index] = fs->volume[super_index] & 0b01111111u;
                        break;
                    case 2:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00111111u;
                        break;
                    case 3:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00011111u;
                        break;
                    case 4:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00001111u;
                        break;
                    case 5:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00000111u;
                        break;
                    case 6:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00000011u;
                        break;
                    case 7:
                        fs->volume[super_index] = fs->volume[super_index] & 0b00000001u;
                        break;
                }
                break;
            }
            else{
                fs->volume[super_index + i] = 0b00000000u;
                remain /= 8;
            }
        }
    }

}

//Return a file_index if the file is found in FCB, otherwise return -1.
__device__ u32 find_file(FileSystem * fs, char * s){
    u32 file_index = -1;
    for (int i = fs->SUPERBLOCK_SIZE; i < fs->FILE_BASE_ADDRESS; i += 32){
        char subbuff[20];
        memcpy(subbuff, &(fs->volume[i]), 20);
        for(int j = 0; j < 20; ++j){
            if(subbuff[j] == s[j]){
                //If we find the file, we will reach \0
                if(s[j] == '\0'){
                    file_index = i;
                    return file_index;
                }
                else continue;
            }
            else{
                break;
            }
        }
    }
    return -1;
}

// Possibl Value for number: 0, 32, 64, ...
__device__ inline u32 get_index(FileSystem * fs, u32 number){
    return (number - fs->SUPERBLOCK_SIZE) / 32;
}
// This function check whether the index is already sorted out.
__device__ void update_used(FileSystem * fs, u32 index){
    u32 array_idx = index / 8; //array_idx: 0-127
    u32 offset = index % 8;  //offset: 0-7
    switch(offset){
        //Change the first bit to 0 (The first space is taken)
        case 0:
            tempArray[array_idx] = tempArray[array_idx] & 0b01111111u;
            break;
        case 1:
            tempArray[array_idx] = tempArray[array_idx] & 0b10111111u;
            break;
        case 2:
            tempArray[array_idx] = tempArray[array_idx] & 0b11011111u;
            break;
        case 3:
            tempArray[array_idx] = tempArray[array_idx] & 0b11101111u;
            break;
        case 4:
            tempArray[array_idx] = tempArray[array_idx] & 0b11110111u;
            break;
        case 5:
            tempArray[array_idx] = tempArray[array_idx] & 0b11111011u;
            break;
        case 6:
            tempArray[array_idx] = tempArray[array_idx] & 0b11111101u;
            break;
        case 7:
            tempArray[array_idx] = tempArray[array_idx] & 0b11111110u;
            break;
    }
}
// This function return whether the index is already be sorted out
__device__ bool is_used(FileSystem * fs, u32 index){
    u32 array_idx = index / 8; //array_idx: 0-127
    u32 offset = index % 8;  //offset: 0-7
    u32 check = tempArray[array_idx];
    switch(offset){
        //Check whether the first bit is 0 (taken)
        case 0:
            return (check >> 7u) & 0b00000001u;
        case 1:
            return (check >> 6u) & 0b00000001u;
        case 2:
            return (check >> 5u) & 0b00000001u;
        case 3:
            return (check >> 4u) & 0b00000001u;
        case 4:
            return (check >> 3u) & 0b00000001u;
        case 5:
            return (check >> 2u) & 0b00000001u;
        case 6:
            return (check >> 1u) & 0b00000001u;
        case 7:
            return check & 0b00000001u;
    }
}



//This function will compact all the following data to eliminate external fragmentation.
__device__ void compaction(FileSystem * fs, u32 empty_addr){
    //This addr is the address of the files that may be compacted.
    u32 compacted_addr;
    //Check every file that is saved "below" the removed file:
    for(int i = fs->SUPERBLOCK_SIZE + 20; i < fs->FILE_BASE_ADDRESS; i += 32){
        compacted_addr = u8tou32(fs, i);
        if(compacted_addr > empty_addr){
            //Move the file that will be compacted to the start address of the freed space
            u32 sizeofFile = u8tou32(fs, i+4);
            for(int k = 0; k < sizeofFile; ++k){
                fs->volume[empty_addr + k] = fs->volume[compacted_addr + k];
            }
            //Update the super block
            updateSuperBlock(fs, empty_addr, sizeofFile);
            //Update FCB
            //Update the compacted file's FCB by changing the start address to the freed space:
            u32tou8(fs, i, empty_addr);
            //Finally, we update the value of empty_addr to the file that we just compact:
            empty_addr = (empty_addr + sizeofFile - 1);
            //Adjust the value to fit into the block.
            if(empty_addr % 32 != 0){
                empty_addr = (empty_addr / 32 + 1) * 32;
            }
            else{
                continue;
            }
            break;
        }

    }
}
//This function will find a free block.
__device__ u32 get_Space(FileSystem * fs){
    int offset = 0;
    for(int i = 0; i < fs->SUPERBLOCK_SIZE; ++i){
        //Check from the larger scale (A cluster with 8 blocks)
        if(fs->volume[i] == 0) continue; //This 8 blocks is full.
        else if(fs->volume[i] == 255) {
            return i * 8;
        }

            //Then check which block is the last allocated space
        else{
            for(int k = 0x80; ; k /= 2){
                if(!(fs->volume[i] & k)){
                    ++offset;
                    continue;
                }
                else{
                    return i * 8 + offset;
                }
            }
        }
    }
}

__device__ void fs_init(FileSystem *fs, uchar *volume, int SUPERBLOCK_SIZE,
							int FCB_SIZE, int FCB_ENTRIES, int VOLUME_SIZE,
							int STORAGE_BLOCK_SIZE, int MAX_FILENAME_SIZE, 
							int MAX_FILE_NUM, int MAX_FILE_SIZE, int FILE_BASE_ADDRESS)
{
  // init variables
  // 0 ~~~ 4095: Super Block
  // 4096 ~~~ 36863: FCB Table
  // 36864 ~~~ 1085439; (1024KB 32768 Blocks): Contents of the file
  fs->volume = volume;

  // init constants
  fs->SUPERBLOCK_SIZE = SUPERBLOCK_SIZE;
  fs->FCB_SIZE = FCB_SIZE;
  fs->FCB_ENTRIES = FCB_ENTRIES;
  fs->STORAGE_SIZE = VOLUME_SIZE;
  fs->STORAGE_BLOCK_SIZE = STORAGE_BLOCK_SIZE;
  fs->MAX_FILENAME_SIZE = MAX_FILENAME_SIZE;
  fs->MAX_FILE_NUM = MAX_FILE_NUM;
  fs->MAX_FILE_SIZE = MAX_FILE_SIZE;
  fs->FILE_BASE_ADDRESS = FILE_BASE_ADDRESS;
  init_volume(fs);
}



__device__ u32 fs_open(FileSystem *fs, char *s, int op)
{
	/* Implement open operation here */
    u32 file_index;
    //First of all, initialize the file pointer.
	u32 fp;
    char * t = s;
	//Look through the FCB table, and check whether we have the file:
	file_index = find_file(fs, s);
    // If file_index = -1, we did not find the file.
    if(file_index == -1){
        //If we want to write the file, we create a new file.
        if(op == G_WRITE){
            //Find an empty FCB entry first: By checking an enrty without file name
            for(int i = fs->SUPERBLOCK_SIZE; i < fs->FILE_BASE_ADDRESS; i += 32){
                if(fs->volume[i] == '\0'){
                    file_index = i;
                    break;
                }
            }
            //Copy the file name to the empty FCB entry.
            for(int i = 0; i < 20; ++i){
                if(s[i] == '\0') {
                    break;
                }
                fs->volume[file_index + i] = s[i];
            }

            // Create a new file after the last allocated block:
            // Find the offset(it is block not byte) first
            int free_block = get_Space(fs);
            // Then we update the FCB:
            // The "2nd parameter" of an FCB entry is the start address of the file
            u32 start_addr = 32 * free_block + fs->FILE_BASE_ADDRESS;
            u32tou8(fs, file_index+20, start_addr);
            return file_index;
        }
        else if (op == G_READ){
            printf("NOT FOUND THE FILE %s", s);
            return -1;
        }
    }

    return file_index;

}

//This function will read data from the disk to the output buffer.
__device__ void fs_read(FileSystem *fs, uchar *output, u32 size, u32 fp)
{
    //We find the file size and the file's start location first:
    u32 start_addr = u8tou32(fs, fp + 20);
    u32 file_size = u8tou32(fs, fp + 24);

    if(fp == -1) {
        printf("NO SUCH FILE IN FS");
        return;
    }
    else if (file_size < size){
//        printf("Start Output the Result");
        for(u32 i = 0; i < file_size; ++i){
//            printf("%c", fs->volume[start_addr + i]);
            output[i] = fs->volume[start_addr + i];
        }
    }
    else{
//        printf("Start Output the Result");
        for(u32 i = 0; i < size; ++i){
            output[i] = fs->volume[start_addr + i];
        }
    }

}

__device__ u32 fs_write(FileSystem *fs, uchar* input, u32 size, u32 fp)
{
    //Update the start address and calculate the old file_size.
    //Update the time:
    u32tou8(fs, fp+28, ++gtime);
    u32 start_addr = u8tou32(fs, fp + 20);
    u32 file_size = u8tou32(fs, fp + 24);
    //Two Situations:
    //1. The allocated space is larger than the new input.
    if(file_size > size){
        //In this situation, we just need to override the old file
        for(u32 i = 0; i < size; ++i){
            fs->volume[start_addr + i] = input[i];
        }
        //Update FCB:
        //We calculate how many blocks it will need first
        int block = size / 32;
        if(size % 32 != 0) ++block;
        u32tou8(fs, fp+24, size);
        //Modify the bit-map --- change all the following bits to 1
        updateSuperBlock(fs, start_addr + 32 * block, 0, 1);
        //This operation will compact the space and modify the bit map.
        compaction(fs, start_addr + block * 32);
    }
    //2. The allocated space is smaller than the new input(new file is created)
    else if (file_size < size){
        //Free the taken space first
        for(u32 i = 0; i < file_size; ++i){
            fs->volume[start_addr + i] = 0;
        }
        //Compact the disk:
        compaction(fs, start_addr);
        //Find a new free space to write the new file
        start_addr = 32 * get_Space(fs) + fs->FILE_BASE_ADDRESS;
        //Write Data
        for(u32 i = 0; i < size; ++i){
            fs->volume[start_addr + i] = input[i];
        }
        //Update FCB:
        //We calculate how many blocks it will need first
        int block = size / 32;
        if(size % 32 != 0) ++block;
        u32tou8(fs, fp + 24, size);
        //Then update the start address
        u32tou8(fs, fp + 20, start_addr);
        //Update Super Block
        updateSuperBlock(fs, start_addr, size);
    }
    //File_size = size;
    else{
        //We just need to update the content in the disk.
        for(u32 i = 0; i < size; ++i){
            fs->volume[start_addr + i] = input[i];
        }
    }
}

__device__ void fs_gsys(FileSystem *fs, int op)
{
    //Refresh the tempArray;
    for(int i = 0; i < 128; ++i){
        tempArray[i] = 0b11111111;
    }

    /*
     * This array has 128 entries,
     * we use each entry to represent 8 files just like a bit-map.
     */
	uchar * sorted_file = tempArray;
	u32 index = 0xffffffff; //Should be 0 - 1023
	u32 size_num; // Current size value
	u32 time_num; //Current time value
    u32 max_num = 0; //Max Value (Can be size or time)
    u32 max_index = 0; //Index with the Max Value
    u32 max_addr = 0; //name address of the max value.
    bool flag = false;
    //According to the input, LS_D mode will output all files ordered by modifed time
    if(op == LS_D){
	    printf("===sort by modified time===\n");
        for(int i = fs->SUPERBLOCK_SIZE; i < fs->FILE_BASE_ADDRESS; i += 32){
            for(int j = fs->SUPERBLOCK_SIZE; j < fs->FILE_BASE_ADDRESS; j += 32){
                if(fs->volume[j] != 0){
                    time_num = u8tou32(fs, j + 28);
//                    printf("%d\n", time_num);
                    if(time_num <= max_num) continue;
                    index = get_index(fs, j);
                    //Check the used array to check whether this index is already be used.
                    if(is_used(fs, index)){
                        max_index = index;
                        max_addr = j;
                        max_num = time_num;
                        flag = true;
                    }
                    else continue;
                }
                // = 0: This FCB entry is empty
                else continue;
            }
           //If no information is updated: We are Done
           if(flag){
                //Update the used array
                update_used(fs, max_index);
                //Output the filename
                for(int k = 0; k < 20; ++k){
                    if(fs->volume[max_addr + k] != 0){
                        printf("%c", fs->volume[max_addr + k]);
                    }
                    else{
                        printf("\n");
                        break;
                    }
                }
                //Reset the maximum value;
                max_num = 0;
                flag = false;
            }
            else break;

        }
	}
	else if(op == LS_S){
        //LS_S mode will output all files ordered by file size
        printf("===sort by file size===\n");
        for(int i = fs->SUPERBLOCK_SIZE; i < fs->FILE_BASE_ADDRESS; i += 32){
            //This variable is the modified time of the biggest size file.
            u32 modified_time = 0;
            for(int j = fs->SUPERBLOCK_SIZE; j < fs->FILE_BASE_ADDRESS; j += 32){
                if(fs->volume[j] != 0){
                    size_num = u8tou32(fs, j + 24);
                    time_num = u8tou32(fs, j + 28);
                    if(size_num < max_num) continue;
                    else if(size_num > max_num){
                        index = get_index(fs, j);
                        //Check the used array to check whether this index is already be used.
                        if(is_used(fs, index)){
                            modified_time = time_num;
                            max_index = index;
                            max_addr = j;
                            max_num = size_num;
                            flag = true;
                        }
                        else continue;
                    }
                }
                    // = 0: This FCB entry is empty
                else continue;
            }
            if(flag){
                //Update the used array
                update_used(fs, max_index);
                //Output the filename
                for(int k = 0; k < 20; ++k){
                    if(fs->volume[max_addr + k] != 0){
                        printf("%c", fs->volume[max_addr + k]);
                    }
                    else{
                        printf("  %u\n", u8tou32(fs, max_addr + 24));
                        break;
                    }
                }
                //Reset the maximum value;
                max_num = 0;
                flag = false;
            }
            else break;


        }
	}
}

// This function will remove the file given the file name.
__device__ void fs_gsys(FileSystem *fs, int op, char *s)
{
    //Find the file's FCB first:
    u32 file_index = find_file(fs, s);
    //We free the space first:
    u32 start_addr = u8tou32(fs, file_index + 20);
    u32 file_size = u8tou32(fs, file_index + 24);
    for(int i = 0; i < file_size; ++i){
        fs->volume[start_addr + i] = 0;
    }
    //Modify the bit-map --- change all the following bits to 1
    int block = file_size / 32;
    if(file_size % 32 != 0) ++block;
    updateSuperBlock(fs, start_addr + 32 * block, 0, 1);
    //This operation will compact the space and modify the bit map.
    compaction(fs, start_addr + block * 32);
    //Then we compact the FCB table, which also override the deleted file's FCB:
    for(u32 i = file_index; i < fs->FILE_BASE_ADDRESS - 64; i += 32){
        for(u32 j = 0; j < 32; ++j){
            fs->volume[i + j] = fs->volume[i + 32 + j];
        }
    }

}
