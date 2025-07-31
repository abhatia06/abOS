#pragma once

#include "../stdint.h"
#include "fs.h"

extern inode_t current_dir_inode;
extern superblock_t superblock;
extern int block_buffer[FS_BLOCK];
extern int sector_buffer[FS_SECTOR];
extern inode_t root_inode;

void rw_sectors(uint32_t sectors, uint32_t starting_sector, uint32_t address, int readwrite);
bool load_file(inode_t* inode, uint32_t address);
bool save_file(inode_t* node, uint32_t address);
inode_t get_inode_in_dir(inode_t current_dir, char* file);
inode_t get_inode(char* path);
inode_t get_parent_inode(char* path);
void set_inode_bitmap(uint32_t bit, bool set);
void set_data_bitmap(uint32_t bit, bool set);
char* get_name_path(char* path);
void update_superblock();
void update_inode(inode_t inode);
uint32_t find_free_bit(uint32_t start_block, uint32_t length_blocks);
inode_t create_file(char* filepath);
bool print_dir(char* path);
bool delete_file(inode_t* inode);
