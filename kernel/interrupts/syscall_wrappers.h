#pragma once

#include "../stdint.h"
#include "syscalls.h"

typedef enum {
	O_CREAT,	// create
	O_RDONLY,	// read only
	O_WRONLY,	// write only
	O_RDWR,		// read and write
} flags_t;

int32_t open(char* path, flags_t);
int32_t close(int32_t file_descriptor);
// might change in the future?
int32_t write(int32_t file_descriptor, void* buffer, uint32_t length);
int32_t read(int32_t file_descriptor, void* buffer, uint32_t length);
