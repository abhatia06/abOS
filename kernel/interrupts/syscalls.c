#include "syscalls.h"
#include "../stdio.h"
#include "../stdint.h"
#include "../stdlib.h"


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

void* syscalls[MAX_SYSCALLS] = {
        sys_test1,
        sys_test2,
        sys_malloc,
        sys_free,
};

// This stuff below is defined by the OSDev wiki in their syscall page: https://wiki.osdev.org/System_Calls
__attribute__((naked))  void syscall_handler() {
        __asm__ volatile (".intel_syntax noprefix\n"

                          ".equ MAX_SYSCALLS, 4\n"
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
