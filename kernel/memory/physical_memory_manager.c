#include "physical_memory_manager.h"

uint32_t *memory_map = (uint32_t*)start_addr;

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
