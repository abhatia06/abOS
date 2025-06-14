#include "stdio.h"


int cursor_pos = 0;
void putc(char c) {
	_x86_Video_WriteCharTeletype(c, 0, cursor_pos);
	cursor_pos += 2;
}

void puts(const char* str) {

	while(*str) {

		putc(*str);
		str++;
	}
}
