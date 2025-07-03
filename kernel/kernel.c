        #include "stdint.h"
#include "stdio.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "interrupts/exceptions.h"
#include "memory/physical_memory_manager.h"
#include "util/string.h"
#include "memory/virtual_memory_manager.h"
#include "global_addresses.h"

void user_mode_entry_point();

/*
void memorysetup() {
        initialize_pmm();
        //deinit_memory_region(0x0, 0x12000);
        deinit_memory_region(0x100000, 81920);  // Reserve 20 blocks of physical memory for the kernel
        deinit_memory_region(0x7EE0000, 20480);
        //initialize_memory_region(0x100000, 4096);
        //int32_t checker = find_free_blocks(1);
        //kprintf("bit address: %d\r\n", checker);
        // Add this to inspect the stack
        unsigned long esp;
        __asm__ volatile ("mov %%esp, %0" : "=r"(esp));
        kprintf("ESP: 0x%x\r\n", esp);

}
*/

void main() {
        directory = (pdirectory*)*(uint32_t *)CURRENT_PAGE_DIR_ADDRESS;
        memory_map = (uint32_t *)MEMMAP_AREA;
        max_blocks = *(uint32_t *)MAX_BLOCKS;
        used_blocks = *(uint32_t *)USED_BLOCKS;
        pic_disable();
        //memorysetup();

        kprintf("\n");
        print_memmap_command();
        kprintf("\n");
        //initialize_vmm();

        initIDT();
        idt_set_descriptor(0, (uint32_t)div_by_0_handler, 0x8E);
        PIC_remap(0x20);

        idt_set_descriptor(0x20, (uint32_t)PIT_handler, 0x8E);
        idt_set_descriptor(0x21, (uint32_t)keyboard_handler, 0x8E);
        idt_set_descriptor(0x0E, (uint32_t)page_fault_handler, 0x8E);

        IRQ_clear_mask(0); // Enable keyboard interrupts
        IRQ_clear_mask(1); // Enable timer (PIT)

        //Set default PIT timer IRQ rate to be about 1 millisecond
        set_PIT(0, 2, 11931);
        // Testing kprintf
        const char* far_str = "far string";
        kprintf("Formatted %% %c %s %s\r\n", 'a', "string", far_str);
        kprintf("Test: %lld\r\n", 10200300400ll);
        kprintf("Formatted %llx\r\n", 0xdeadbeeffeebdaedull);
        kprintf("Formatted %d %i %x %p %o %hd %hi %hhu %hhd\r\n", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (unsigned char)20, (signed char)-10);

        // testing input buffer from keyboard interrupts
        kprintf("Type your name: ");
        char* name = readline();
        kprintf("Hello, %s!\r\n", name);

        //map_page((void*)0xB8000, (void*)0xC00B8000);
        //volatile char* vid = (volatile char*)0xC00B8000;      // 0xB8000 should map to 0xC00B8000 now, so writing at
                                                                // 0xC00B00 should write to 0xB8000 instead. Inside the
                                                                // page table, (I believe it is entry 184, PD 768),
                                                                // bits 31-12 will be the address (0xB8000), with the
                                                                // rest of the bits being the flags (in this case, it
                                                                // will be PRESENT and WRITABLE). So, writing to
                                                                // 0xC00B8000 MUST write to 0xB8000.
        //vid[0] = 'X';
        //vid[1] = 0x0F;

        map_page((void*)0x700000, (void*)0xBFFFF000); // for user stack. Picked an arbitrary position (7MB) and an arbitrary virtual address for testing purposes
        
        __asm__ volatile("cli\n"
                "mov $0x23, %%eax\n"
                "mov %%ax, %%ds\n"
                "mov %%ax, %%es\n"
                "mov %%ax, %%fs\n"
                "mov %%ax, %%gs\n"

                "pushl $0x23\n"
                "pushl %[stack]\n"
                "pushf\n"
                "pop %%eax\n"
                "or $0x200, %%eax\n"
                "push %%eax\n"
                "pushl $0x1B\n"
                "pushl %[entry]\n"
                "iret\n"
                :
                : [stack] "r"(USER_STACK), [entry] "r"(user_mode_entry_point)
                : "eax"
        );

        while(true) {
                char* command = readline();
                if(strcmp((const char*)command, "printmem") == 0) {
                        kprintf("\r\n");
                        print_memmap_command();
                        kprintf("\r\n");
                }
        }



        //uint32_t check = (uint32_t *)0xC0000000;
        //kprintf("First word printed at 0xC0000000 = 0x%x\n", check);

        //clrscr();
        //kprintf("reaching here");
        //((void (*)(void))0xC0000000)();
}

// will rename this to shell eventually. Because it all started in the shell
void user_mode_entry_point() {
        while(1) {
                __asm__ volatile("hlt");
        }
}
