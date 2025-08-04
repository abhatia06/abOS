#include "malloc.h"
#include "../stdint.h"
#include "../global_addresses.h"
#include "virtual_memory_manager.h"
#include "physical_memory_manager.h"
#include "../util/string.h"

malloc_node_t* malloc_head = 0;
uint32_t malloc_virt_address = 0;                // start of malloc virtual address space for all processes
uint32_t malloc_phys_address = 0;
uint32_t total_malloc_pages = 0;
uint32_t malloc_start = 0;
uint32_t heap_end = 0;

void* malloc_init() {
        total_malloc_pages = 256;       // 1MB/4KB. We want to allocate 1MB worth of pages for the initialization
        malloc_phys_address = (uint32_t)allocate_blocks(total_malloc_pages);

        heap_end = malloc_virt_address + total_malloc_pages * PAGE_SIZE;        // keep track of the heap end
        uint32_t virt = malloc_virt_address;    // localize it to malloc_init, as we want EVERY process to begin with virt address
                                        // 4MB for their heap. Alternatively, we could just set virt_address = 4MB at the end of
                                        // each initialize, but whatever

        // THIS MAY CAUSE ISSUES IN THE FUTURE... WILL HAVE TO SEE
        malloc_head = (malloc_node_t*)malloc_virt_address;

        for(uint32_t i = 0; i < total_malloc_pages; i++) {
                map_address(directory, malloc_phys_address + i*PAGE_SIZE, virt, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
                virt+=PAGE_SIZE;
        }


        malloc_head->size = (total_malloc_pages * PAGE_SIZE) - sizeof(malloc_node_t);   // malloc_block_t is basically header info
                                                                                        // each malloc'd piece of memory
        malloc_head->free = true;
        malloc_head->next = NULL;
        malloc_head->prev = NULL;
        malloc_head->address = (void*)((uint32_t)malloc_head + sizeof(malloc_node_t));  // again, might remove later..

        return (void*)malloc_head->address;
}

// This function is effectively an sbrk(). It is only used when we absolutely NEED more memory. This function will likely be the
// only function that exists here, in the kernel implementation (and not user space).
void* malloc_more_pages(uint32_t size) {
        total_malloc_pages = size/PAGE_SIZE;
        if(size % PAGE_SIZE > 0) {
                total_malloc_pages++;   // in case we want <4KB worth of bytes to be allocated in heap
        }

        malloc_phys_address = (uint32_t)allocate_blocks(total_malloc_pages);
        uint32_t virt = heap_end;
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
        new_node->prev = temp;
        new_node->address = (void*)((uint32_t)new_node + sizeof(malloc_node_t));

        // Previous element in list should point to new node
        temp->next = new_node;

        return (void*)new_node->address;
}

// this is technically WRONG.. Syntax for calloc is (int n, int size), so if we have calloc(5, sizeof(int)), it should really give us 5 blocks of 4-byte things, but... I don't care.
void* calloc_more_pages(uint32_t size) {
        void* ptr = malloc_more_pages(size);
        memset(ptr, 0, total_malloc_pages*PAGE_SIZE);   // calloc is just malloc but we 0 out the region
        return ptr;
}

// realloc implementation, (might have to fix in the future)
void malloc_realloc(void* ptr, uint32_t size) {
        malloc_node_t* temp = malloc_head;
        while(temp != NULL) {
                if(temp->address == ptr) {
                        // assume the caller will only want to make it BIGGER not SMALLER
                        uint32_t diff = size - temp->size;
                        total_malloc_pages = diff/PAGE_SIZE;
                        if(diff % PAGE_SIZE > 0) {
                                total_malloc_pages++;
                        }
                        malloc_phys_address = (uint32_t)allocate_blocks(total_malloc_pages);
                        uint32_t virt = temp->address + temp->size;

                        // now, here, a lock would be quite nice in the future once we have a scheduler
                        for(uint32_t i = 0; i < total_malloc_pages; i++) {
                                map_address(directory,  malloc_phys_address + i*PAGE_SIZE, virt, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
                                virt+=PAGE_SIZE; 
                        }
                        
                        temp->size = size; 
                        heap_end += diff; 
                        return; 
                }
                temp = temp->next
        }
}

void* split_blocks(malloc_node_t* node, uint32_t size) {

        // Originally, I tried to use the approach that the last block will always be free, which is true, but I didn't take
        // into account that fragmentation could occur, sooo...
        malloc_node_t* temp = node;

        // Assumed to be free, but just in case
        if(temp->free) {
                if(temp->size >= size + sizeof(malloc_node_t)) {

                        //  We want the bigger chunk (the older node w/ free data) to be, again, the end-most element of the dll
                        malloc_node_t* new_node = temp;
                        malloc_node_t* old_node = (malloc_node_t*)((char*)new_node + size + sizeof(malloc_node_t));

                         /*
                         * There is likely to be a logical bug here. Basically, what I'm trying to achieve is that each node will 
                         * be a certain size and include header information that can be used to identify each node. This splitting
                         * removes the amount of bytes we want out of the bigger, free node, allocates it to be used, and returns
                         * the address of the start of the available user data block (it skips the metadata header). 
                         */
                        old_node->next = NULL;
                        old_node->prev = new_node;
                        old_node->size = temp->size - size - sizeof(malloc_node_t);
                        old_node->free = true;
                        old_node->address = (void*)((uint32_t)old_node + sizeof(malloc_node_t));

                        new_node->next = old_node;
                        new_node->prev = temp->prev;
                        new_node->size = size;
                        new_node->free = false;
                        new_node->address = (void*)((uint32_t)new_node + sizeof(malloc_node_t));

                        node = new_node;         // We have to actually reflect the changes in the list too
                        
                        return (void*)(new_node->address);
                }
                else {
                        return malloc_more_pages(size);
                }
        }
        return NULL;
}

void merge_free_blocks() {
        malloc_node_t* temp = malloc_head;
        while(temp && temp->next) {
                if(temp->free && temp->next->free) {
                        temp->size += (temp->next->size) + sizeof(malloc_node_t);
                        temp->next = temp->next->next;
                }
                temp = temp->next;
        }
}

void* malloc_next(uint32_t size) {
        if(size == 0) {
                return NULL;
        }

        malloc_node_t* temp = malloc_head;
        while(temp != NULL) {        // NEW
                if(temp->free && temp->size == size + sizeof(malloc_node_t)) {
                        temp->free = false;
                        return temp->address;
                }
                else if(temp->free && temp->size > size + sizeof(malloc_node_t)) {
                        return split_blocks(temp, size);
                }
                temp = temp->next;
        }

        // Otherwise, we assume that there is not enough memory
        void* ptr = malloc_more_pages(size);

        // this if statement doesn't avoid internal fragmentation, but I don't care for right now
        if(size < PAGE_SIZE) {
                malloc_node_t* cur = malloc_head;
                while(cur->next != NULL) {
                        if(cur->address == ptr) {
                                return split_blocks(cur, size);
                        }
                        cur = cur->next;
                }
        }

        return ptr;
}

void malloc_free(void* ptr) {
        malloc_node_t* temp = malloc_head;
        while(temp->next != NULL) {
                if(temp->address == ptr) {
                        temp->free = true;
                        merge_free_blocks();
                        break;
                }
        }
}
