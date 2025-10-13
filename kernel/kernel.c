#include "stdint.h"
#include "stdio.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "interrupts/exceptions.h"
#include "memory/physical_memory_manager.h"
#include "util/string.h"
#include "memory/virtual_memory_manager.h"
#include "global_addresses.h"
#include "interrupts/syscalls.h"
#include "memory/malloc.h"
#include "fs/fs.h"
#include "fs/fs_commands.h"
#include <stdbool.h>
#include "stdlib.h"
#include "interrupts/unistd.h"

void shell(bool returning);
void tester23();
void init_malloc();
uint32_t file_virtual_address;
open_file_t* open_file_table;
inode_t* open_inode_table;
uint32_t current_open_files;
uint32_t current_open_inodes;
void* user_stack = (void*)0xB0000000;
bool print_registers();

void main() {
        directory = (pdirectory*)*(uint32_t*)CURRENT_PAGE_DIR_ADDRESS;
        //current_pd_address = *(uint32_t*)CURRENT_PD_ADD;
        memory_map = (uint32_t *)MEMMAP_AREA;
        max_blocks = *(uint32_t *)MAX_BLOCKS;
        used_blocks = *(uint32_t *)USED_BLOCKS;

        superblock = *(superblock_t*)0x8C00;
        memset((void*)sector_buffer[FS_SECTOR], 0, 512);
        //kprintf("superblock first inode block: %d\n", superblock.first_inode_block);
        rw_sectors(1, superblock.first_inode_block*8, (uint32_t)sector_buffer, READ);
        //__asm__ volatile("cli;hlt");

        root_inode = *((inode_t*)((uint8_t*)sector_buffer+64));
        superblock.root_i_number = (uint32_t)&root_inode;
        current_dir_inode = root_inode;
        //kprintf("root inode directory type: %d\n", root_inode.file_type);

        //__asm__ volatile("cli;hlt");
        init_malloc();

        //malloc_init();
        //open_file_t* open_file_table = malloc_next(sizeof(open_file_t) * 256);
        //open_file_t* open_file_table = malloc(sizeof(open_file_t) * 256);
        //*open_file_table = (open_file_t){0};
        //__asm__ volatile ("cli;hlt" : : "a"(0xDEADBEEF));
        //open_file_t open_file_table[256];
        //inode_t open_inode_table[256];

        //kprintf("Current page directory address: 0x%x\n", (uint32_t)directory);
        //kprintf("Superblock address: 0x%x\n", (uint32_t)&superblock);

        pic_disable();

        kprintf("\n");
        print_memmap_command();
        kprintf("\n");

        //__asm__ volatile("cli;hlt");
        initIDT();

        // TODO: swap out magic hexadecimal numbers (0x8E and 0xEE) for actual macros that define what the flags are
        idt_set_descriptor(0, (uint32_t)div_by_0_handler, 0x8E);        // present, DPL 0, 32-bit interrupt gate

        idt_set_descriptor(0x20, (uint32_t)PIT_handler, 0x8E);
        idt_set_descriptor(0x21, (uint32_t)keyboard_handler, 0x8E);
        idt_set_descriptor(0x0E, (uint32_t)page_fault_handler, 0x8E);
        idt_set_descriptor(0x80, (uint32_t)syscall_handler, 0xEE);      // present, DPL 3, 32-bit interrupt gate
        idt_set_descriptor(0xD, (uint32_t)gpf_handler, 0x8E);

        PIC_remap(0x20);
        IRQ_clear_mask(0); // Enable keyboard interrupts
        IRQ_clear_mask(1); // Enable timer (PIT)

        //Set default PIT timer IRQ rate to be about 1 millisecond
        set_PIT(0, 2, 11931);


        //__asm__ volatile("cli;hlt" :: "a"(0xdeadbeef));
        //malloc_init();
        open_file_table = malloc(sizeof(open_file_t) * 256);
        *open_file_table = (open_file_t){0};
        //__asm__ volatile("cli;hlt" :: "a"(0xdeadbeef));
        //kprintf("0x%x\n", open_file_table);
        open_inode_table = malloc(sizeof(inode_t) * 256);
        *open_inode_table = (inode_t){0};
        //kprintf("0x%x\n", open_inode_table);
        current_open_files = 0;
        current_open_inodes = 0;
        file_virtual_address = 0x40000000;


        /*
        malloc_init();
        open_file_table = malloc_next(sizeof(open_file_t) * 256);
        *open_file_table = (open_file_t){0};
        kprintf("0x%x\n", open_file_table);
        open_inode_table = malloc_next(sizeof(inode_t) * 256);
        *open_inode_table = (inode_t){0};
        kprintf("0x%x\n", open_inode_table);
        current_open_files = 0;
        current_open_inodes = 0;
        file_virtual_address = 0x40000000;
        */

        //__asm__ volatile("cli;hlt");
        // Testing kprintf

        /*
        const char* far_str = "far string";
        kprintf("Formatted %% %c %s %s\r\n", 'a', "string", far_str);
        kprintf("Test: %lld\r\n", 10200300400ll);
        kprintf("Formatted %llx\r\n", 0xdeadbeeffeebdaedull);
        kprintf("Formatted %d %i %x %p %o %hd %hi %hhu %hhd\r\n", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (unsigned char)20, (signed char)-10);
        */

        // testing input buffer from keyboard interrupts
        //kprintf("Type your name: ");
        //char* name = readline();
        //kprintf("Hello, %s!\r\n", name);

        /*
        kprintf("root inode directory type: %d\n", root_inode.file_type);
        inode_t test1 = get_inode_in_dir(current_dir_inode, "test.txt");
        kprintf("test test1 inode: %d\n", test1.i_number);
        char* tbuffer = (char*)0x500000;
        bool test2 = load_file(&test1, (uint32_t)tbuffer);
        inode_t test3 = get_inode("/kernel.bin");
        kprintf("test3 inode: %d\n", test3.i_number);
        inode_t test4 = create_file("/testing23.txt");
        kprintf("test4 inode: %d\n", test4.i_number);
        inode_t test5 = create_file("/testing123.txt");
        kprintf("test5 inode: %d\n", test5.i_number);
        */
        //print_dir("/TEST2323/");
        //__asm__ volatile("cli;hlt");

        /*
        // should be 0. You should only be getting a fd of >3 IF you have stderr, stdout, and stdin, which we don't
        int32_t testing2323 = open("/test.txt", O_RDWR);
        kprintf("testing2323 fd: %d\n", testing2323);
        kprintf("inode?: %d\n", open_inode_table[open_file_table[testing2323].inode_index].i_number);
        kprintf("address: 0x%x\n", open_file_table[testing2323].address);
        char* tbuffer = 0;
        read(testing2323, tbuffer, 4096);
        kprintf("%s\n", tbuffer);
        char* wahbungusbuffer = "IT WORKS!!!! I BELIEVE SO!";
        write(testing2323, wahbungusbuffer, 4096);
        read(testing2323, tbuffer, 4096);
        kprintf("%s\n", tbuffer);
        int32_t testing200 = open("/boobootest.txt", O_RDWR);
        kprintf("testing200 fd: %d\n", testing200);
        kprintf("address: 0x%x\n", open_file_table[testing200].address);
        */

        //int32_t pleasework = close(testing2323);
        //kprintf("worked?: %d\n", pleasework);
        //kprintf("inode: %d\n", open_inode_table[open_file_table[testing2323].inode_index].i_number);
        //__asm__ volatile("cli;hlt" : : "a"(0xDEADBEEF));
        //if(test2) {
        //      kprintf("WORKS\n");
        //}
        //else {
        //      kprintf("FAILS\n");
        //}

        //kprintf("here it is: %s\n", tbuffer);
        //__asm__ volatile ("cli;hlt" :: "r"(0xDEADBEEF));
        //bool tester2 = map_page((void*)0x700000, (void*)0xBFFF000);
        //kprintf("boolean: %d\n", tester2);
        //volatile char* vid = (volatile char*)0xC00B8000;      // 0xB8000 should map to 0xC00B8000 now, so writing at
                                                                // 0xC00B00 should write to 0xB8000 instead. Inside the
                                                                // page table, (I believe it is entry 184, PD 768),
                                                                // bits 31-12 will be the address (0xB8000), with the
                                                                // rest of the bits being the flags (in this case, it
                                                                // will be PRESENT and WRITABLE). So, writing to
                                                                // 0xC00B8000 MUST write to 0xB8000.
        //vid[0] = 'X';
        //vid[1] = 0x0F;

        //name = readline();
        //bool tester1 = map_page((void*)0xB000000, (void*)0x300000);
        //kprintf("boolean: %d\n", tester1);
        //name = readline();

        //bool tester3 = map_page((void*)0x800000, (void*)0x800000);
        //name = readline();

        //pt_entry* stack_page = get_page((virtual_address)0xBFFFEFFC);
        //SET_ATTRIBUTE(stack_page, PTE_PRESENT);
        //SET_ATTRIBUTE(stack_page, PTE_USER);
        //SET_ATTRIBUTE(stack_page, PTE_WRITABLE);


        /*
        for(uint32_t i = 0; i < 4; i++) {
                uint32_t va = USER_STACK - i * 0x1000;
                map_page((void*)0x600000 - i * 0x1000, (void*)va);
        }


        uint32_t virtVGA = 0xB8000;
        for(uint32_t i = 0; i < 0x1000; i++) {
                map_address(directory, virtVGA, virtVGA, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
        }
        */

        //map_page((void*)0x600000, (void*)0xBFFFEFFC);

        /*
        uint32_t address = (uint32_t)malloc_init();
        kprintf("malloc initialized at address: 0x%x\n", address);
        address = (uint32_t)malloc_more_pages(4096);
        kprintf("malloc 1 page: 0x%x\n", address);
        address = (uint32_t)malloc_next(1000);
        kprintf("malloc next: 0x%x\n", address);
        */

        //((void (*)(void))0xA0000000)();
        // flush TLB

        //__asm__ __volatile__ ("movl %%cr3, %%ecx; movl %%ecx, %%cr3" ::: "ecx");

        /*
        int32_t ihopeitworks = open("/elftesting.bin", O_RDWR);
        kprintf("test: %d\n", ihopeitworks);
        uint32_t* jumpto = open_file_table[ihopeitworks].address;
        kprintf("address: 0x%x\n", jumpto);
        //((int (*)(void))jumpto)();


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
                        : [stack] "r"(USER_STACK), [entry] "r"(jumpto)
                        : "eax"
                );
        */

        /*
        uint32_t address = (uint32_t)malloc_init();
        kprintf("malloc initialized at address: 0x%x\n", address);
        address = (uint32_t)malloc_more_pages(4096);
        kprintf("malloc 1 page: 0x%x\n", address);
        address = (uint32_t)malloc_next(1000);
        kprintf("malloc next: 0x%x\n", address);
        address = (uint32_t)malloc_next(3);
        kprintf("malloc next2:0x%x\n", address);
        */
        /*
        int32_t ihopeitworks = open("/elftesting.bin", O_RDWR);
        kprintf("test: %d\n", ihopeitworks);
        uint32_t jumpto = open_file_table[ihopeitworks].address;
        kprintf("address: 0x%x\n", jumpto);
        ((int (*)(void))jumpto)();
        */
        //__asm__ volatile("cli;hlt" :: "a"(0xdeadbeef));
        clrscr();
        shell(false);

        /*
        //TODO: in the future, I would like to have the ""terminal"" operating in user space too, but that would
        //require a LOT of syscalls and I'm not dealing with that right now, (or maybe it won't require a lot of syscalls?
        //I dunno, I'll see in the future).
        kprintf("--------------------------------------------------------------------------------");
        kprintf("|                               Welcome To FishOS                              |");
        kprintf("--------------------------------------------------------------------------------\n");
        kprintf("This is currently an unfinished hobby OS. I plan on working on it in the future\n");
        kprintf("Type \"help\" to get a list of commands.\n");
        while(true) {
                kprintf(">");
                char* command = readline();
                if(strcmp((const char*)command, "printmem") == 0) {
                        print_memmap_command();
                }
                else if(strcmp(command, "ls") == 0) {   // yes I know this isn't how ls works
                        kprintf("Enter directory pathway to TEST:\n>");
                        command = readline();
                        kprintf("command: %s\n", command);
                        print_dir(command);
                }
                else if(strcmp(command, "exit") == 0) {
                        kprintf("Shutting down...\n");

                        //qemu shut down apparently
                        __asm__ ("outw %%ax, %%dx" : : "a"(0x2000), "d"(0x604));
                }
                else if(strcmp(command, "exec") == 0) {
                        kprintf("Enter filepath for file you wish to execute:\n>");
                        command = readline();
                        int32_t fd = open(command, O_RDWR);
                        uint32_t* jumpto = open_file_table[fd].address;
                        // enter user mode:
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
                                        : [stack] "r"(USER_STACK), [entry] "r"(jumpto)
                                        : "eax"
                        );
                }
                else if(strcmp(command, "clear") == 0) {
                        clrscr();
                }

        }
        */


        //uint32_t check = (uint32_t *)0xC0000000;
        //kprintf("First word printed at 0xC0000000 = 0x%x\n", check);

        //clrscr();
        //kprintf("reaching here");
        //((void (*)(void))0xC0000000)();
}


/*
 * A few error codes that can appear from interrupt 13 (general protection fault):
 * bit 0: segment error (privilege, type, limit, read/write rights)
 * bit 1: executing a privileged instruction outside of CPL 0 (or outside of ring 0)
 * bit 2: writing a 1 in a reserved register field or writing invalid value combinations (e.g. CR0 with PE=0 and PG=1).
 * bit 3: referencing or accessing null descriptor
 * bit 4: accessing a memory address with bits 48-63 not matching bit 47, (I don't really get this one lol)
 * bit 5: executing instruction that requires memory operands to be aligned (like movaps) without proper alignment
 * realistically, the only ones I will be seeing (for now, I hope) are error codes 0x0, and 0x2.
 */
__attribute__((noreturn)) void shell(bool returning) {
        if(!returning) {
                //TODO: in the future, I would like to have the ""terminal"" operating in user space too, but that would
                //require a LOT of syscalls and I'm not dealing with that right now, (or maybe it won't require a lot of syscalls?
                //I dunno, I'll see in the future).
                kprintf("--------------------------------------------------------------------------------");
                kprintf("|                               Welcome To ABOS                                |");
                kprintf("--------------------------------------------------------------------------------\n");
                kprintf("This is currently an unfinished hobby OS. I plan on working on it in the future\n");
                kprintf("Type \"help\" to get a list of commands.\n");
        }
        while(true) {
                kprintf(">");
                char* command = readline();
                if(strcmp(command, "printmem") == 0) {
                        print_memmap_command();
                }
                else if(strcmp(command, "ls") == 0) {   // yes I know this isn't how ls works
                        kprintf("Enter directory pathway to read:\n>");
                        command = readline();
                        print_dir(command);
                }
                else if(strcmp(command, "exit") == 0) {
                        kprintf("Shutting down...\n");

                        //qemu shut down apparently
                        __asm__ volatile ("outw %%ax, %%dx" : : "a"(0x2000), "d"(0x604));
                }
                else if(strcmp(command, "clear") == 0) {
                        clrscr();
                }
                else if(strcmp(command, "exec") == 0) {
                        kprintf("Enter the filepath of the file you wish to execute:\n>");
                        command = readline();
                        int32_t fd = open(command, O_RDWR);
                        if(fd < 0) {
                                kprintf("Invalid filepath\n");
                                continue;
                        }
                        kprintf("fd: %d\n", fd);
                        uint32_t* jumpto = open_file_table[fd].address;
                        int32_t stack = 0;
                        __asm__ volatile("mov %%esp, %0" : "=a"(stack));
                        uint8_t* gdt = (uint8_t*)*(uint32_t*)GDT_ADDRESS;

                        uint32_t tss_addr = *(uint16_t*)(gdt+0x2A);
                        uint32_t* tss = (uint32_t*)tss_addr;
                        *(tss+1) = stack;

                        user_stack =  user_stack + PAGE_SIZE;
                        void* phys_address = allocate_blocks(1);
                        map_address(directory, (uint32_t)phys_address, (uint32_t)user_stack, PTE_PRESENT | PTE_WRITABLE | PTE_USER);

                        kprintf("user stack: %x\n", user_stack);
                        kprintf("stack: %x\n", stack);
                        kprintf("address: 0x%x\n", jumpto);
                        __asm__ volatile("cli\n"
                                        "mov $0x23, %%eax\n"
                                        "mov %%ax, %%ds\n"
                                        "mov %%ax, %%es\n"
                                        "mov %%ax, %%fs\n"
                                        "mov %%ax, %%gs\n"
                                        "mov %0, %%EDX\n"

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
                                        : "r"(fd), [stack] "r"(USER_STACK), [entry] "r"(jumpto)
                                        : "eax"
                        );
                }
                else if(strcmp(command, "help") == 0) {
                        kprintf("List of commands:\n");
                        kprintf("\"printmem\" - prints out physical memory map\n");
                        kprintf("\"ls\" - allows you to look through a directory\n");
                        kprintf("\"exec\" - allows you to execute a raw bin file (NO .ELF SUPPORT YET)\n");
                        kprintf("\"touch\" - allows you to create a new file (NOT A DIRECTORY)\n");
                        kprintf("\"exit\" - shuts down\n");
                        kprintf("\"cat\" - reads a file (.txt only)\n");
                        kprintf("\"editfile\" - edits a file\n");
                        kprintf("\"printregisters\" - edits a file\n");
                }
                else if(strcmp(command, "touch") == 0) {
                        kprintf("Enter filepath of new file you wish to make:\n>");
                        command = readline();
                        create_file(command);

                }
                else if(strcmp(command, "cat") == 0) {
                        kprintf("Enter filepath of file you wish to read:\n>");
                        command = readline();
                        //__asm__ volatile("cli;hlt" :: "0xdeadbeef");
                        int32_t fd = open(command, O_RDWR);
                        if(fd < 0) {
                                kprintf("Invalid filepath\n");
                                continue;
                        }
                        read(fd, command, 4096);
                        kprintf("%s\n", command);
                        close(fd);
                }
                else if(strcmp(command, "editfile") == 0) {
                        kprintf("Enter filepath of file you wish to edit:\n>");
                        command = readline();
                        int32_t fd = open(command, O_RDWR);
                        if(fd < 0) {
                                continue;
                        }
                        kprintf("Enter what you'd like to edit the file to:\n>");
                        command = readline();
                        write(fd, command, 4096);
                        kprintf("Done\n");
                        close(fd);
                }
                else if(strcmp(command, "printregisters") == 0) {
                        print_registers();
                }
                else {
                        kprintf("Command not found\n");
                }

        }
}


void tester23() {
        __asm__ volatile("int $0x80" : : "a"(1));
}


void init_malloc() {
        extern malloc_node_t* malloc_head;
        extern uint32_t malloc_phys_address;
        extern uint32_t malloc_virt_address;
        extern uint32_t malloc_start;

        malloc_start = 0x400000;
        malloc_virt_address = malloc_start;
        malloc_phys_address = (uint32_t)allocate_blocks(1);
        map_address(directory, malloc_phys_address, malloc_virt_address, PTE_PRESENT | PTE_WRITABLE | PTE_USER);

        malloc_head = (malloc_node_t*)malloc_virt_address;
        malloc_head->size = PAGE_SIZE - sizeof(malloc_node_t);
        malloc_head->free = true;
        malloc_head->next = 0;
        malloc_head->prev = 0;

        total_malloc_pages = 1;
}

bool print_registers() {
        kprintf("REGISTERS      |MEM LOCATION\n");
        uint32_t num = 0;
        __asm__ volatile("mov %%EAX, %0" : "=r"(num));
        kprintf("EAX:           |0x%x\n", num);
        __asm__ volatile("mov %%EBX, %0" : "=r"(num));
        kprintf("EBX:           |0x%x\n", num);
        __asm__ volatile("mov %%ECX, %0" : "=r"(num));
        kprintf("ECX:           |0x%x\n", num);
        __asm__ volatile("mov %%EDX, %0" : "=r"(num));
        kprintf("EDX:           |0x%x\n", num);
        __asm__ volatile("mov %%ESI, %0" : "=r"(num));
        kprintf("ESI:           |0x%x\n", num);
        __asm__ volatile("mov %%EDI, %0" : "=r"(num));
        kprintf("EDI:           |0x%x\n", num);
        __asm__ volatile("mov %%CS, %0" : "=r"(num));
        kprintf("CS:            |0x%x\n", num);
        __asm__ volatile("mov %%DS, %0" : "=r"(num));
        kprintf("DS:            |0x%x\n", num);
        __asm__ volatile("mov %%ES, %0" : "=r"(num));
        kprintf("ES:            |0x%x\n", num);
        __asm__ volatile("mov %%SS, %0" : "=r"(num));
        kprintf("SS:            |0x%x\n", num);
        kprintf("\n");
        return true;
}
