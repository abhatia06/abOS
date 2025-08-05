#include "../stdio.h"
#include "../stdint.h"

#pragma once

#define MAX_SYSCALLS 6

void syscall_handler();

void sys_test1();
void sys_test2();
void sys_malloc();
void sys_free();
void sys_open();
void sys_close();

extern void* syscalls[MAX_SYSCALLS];
