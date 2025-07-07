#include "syscalls.h"
#include "../stdio.h"
#include "../stdint.h"

// The main system call will be int 0x80

void sys_test1() {
	kprintf("Test syscall 1\n");
}


void sys_test2() {
	kprintf("Test syscall 2\n");
}

void* syscalls[MAX_SYSCALLS] = {
	sys_test1,
	sys_test2,
};

__attribute__((naked)) 	void syscall_handler() {
	__asm__ volatile (".intel_syntax noprefix\n"

			  ".equ MAX_SYSCALLS, 2\n"
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
			  "call [syscalls+eax*4]\n"
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
			  "iretd\n");
}
