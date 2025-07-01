#pragma once

void clrscr();
void putc(char c);
void puts(const char* str);
void putchar(int x, int y, char c);
void kprintf(const char* fmt, ...);
char* readline();
int get_cursor_pos();
void print_physical_memory();
void print_memmap_command();
