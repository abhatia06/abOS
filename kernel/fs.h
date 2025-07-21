#pragma once

#include "stdint.h"
#define FS_BLOCK        4096
#define FS_SECTOR       512

#define file_type_file 0
#define file_type_dir 1

#define inodeStartAddr 3072     // 3KiB. We're going to be assuming superblock, and the two bitmaps = 1KiB each

// MACRO FUNCTIONS FOR CALCULATING STUFF
#define inode_block(i_number) ((i-number * sizeof(struct inode) / FS_BLOCK);    // finds which i-block your inode is in
#define inode_sector(i_number) (((inode_block(i_number) * sizeof(struct inode)) + inodeStartAddr) / FS_SECTOR)

typedef struct boot_record {
        uint8_t sector[8][FS_SECTOR];           // 8 sectors, each of 512 bytes, hence 2D array
} boot_record_t;

typedef struct inode {
        uint32_t i_number;                      // reserve inode 0, by the way
        uint32_t file_type;
        fs_time_t time_accessed;
        fs_time_t time_modified;
        fs_time_t time_created;
        uint32_t direct_pointers[3];            // 3 direct pointers
        uint32_t single_indirect_block;         // 1 single indirect pointer
        uint32_t beginning_block_ptr;
        uint8_t padding;
} inode_t;

typedef struct super_block {
        uint32_t size_of_data;
        uint32_t num_inodes;
        uint32_t num_inodes_bitmap;
        uint32_t inode_size;            // likely going to be 128 bytes?
        uint32_t num_datablocks;
        uint32_t num_datablocks_bitmap;
        uint32_t starting_inode_addr;
        uint32_t max_file_size;
        uint32_t root_i_number;         // using inode_block() and inode_sector() functions can give us the root inode
        uint8_t padding
        uint32_t first_free_inode_bit;  // stole this off queso fuego (originally was gonna make a separate function)
        uint32_t first_free_data_bit;
} super_block_t;                // CURRENT SIZE: 41 BYTES + however big root_inode is

// everything below is for the files & directories

/*
 * Each entry for a directory is going be housing stuff, so we need each directory to
 */
typedef struct dir_entry_t {
        uint32_t i_number;
        char name[20];
} dir_entry_t;

typedef struct time {
        uint8_t second;
        uint8_t minute;
        uint8_t hour;
        uint8_t day;
        uint8_t month;
        uint32_t year;
} fs_time_t;

extern uint32_t inode_bitmap;
extern uint32_t data_bitmap;
