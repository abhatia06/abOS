#pragma once

#define FRAME_SIZE	4096 // generally paging has each frame be 4KB
#define BITMAP_DIVISON 	8    // Memory is already divided into partitions of 8 bits (or 1 byte), so this is easier
#define start_addr      0x7EE0000  // Close to the end of available memory for us

extern uint32_t *memory_map;
extern uint32_t max_blocks;
extern uint32_t used_blocks;

void set_block(uint32_t bit);
void unset_block(uint32_t bit);
int32_t find_free_blocks(uint32_t num_blocks);
void* allocate_blocks(uint32_t num_blocks);
void free_blocks(uint32_t num_blocks);
