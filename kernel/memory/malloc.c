#include "malloc.h"
#include "../stdint.h"
#include "../global_addresses.h"
#include "virtual_memory_manager.h"
#include "physical_memory_manager.h"
#include "../util/string.h"

#define NULL 0

malloc_head = 0;
malloc_virt_address = 0x400000;         // start of malloc virtual address space for all processes
malloc_phys_address = 0;
total_malloc_pages = 0;
malloc_start = 0;
heap_end = 0;

void* malloc_init() {
        total_malloc_pages = 256;       // 1MB/4KB. We want to allocate 1MB worth of pages for the initialization
        malloc_phys_address = (uint32_t)allocate_blocks(total_malloc_pages);

        heap_end = malloc_virt_address + total_malloc_pages * PAGE_SIZE;        // keep track of the heap end
        virt = malloc_virt_address;     // localize it to malloc_init, as we want EVERY process to begin with virtual address
                                        // 4MB for their heap. Alternatively, we could just set virt_address = 4MB at the end of
                                        // each initialize, but whatever

        malloc_head = (malloc_block_t*)malloc_virt_address;

        for(uint32_t i = 0; i < total_malloc_pages; i++) {
                map_address(directory, malloc_phys_address + i*PAGE_SIZE, virt, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
                virt+=PAGE_SIZE;
        }


        malloc_head->size = (total_malloc_pages * PAGE_SIZE) - sizeof(malloc_block_t);  // malloc_block_t is basically header info
                                                                                        // each malloc'd piece of memory
        malloc_head->free = true;
        malloc_head->next = NULL;
        malloc_head->address = malloc_phys_address;     // again, might remove later..

        return (void*)malloc_phys_address;
}

// This function is effectively an sbrk(). It is only used when we absolutely NEED more memory
void* malloc_more_pages(uint32_t size) {
        total_malloc_pages = size/PAGE_SIZE;
        if(size % PAGE_SIZE > 0) {
                total_malloc_pages++;   // in case we want <4KB worth of bytes to be allocated in heap
        }

        malloc_phys_address = (uint32_t)allocate_blocks(total_malloc_pages);
        virt = heap_end;
        heap_end = virt + total_malloc_pages * PAGE_SIZE;       // update heap end to reflect change in memory

        malloc_node_t* temp = malloc_head;
        while(temp->next != NULL) {
                temp = temp->next;
        }

        malloc_node_t* new_node = (malloc_node_t*)virt;

        for(uint32_t i = 0; i < total_malloc_pages; i++) {
                map_address(directory, malloc_phys_address + i*PAGE_SIZE, virt, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
                virt += PAGE_SIZE;
        }

        // initialize the node
        new_node->size = (total_malloc_pages * PAGE_SIZE) - sizeof(malloc_node_t);
        new_node->free = true;
        new_node->next = NULL;
        new_node->address = malloc_phys_address;

        // Previous element in list should point to new node
        temp->next = new_node;

        return (void*)malloc_phys_address;
}

void* calloc_more_pages(uint32_t size) {
        void* ptr = malloc_more_pages(size);
        memset(ptr, 0, total_malloc_pages*PAGE_SIZE);   // calloc is just malloc but we 0 out the region
        return ptr;
}

void* split_blocks(uint32_t size) {
        return NULL;
}

void merge_free_blocks() {

}
