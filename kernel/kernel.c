#include "stdint.h"
#include "stdio.c"

void main() {
	const char* not_really_far_string = "far string";
	puts("Hello World from C!");
	kprintf("Formatted %% %c %s %ls" 'a', "string", not_really_far_string);
	kprintf("Formatted %d %i %x %p %o %hd %hi %hhu %hhd", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (unsigned char)20, (signed char)-10);
	kprintf("Formatted %ld %lx %lld %llx", -100000000l, 0xdeadbeeful, 10200300400ll, 0xdeadbeeffeebdaedull);
	return;
}
