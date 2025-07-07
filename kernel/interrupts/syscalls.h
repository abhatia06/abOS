#include "../stdio.h"
#include "../stdint.h"

#pragma once

#define MAX_SYSCALLS 2

void syscall_handler();

void sys_test1();
void sys_test2();

extern void* syscalls[MAX_SYSCALLS];
