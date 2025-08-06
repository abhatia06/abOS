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
		temp++;
        }
	if((char)c == '\0') {
		return str;
	}
	return p;
}

char* strncpy(char* dst, const char* src, uint32_t n) {
        char* temp;
        temp = dst;
        for(uint32_t i = 0; i < n; i++) {
                *dst++ = *src++;
        }
        return temp;
}

void* memset(void* ptr, uint8_t value, uint32_t length) {

        unsigned char* ptr_byte = (unsigned char*)ptr;
        for(uint32_t i = 0; i < length; i++) {
                ptr_byte[i] = (unsigned char)value;
	}
        return ptr;
}

char* strcpy(char* dst, const char* src) {
        char* temp;
        temp = dst;
        while(true) {
                *dst = *src;
		if(*src == '\0') {
			break;
		}
		dst++;
		src++;
        }
        return temp;
}

// assume dst and src are not null
void* memcpy(void* dst, void* src, uint32_t size) {
        char* dest = (char*)dst;
        char* source = (char*)src;

        for(uint32_t i = 0; i < size; i++) {
                dest[i] = source[i];
        }

        return dest;
}
