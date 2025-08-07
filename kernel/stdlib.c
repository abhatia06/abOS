#include "stdint.h"
//#include "stdlib.h"

/*
 * I SHOULD PUT A DISCLAIMER HERE:
 * technically, these are WRONG. Malloc and free are NOT supposed to be syscalls, instead, they should be part of the stdlib
 * library (which they are), and they should be available for the user to use. BUT, for right now, I am not doing that. Currently,
 * I only care about my implementation of malloc WORKING. Eventually, I will move it over to be existing in the user space, and
 * then have sbrk and brk be the only syscalls it executes.
 */

void* malloc(uint32_t size) {
	void* ptr;

	__asm__ volatile("int $0x80" : "=r"(ptr) : "a"(2), "b"(size)); 

	return ptr;  
}

void free(void *ptr) {
	// the 4th syscall can handle it all lol
	__asm__ volatile("int $0x80" : : "a"(3), "b"(ptr));
}	



