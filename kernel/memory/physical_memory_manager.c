#include "physical_memory_manager.h"
#include "../stdint.h"
#include <stdbool.h>
#include "../util/string.h"

uint32_t *memory_map = 0;
uint32_t max_blocks = 0;                // Maximum number of available blocks
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

/*        // I added this like a while ago... I don't remember what I used it for, but I don't think it's ever used. But, in the case it is, it's here.
void unset_block_bulk(uint32_t start_bit, uint32_t count) {
        uint32_t bit = start_bit;
        uint32_t end_bit = start_bit + count;

        while(bit < end_bit) {
                uint32_t word_index = bit/32;
                uint32_t bit_offset = bit%32;

                if(bit_offset == 0 && (end_bit - bit) >= 32) {
                        memory_map[word_index] = 0;
                        bit += 32;
                }
                else {
                        memory_map[word_index] &= ~(1 << bit_offset);
                        bit++;
                }
        }

        used_blocks -= count;
}
*/

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

                // we don't want to go 1-by-1 or words that are entirely allocated. This saves a BIT of time during launch
                if(holder == 0xFFFFFFFF) {
                        count = 0;
                        i = (((i/32) + 1) * 32) - 1;         // fancy math that basically skips this word. We take i, divide by 32 to get the word, go to the next word, put it back
                        // to being a multiple of 32, and then subtract 1 because when we do continue below, the for loop will add 1 to i.
                        continue;
                }
                
                if(!(holder & (1u << (i%32)))) { // Check to see if the current bit is 0
                        count++;
                        if(count == num_blocks) {
                                return (i-num_blocks+1);        // Return starting address of available memory
                        }
                }
                else {
                        count = 0;
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
// like the ones specified by INT 0x15 EAX=E820.
void initialize_memory_region(uint32_t base_address, uint32_t size) {

        int32_t align = base_address/BLOCK_SIZE;
        int32_t num_blocks = size/BLOCK_SIZE;

        if(num_blocks == 0) {
                return;
        }
        
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
        memory_map = (uint32_t*)start_addr;
        max_blocks = 0xFFFFFFFF/BLOCK_SIZE;
        // We begin by setting the entire bitmap to be inactive.
        // Of course, they will still have access
        deinit_memory_region(0x0, 0xFFFFFFFF);
        //memset(memory_map, 0xFF, max_blocks / BITMAP_DIVISON);
}
