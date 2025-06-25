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
        uint8_t key = inb(0x60);
        static key_info_t key_info = {0};
        static bool e0 = false; // honestly not really used at all. I don't think I'll have to worry about it JUST YET
        if(true) {        // I develop my stuff on Vim, and previously I had an if statement here that I later removed. I didn't want to fix the 
                          // indents, so I just decided to put a if(true) here because im lazy
                if(key == ENTER) {
                        // we do nothing for now, but eventually, we'll want to save whatever the user types
                        // idk how to do that just yet, but we'll probably need to use a global buffer of some
                        // sort
                }
                if(key) {

                        if(key == LSHIFT_MAKE || key == RSHIFT_MAKE) {
                                key_info.shift = true;
                        }
                        else if(key == LSHIFT_BREAK || key == RSHIFT_BREAK) {
                                key_info.shift = false;
                        }
                        else if (key == LCTRL_MAKE) {
                                key_info.ctrl = true;
                        }
                        else if (key == LCTRL_BREAK) {
                                key_info.ctrl = false;
                        }
                        else if(key == CAPSLOCK && key_info.caps == true) {
                                key_info.caps = false;
                        }
                        else if(key == CAPSLOCK && key_info.caps == false) {
                                key_info.caps = true;
                        }
                        else if(key == 0xE0) {
                                e0 = true;        // E0 is for when stuff like the arrow keys are pressed. We don't actually handle these keys just yet, but we will
                                                  // eventually, I hope.
                        }

                        if(!(key & 0x80)) {
                                if(!e0 && key < 128) {
                                        char keypressed = keymap[key];

                                        if(key_info.shift == true || key_info.caps == true) {
                                                if(keypressed >= 'a' && keypressed <= 'z') {
                                                        keypressed -= 0x20;
                                                        // convert lowercase to uppercase (look at ASCII table)
                                                }
                                                else if(keypressed >= '0' && keypressed <= '9') {
                                                // The numbers start at 0x30 in the ASCII table. Num_row_shifts is an
                                                // array that contains the shift-characters of each number. So, we
                                                // simply subtract 0x30 off the key pressed, and base it off the
                                                // num row shifts array.
                                                        keypressed = num_row_shifts[keypressed-0x30];
                                                }
                                                else {
                                                        if(keypressed == '=') {
                                                                keypressed = '+';
                                                        }
                                                        if(keypressed == '/') {
                                                                keypressed = '?';
                                                        }
                                                        if(keypressed == '\'') {
                                                                keypressed = '"';
                                                        }
                                                        if(keypressed == ';') {
                                                                keypressed = ':';
                                                        }
                                                        if(keypressed == '-') {
                                                                keypressed = '_';
                                                        }
                                                        // add more shifted keys (booorrrinngg)
                                                }
                                        }
                                        if(keypressed != 0) {   // so we don't create spaces whenever we press shift
                                                // Another case of incorrect if statement that I was too lazy to fix the indentations for so I just used if(true).
                                                // The reasoning for this change is because the previous one, if the if condition wasn't satisfied, didn't allow the
                                                // user to do anything at all, (couldn't even press enter), so I just decided to use this. It still works just fine,
                                                // but we can only accept 256 characters the user types, but we can allow the user to type an infinite number of characters
                                                if(true) {
                                                        if(keypressed == '\b') {
                                                                // We only allow the user to delete what they've typed
                                                                if(input_pos > 0) {
                                                                        // Go back (duh), and
                                                                        input_pos--;
                                                                        // override the previous character with sentinel
                                                                        input_buffer[input_pos] = '\0';
                                                                        // then display the changes
                                                                        kprintf("\b");
                                                                }
                                                        }
                                                        else if(keypressed == '\n') {
                                                                // if the user enters, that means their message is
                                                                // complete, and we can process it now
                                                                input_ready = true;
                                                                kprintf("\n");
                                                                input_pos = 0;      // We need to reset the buffer to allow for more messages.
                                                        }
                                                        else {
                                                                //input_pos always points to the sentinel, so we just
                                                                //have to override the sentinel with the new key
                                                                input_buffer[input_pos] = keypressed;

                                                                //then increment and add a new sentinel there
                                                                input_pos++;
                                                                input_buffer[input_pos] = '\0';
                                                                kprintf("%c", keypressed);
                                                        }
                                                }
                                        }
                                }
                                if(e0) {
                                        e0 = false;
                                }
                        }
                }
        }
        PIC_sendEOI(1);
}
