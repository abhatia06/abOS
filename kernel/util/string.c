#include "string.h"
#include "../stdint.h"
#include "../stdio.h"

int strcmp(const char* str1, const char* str2) {
	while(*s1 && (*s1 == *s2)) {
		str1++;
		str2++;
	}

	return *(const unsigned char*)str1 - *(const unsigned char*)str2
}

long int strlen(const char* str1) {
	long int length = 0;
	while(*str1) {
		length++;
		str1++;
	}
}
