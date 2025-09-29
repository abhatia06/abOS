#include "../stdint.h"
#include "../stdio.h"
#include "idt.h"
#include <stdbool.h>
#include "exceptions.h"
#include "pic.h"
#include "../x86.h"
#include "keyboard.h"
#include "../memory/virtual_memory_manager.h"
#include "../memory/physical_memory_manager.h"

// input stuff for keyboard
char input_buffer[INPUT_BUFFER_SIZE] = {0};
unsigned int input_pos = 0;
bool input_ready = false;

// PIT tick tracker
volatile uint64_t ticks = 0;        // This is likely temporary. Since we don't have scheduling, the PIT is a little bit useless, but who cares. I'll enable it.

__attribute__((interrupt)) void div_by_0_handler(int_frame_32_t *frame) {
        kprintf("DIVIDE BY 0 ERROR\r\n");

        __asm__ volatile ("cli ; hlt");
}

__attribute__((interrupt)) void keyboard_handler(int_frame_32_t *frame) {
        uint8_t key = inb(0x60);
        static key_info_t key_info = {0};
        static bool e0 = false; // honestly not really used at all. I don't think I'll have to worry about it JUST YET
        if(true) {
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
                                e0 = true;
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
                                                        // add more shifted keys (booorrrinngg)
                                                }
                                        }
                                        if(keypressed != 0) {
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
                                                                input_pos = 0;
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
                        }
                        if(e0) {
                                e0 = false;
                        }
                }
        }
        PIC_sendEOI(1);
}        
__attribute__((interrupt)) void PIT_handler(int_frame_32_t *frame) {
        (void)frame;
        ticks++;
        PIC_sendEOI(0);
}

__attribute__((interrupt)) void page_fault_handler(int_frame_32_t *frame, uint32_t error_code) {

        uint32_t bad_address;
        __asm__ volatile ("mov %%cr2, %0" : "=r"(bad_address));

        //kprintf("PAGE FAULT AT 0x%x. Error code: 0x%x\n", bad_address, error_code);

        // handling page faults comes later

        void* phys_address = allocate_blocks(1);
        map_address(directory, (uint32_t)phys_address, bad_address, PTE_PRESENT | PTE_WRITABLE | PTE_USER);

        //kprintf("RESOLVED PAGE FAULT. NEW PHYS ADDRESS: 0x%x, NEW VIRT ADDRESS: 0x%x\n", (uint32_t)phys_address, bad_address);
        
        // flush tlb
         __asm__ volatile ("movl %%cr3, %%ecx; movl %%ecx, %%cr3" ::: "ecx");
}

__attribute__((interrupt)) void gpf_handler(int_frame_32_t* frame, uint32_t error_code) {
        kprintf("GENERAL PROTECTION FAULT. Error Code: 0x%x\n", error_code);
        __asm__ volatile("cli;hlt");
}
