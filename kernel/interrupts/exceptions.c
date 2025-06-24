#include "../stdint.h"
#include "../stdio.h"
#include "idt.h"
#include <stdbool.h>
#include "exceptions.h"
#include "pic.h"
#include "../x86.h"

// These codes are the hex ASCII code for the keys they represent.
#define LSHIFT_MAKE     0x2A        
#define LSHIFT_BREAK    0xAA
#define RSHIFT_MAKE     0x36
#define RSHIFT_BREAK    0xB6
#define LCTRL_MAKE      0x1D
#define LCTRL_BREAK     0x9D
#define ENTER           0x1C

__attribute__((interrupt)) void div_by_0_handler(int_frame_32_t *frame) {
        kprintf("DIVIDE BY 0 ERROR\r\n");

        __asm__ volatile ("cli ; hlt");
}

__attribute__((interrupt)) void keyboard_handler(int_frame_32_t *frame) {
        uint8_t key = inb(0x60);        // This is the port that the key data goes into (input)
                                        // For mouse, we need to use port 64, and then put a certain hex code into AL, I believe.

        if(key & 0x80) {
                // nothing for now
        }
        else {
                if(key == ENTER) {
                        kprintf("\r\n");
                }
                kprintf("key pressed: 0x%x\r\n", key);
        }
        PIC_sendEOI(1);        // We tell the PIC that we're done with processing this interrupt
}
