#include "stdint.h"
//#include "stdlib.h"

void testertesting123() {
        __asm__ volatile("int $0x80" : : "a"(1));
}

void* malloc(uint32_t size) {
	void* ptr;

	__asm__ volatile("int $0x80" : "=r"(ptr) : "a"(2), "b"(size)); 

	return ptr;  
}

void free(void *ptr) {
	// the 4th syscall can handle it all lol
	__asm__ volatile("int $0x80" : : "a"(3), "b"(ptr));
}	

void exit(int32_t fd) {
        __asm__ volatile("int $0x80" : : "a"(8), "b"(fd));
}


