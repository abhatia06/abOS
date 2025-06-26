#pragma once

#define FRAME_SIZE	4096 // generally paging has each frame be 4KB
#define BITMAP_DIVISON 	8    // Memory is already divided into partitions of 8 bits (or 1 byte), so this is easier

uint32_t *memory_map = 0;
