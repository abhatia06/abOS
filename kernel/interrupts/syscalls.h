#include "../stdio.h"
#include "../stdint.h"

#pragma once

#define MAX_SYSCALLS 9

void syscall_handler();

void sys_test1();
void sys_test2();
void sys_malloc();
void sys_free();
void sys_open();
void sys_close();
void sys_write();
void sys_read();
void sys_close();

extern void* syscalls[MAX_SYSCALLS];
