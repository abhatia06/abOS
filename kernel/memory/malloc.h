#pragma once

#include "../stdint.h"
#include "../global_addresses.h"

#define PAGE_SIZE 4096
#define NULL0 0

typedef struct malloc_node {
        uint32_t size;
        bool free;
        struct malloc_node* next;
        struct malloc_node* prev;
        void* address;                  // This might seem weird. Most malloc implementations don't have this, but I do because
                                        // I'm a bad coder lol
} malloc_node_t;

// We will be using a linked list approach for malloc
extern malloc_node_t* malloc_head;
extern uint32_t malloc_virt_address;
extern uint32_t malloc_phys_address;
extern uint32_t total_malloc_pages;
extern uint32_t malloc_start;
extern uint32_t heap_end;

void* malloc_init();
void* malloc_more_pages(uint32_t size);
void* calloc(uint32_t size);
void* split_blocks(malloc_node_t* node, uint32_t size);
void* malloc_next(uint32_t size);
void merge_free_blocks();
void malloc_free(void* ptr);
