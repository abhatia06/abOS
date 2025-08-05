#include "syscalls.h"
#include "../stdio.h"
#include "../stdint.h"
#include "../stdlib.h"
#include "../memory/malloc.h"
#include "../fs/fs.h"
#include "../fs/fs_commands.h"
#include "../memory/virtual_memory_manager.h"
#include "../memory/physical_memory_manager.h"
#include "../util/string.h"
#include "syscall_wrappers.h"

extern open_file_t* open_file_table;
extern inode_t* open_inode_table;
extern uint32_t current_open_files;
extern uint32_t current_open_inodes;
extern uint32_t file_virtual_address;
// The main system call will be int 0x80

void sys_test1() {
        kprintf("Test syscall 1\n");
}


void sys_test2() {
        kprintf("Test syscall 2\n");
}

void sys_malloc() {
        uint32_t bytes;

        __asm__ volatile("mov %%EBX, %0" : "=b"(bytes));

        if(!malloc_head) {
                void* ptr1 = malloc_init();
                __asm__ volatile ("mov %0, %%EAX" : : "r"(ptr1));
                return;
        }

        void* ptr = malloc_next(bytes);
        merge_free_blocks();
        __asm__ volatile("mov %0, %%EAX" : : "r"(ptr));
}

void sys_free() {
        void* ptr = 0;

        __asm__ volatile("mov %%EBX, %0" : "=b"(ptr));

        malloc_free(ptr);
}

void sys_open() {
        int32_t file_descriptor = -1;
        char* path = 0;
        __asm__ volatile("mov %%EBX, %0" : "=r"(path));         // arg 1 (EBX) contains path
        uint32_t flags = 0;
        __asm__ volatile("mov %%ECX, %0" : "=r"(flags));        // arg 2 (ECX) contains flags

        // get file from file path
        inode_t file_inode = get_inode(path);

        // if file doesn't exist yet, create it if given proper flag
        if(file_inode.i_number == 0) {
                if(!(flags & O_CREAT)) {
                        // after reading the gnu manual I believe that "" : : "a"(file_descriptor) will also work
                        __asm__ volatile("mov %0, %%EAX" : : "r"(file_descriptor));
                        return;
                }
                else {
                        file_inode = create_file(path);
                        if(file_inode.i_number == 0) {
                                __asm__ volatile("mov %0, %%EAX" : : "r"(file_descriptor));
                                return;
                        }
                }
        }

        // add to open inode table, if already in table, increment max count
        inode_t* temp = open_inode_table;
        uint32_t index = 0;
        while(index < 256 && temp->i_number != file_inode.i_number) {
                index++;
                temp++;
                if(file_inode.i_number == temp->i_number) {
                        temp->max_count++;
                        break;  // kind of useless, as it'll break automatically, but whatever
                }
        }
        if(index == 256) {
                temp = open_inode_table;
                index = 0;
                while(index < 256 && temp->i_number != 0) {
                        index++;
                        temp++;
                }
                *temp = file_inode;
        }
        current_open_inodes++;

        // add to open file table, if already in file table, increase max count
        open_file_t* temp_file = open_file_table;
        index = 0;
        while(index < 256 && file_inode.i_number != temp[temp_file->inode_index].i_number) {
                index++;
                temp_file++;
                if(file_inode.i_number == temp[temp_file->inode_index].i_number) {
                        temp_file->max_count++;
                }
        }
        if(index == 256) {
                temp_file = open_file_table;
                index = 0;
                while(index < 256 && temp[temp_file->inode_index].i_number != 0) {
                        index++;
                        temp_file++;
                }
                temp_file->inode_index = index;
                temp_file->lseek = 0;
                temp_file->address = 0;
                temp_file->max_count = 1;
                temp_file->flags = flags;
        }
        current_open_files++;
        
        uint32_t size = bytes_to_blocks(temp[temp_file->inode_index].size);
        if(size == 0) {
                size = 1;
        }
        for(uint32_t i = 0; i < size; i++) {
                uint32_t phys_address = (uint32_t)allocate_blocks(1);
                map_address(directory, phys_address, file_virtual_address, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
                file_virtual_address += PAGE_SIZE;
                temp_file->pages_allocated++;
        }

        // load file into memory
        if(!load_file(&temp[temp_file->inode_index], (uint32_t)temp_file->address)) {
                file_descriptor = -1;
        }

        __asm__ volatile("mov %0, %%EAX" : : "r"(file_descriptor));     // EAX contains file descriptor
}

void sys_close() {
        int32_t file_descriptor = -1;
        __asm__ volatile("mov %%EBX, %0" : "=r"(file_descriptor));      // we just need file descriptor I believe
        // close should be similar to sys_open hopefully
        // find file via open inode table w/ inode_index
        // if file doesn't exist, do nothing and return
        // if file does exist, look through open inode table, decrement max count
        // if file does exist, look through open file table, decrement max count
        // unload from memory (save file)

}

void* syscalls[MAX_SYSCALLS] = {
        sys_test1,
        sys_test2,
        sys_malloc,
        sys_free,
        sys_open,
        sys_close,
};

//TODO: once I make a file system, create sys_write(), sys_open(), and sys_read(). (duh)

// This stuff below is defined by the OSDev wiki in their syscall page: https://wiki.osdev.org/System_Calls
__attribute__((naked))  void syscall_handler() {
        __asm__ volatile (".intel_syntax noprefix\n"

                          ".equ MAX_SYSCALLS, 6\n"
                          "cmp eax, MAX_SYSCALLS-1\n"
                          "ja invalid_syscall\n"

                          "push gs\n"
                          "push fs\n"
                          "push es\n"
                          "push ds\n"
                          "push ebp\n"
                          "push edi\n"
                          "push esi\n"
                          "push edx\n"
                          "push ecx\n"
                          "push ebx\n"
                          "push esp\n"
                          "call [syscalls+eax*4]\n"        // Each function pointer is 4 bytes, so it's not as simple as just syscalls+eax, (eax holds which syscall)
                          "add esp, 4\n"
                          "pop ebx\n"
                          "pop ecx\n"
                          "pop edx\n"
                          "pop esi\n"
                          "pop edi\n"
                          "pop ebp\n"
                          "pop ds\n"
                          "pop es\n"
                          "pop fs\n"
                          "pop gs\n"

                          "iretd\n"

                          "invalid_syscall:\n"
                          "mov eax, -1\n"
                          "iretd\n");
}
