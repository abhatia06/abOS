#ifndef STDIO_H
#define STDIO_H

void putc(char c);
void puts(const char* str);

extern void _x86_Video_WriteCharTeletype(char c, int color, int pos);

#endif
