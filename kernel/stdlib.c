#include "stdint.h"
#include "stdlib.h"

void* malloc(uint32_t size) {
	void* ptr;

	__asm__ volatile("int $0x80" : : "a"(3), "b"(size));
	__asm__ volatile("movl %%eax, %0" : "=r"(ptr));

	return ptr;  
}

void free(void *ptr) {
	// the 4th syscall can handle it all lol
	__asm__ volatile("int $0x80" : : "a"(4), "b"(ptr));
}	



