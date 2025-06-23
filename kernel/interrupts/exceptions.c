#include "../stdint.h"
#include "../stdio.h"
#include "idt.h"
#include <stdbool.h>
#include "exceptions.h"

__attribute__((interrupt)) void div_by_0_handler(int_frame_32_t *frame) {
        kprintf("DIVIDE BY 0 ERROR\r\n");

        __asm__ volatile ("cli ; hlt");
}
