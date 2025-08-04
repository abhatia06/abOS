#include "syscall_wrappers.h"
#include "syscalls.h"
#include "../stdint.h"

int32_t open(char* path, flags_t flags) {
	int32_t result = -1;

	__asm__ volatile("int $0x80" : "=a"(result) : "a"(5), "b"(path), "c"(flags) : "memory");

	return result; 
}

int32_t close(int32_t file_descriptor) {
	int32_t result = -1;

	__asm__ volatile("int $0x80" : "=a"(result) : "a"(6), "b"(file_descriptor) : "memory");

	return result;
}
