#include "physical_memory_manager.h"

uint32_t *memory_map = (uint32_t*)start_addr;
uint32_t max_blocks = 0xFFFFFFFF/BLOCK_SIZE;    // Maximum number of available blocks
uint32_t used_blocks = 0;

void set_block(uint32_t bit) {
        // We've told the compiler that everything starting at the start_addr has 32-bit integers. In a bitmap, we
        // state that every bit controls its own page/chunk (the 4KB chunks). So, when, for example, the 40th chunk has
        // been occupied, we must flip the 40th available bit in the memory_map. The memory_map is, again, divided into
        // 32-bit integers, (so, memory_map[0] is bits 0-31, memory_map[1] is bits 32-63, etc.), so if we wish to flip
        // the 40th bit, we need to access memory_map[1], and a good way to do this is to do bit/32. Since bit is an
        // integer, it'll use the ceiling/floor function (I forgot which one) to round and give us 1. After that, we
        // can use modulo, which will give us the remainder for which the bit is divided by 32. We can then use this
        // remainder to say "hey, at index 1, I want the bit%32-th bit to be flipped to 1". This is exactly what the
        // line below accomplishes.
        memory_map[bit/32] |= (1 << (bit % 32));
}

void unset_block(uint32_t bit) {
        // The code below has a similar explanation to the top, but this uses a different bit-wise operation. The
        // general rule of thumb, (which you should already know if you want to get into OSDev), is that if you want to
        // flip a bit to 1, you use the bit-wise OR operation with 1, and if you want to flip a bit to 0, you use the
        // bit-wise AND oepration with 0, (duh).
        memory_map[bit/32] &= ~(1 << (bit % 32));
}

// This function is quite unoptimized. You will find better versions/make a better version yourself, but for now, I only
// care about if it works or not. This function will scan every bit 1-by-1, so the performance is quite slow.
int32_t find_free_blocks(uint32_t num_blocks) {
        if(num_blocks <= 0) {
                return -1;
        }

        int count = 0;
        uint32_t holder = 0;
        for(uint32_t i = 0; i < max_blocks; i++) {
                holder = memory_map[i/32];      // Holds the 32-bit integer inside index i/32
                if(!(holder & (1 << (i%32)))) { // Check to see if the current bit is 0 (not mapped/unset)
                        count++;
                        if(count == num_blocks) {
                                return (i-num_blocks+1)*BLOCK_SIZE;     // Return starting address of available memory
                        }
                }
                else {
                        count = 0;        // If the current bit is 1, we should rese the count
                }
        }
        // Unable to find available memory
        return -1;
}

void* allocate_blocks(uint32_t num_blocks) {
        int32_t first_starting_bit = find_free_blocks(num_blocks);
        if(first_starting_bit == -1) {        // If the function is unable to find free blocks for any reason, we return
                return 0;
        }

        for(uint32_t i = 0; i < num_blocks; i++) {
                set_block(first_starting_bit + i);        // Otherwise, we just set the blocks
        }

        used_blocks += num_blocks;        // We make sure to record the blocks set

        uint32_t address = first_starting_bit * BLOCK_SIZE;

        return (void*)address;  // Physical memory location of allocated blocks
}

void free_blocks(uint32_t bit, uint32_t num_blocks) {
        for(uint32_t i = 0; i < num_blocks; i++) {
                unset_block(bit + i);
        }

        used_blocks -= num_blocks;
}

// I basically stole this off from Queso Fuego's OS. I also stole some of his planning. Basically, I will initially set
// every bit on the bitmap to 1, meaning it is being used. Then, we will manually go in and free various memory regions,
// (like the ones specified by INT 0x15 EAX=E820. Why? Because it's a pain to just use the allocate_blocks by hand.
// Also, the place where I will likely diverge from Queso Fuego's OS (I dunno, he might do the same), is I will be
// reserving part of the 0x100000 memory too. This is because that's where my kernel is, so I will likely reserve the
// first 20 or so KB to be for kernel, then the rest can be for whatever we want. Most textbooks/online sources I've
// read recommend keeping the maps even higher (like maybe at 2 MB), and I might do that. I dunno, I'll see.
void initialize_memory_region(uint32_t base_address, uint32_t size) {

        int32_t align = base_address/BLOCK_SIZE;
        int32_t num_blocks = size/BLOCK_SIZE;

        for(; num_blocks > 0; num_blocks--) {
                unset_block(align++);
                used_blocks--;
        }

        // We set the first block (first 4KB, from 0x0) to be 1. Always. This is because that place has some important
        // stuff, like the IVT and whatnot, so we would rather not touch it.
        set_block(0);
}

// Similar to initialize but opposite, (duh)
void deinit_memory_region(uint32_t base_address, uint32_t size) {
        int32_t align = base_address/BLOCK_SIZE;
        int32_t num_blocks = size/BLOCK_SIZE;

        for(; num_blocks > 0; num_blocks--) {
                set_block(align++);
                used_blocks++;
        }
}

void initialize_pmm() {
        // We begin by setting the entire bitmap to be inactive.
        // Of course, they will still have access
        deinit_memory_region(0x0, 0xFFFFFFFF);
}

// And I believe that is the entire physical memory manager. So far, here is my gameplan, which I will be putting into
// my explanations.txt too:
// I begin by calling initialize_pmm(), which will set the entire bitmap to be 1's. This will effectively state,
// "hey, the entire memory is in usage/reserved". But in reality, it is not. Then, in the kernel, based on the available
// entries in the SMAP (from INT 0x15, EAX E820), I will call initialize_memory_region with their base addresses and
// lengths (all of which is given by the SMAP). Then, I will manually go in and deinitialize some of the memory from
// 0x100000 (a few KB, maybe a full MB), because I want that to be reserved for the kernel. I will also deinitialize
// some of the memory from 0x7EE0000, because that is where the bitmap is, and I don't want it to ever override it.
// Finally comes the creation of malloc, or the memory file in general. My plan for memory is that I will have two
// functions, at least. One function will find the size/length of a process/program/file. How I will find the size,
// I don't know yet. I'll have to look at some tutorials, maybe. After that, I will have a memory allocation function,
// (kmalloc), that will put a given program/file/process into memory based on the base_address that is going to be
// provided when I use allocate_blocks(num_blocks). And then, boom! I think that'll be everything. I'll need to make
// a de-allocation method too, that will also free memory of the process. Then that should be it.. I should have a
// genuine physical memory manager then.
