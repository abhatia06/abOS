#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "kernel/fs/fs.h"

// Here's the basic plan:
// Somehow I need to either grab/generate an img file that is 1.44MB large, just like our actual disk image
// Initialize/build superblock
// Initialize/build bootblock
// Initialize/build bitmaps (inode & data)
// Initialize/build inode block
// Initialize/build data blocks

typedef struct {
        char path[60];
        uint32_t size;
        FILE *file;
} file_pointer;

file_pointer files[] = {
        {"build/boot.bin", 0, NULL},
        {"build/bootstage2.bin", 0, NULL},
        {"build/prekernel.bin", 0, NULL},
        {"build/kernel.bin", 0, NULL},
};

superblock_t superblock = (superblock_t){0};
uint32_t disk_size = 512*2880;  // keeping it to be 1.44MB for right now
uint32_t num_files = sizeof(files)/sizeof(files[0]);    // total bytes of array/bytes per element = size of array

bool write_boot_block() {
        return false;
}

bool write_superblock() {
        return false;
}


int main() {
	
	// correct file attributes 
        for(uint32_t i = 0; i < num_files; i++) {
                files[i].file = fopen(files[i].path, "rb");     //open file (get pointer)

                fseek(files[i].file, 0, SEEK_END);              //set pointer to end of the file
                files[i].size = ftell(files[i].file);           //get current pos of pointer (which is the same as the size then)
		fclose(files[i].file);
        }
	
        return 0;
}
