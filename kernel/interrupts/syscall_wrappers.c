#include "syscall_wrappers.h"
#include "syscalls.h"
#include "../stdint.h"

int32_t open(char* path, flags_t flags) {
	int32_t result = -1;

	__asm__ volatile("int $0x80" : "=a"(result) : "a"(4), "b"(path), "c"(flags) : "memory");

	return result; 
}

int32_t close(int32_t file_descriptor) {
	int32_t result = -1;

	__asm__ volatile("int $0x80" : "=a"(result) : "a"(5), "b"(file_descriptor) : "memory");

	return result;
}

// also might change in the future? Might not require a buffer, who knows. I'll see.
// if we had like stdout, stderr, and stdin, if we wanted to be able to print to screen, we would call write() with
// fd = 1, or 0, or 2, whatever fd we assign stdout to. If an error popped up, it would write to stderr, and if we
// wanted to input a file, we'd use stdin. But uh. I don't really wanna deal with the standard stream stuff just yet
int32_t write(int32_t file_descriptor, void* buffer, uint32_t length) {
        int32_t result = -1;

        __asm__ volatile("int $0x80" : "=a"(result) : "a"(6), "b"(file_descriptor), "c"(buffer), "d"(length) : "memory");
        return result;
}

// read is supposed to read the contents of a file and write to a buffer, and then we can use that buffer for other syscalls (write)
int32_t read(int32_t file_descriptor, void* buffer, uint32_t length) {
        int32_t result = -1;

        __asm__ volatile("int $0x80" : "=a"(result) : "a"(7), "b"(file_descriptor), "c"(buffer), "d"(length) : "memory");
	
        return result;
}
