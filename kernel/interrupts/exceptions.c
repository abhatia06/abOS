#include "../stdint.h"
#include "../stdio.h"
#include "idt.h"
#include <stdbool.h>
#include "exceptions.h"
#include "pic.h"
#include "../x86.h"
#include "keyboard.h"

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
                char keymapped = keymap[key];
                if(key == ENTER) {
                        // for now, we don't do anything, but eventually we'll want to save whatever the user types.
                        // idk how to do that just yet, im guessing i'll need a global buffer of some sort or something idk lol
                }
                kprintf("%c", keymapped);
        }
        PIC_sendEOI(1);        // We tell the PIC that we're done with processing this interrupt
}
