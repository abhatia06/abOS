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
	return length; 	// oops forgot
}

char* strrchr(char* str, int c)  {
        char* p = 0;
        char* temp = str;
        while(*temp != '\0') {
                if(*temp == (char)c) {
                        p = temp;
                }
                if(*temp++ == '\0') {
                        return (char*)p;
                }
        }
}

void* memset(void* ptr, uint8_t value, uint32_t length) {

        unsigned char* ptr_byte = (unsigned char*)ptr;
        for(uint32_t i = 0; i < length; i++) {
                ptr_byte[i] = (unsigned char)value;
	}
        return ptr;
}
