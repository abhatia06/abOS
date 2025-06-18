#include "stdio.h"
#include "x86.h"

int cursor_pos = 0;
void putc(char c) {
	_x86_Video_WriteCharTeletype(c, 0, cursor_pos);	// The 2nd argument isn't used JUST YET. It is for memory paging
	cursor_pos += 2;	// We add 2 to cursor_pos because 0xB8000 = top left character, 0xB8002 = character after that
}

void puts(const char* str) {

	while(*str) {

		putc(*str);
		str++;
	}
}

/*
 * In the code below I will be implementing printf() according to the format specified by the regular printf()
 * found in the stdio.h library in the actual C language. According to the OSDev wiki, I am basically forced to use
 * the va_list. Thankfully, I don't have to implement va_list, because it can be found in the freestanding
 * <stdarg.h>, so, woohoo! Also I used Nanobyte's implementation, but he doesn't use va_args, and he's in 16 bit mode, so I changed it
 * just a slight to make it work for me.
 */

#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_LENGTH_SHORT 2
#define PRINTF_STATE_LENGTH_LONG 3
#define PRINTF_STATE_SPEC 4

#define PRINTF_LENGTH_DEFAULT 0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4

const char g_HexChars[] = "0123456789abcdef";

void printf_number(va_list* args, int length, bool sign, int base10) {
        char buffer[32];
        unsigned long long number;
        int number_sign = 1;
        int pos = 0;

        switch (length) {
                case PRINTF_LENGTH_SHORT_SHORT:
                case PRINTF_LENGTH_SHORT:
                case PRINTF_LENGTH_DEFAULT:
                        if(sign) {
                                int n = va_arg(*args, int);
                                if(n < 0) {
                                        n = -n;
                                        number_sign = -1;
                                }
                                number = (unsigned long long)n;
                        }
                        else {
                                number = va_arg(*args, unsigned int);
                        }
                        break;

                case PRINTF_LENGTH_LONG:
                        if(sign) {
                                long n = va_arg(*args, long);
                                if(n < 0) {
                                        n = -n;
                                        number_sign = -1;
                                }
                                number = (unsigned long long) n;
                        }
                        else {
                                number = va_arg(*args, unsigned long);
                        }
                        break;

                case PRINTF_LENGTH_LONG_LONG:
                        if(sign) {
                                long long n = va_arg(*args, long long);
                                if(n < 0) {
                                        n = -n;
                                        number_sign = -1;
                                }
                                number = (unsigned long long)n;
                        }
                        else {
                                number = va_arg(*args, unsigned long long);
                        }
                        break;
        }

        do {
                uint32_t rem;
                _x86_div64_32(number, base10, &number, &rem);
                buffer[pos++] = g_HexChars[rem];
        } while (number > 0);

        if(sign && number_sign < 0) {
                buffer[pos++] = '-';
        }

        while(--pos >= 0) {
                putc(buffer[pos]);
        }
}

void kprintf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int state = PRINTF_STATE_NORMAL;
        int length = PRINTF_LENGTH_DEFAULT;
        int base10 = 10;

        bool sign = false;

        while(*fmt) {

                switch(state) {
                        case PRINTF_STATE_NORMAL:
                                switch(*fmt) {
                                        case '%' : state = PRINTF_STATE_LENGTH;
                                                   break;
                                        default : putc(*fmt);
                                                  break;
                                }
                                break;

                        case PRINTF_STATE_LENGTH:
                                switch(*fmt) {
                                        case 'h' : length = PRINTF_LENGTH_SHORT;
                                                   state = PRINTF_STATE_LENGTH_SHORT;
                                                   break;
                                        case 'l' : length = PRINTF_LENGTH_LONG;
                                                   state = PRINTF_STATE_LENGTH_LONG;
                                                   break;
                                        default : goto PRINTF_STATE_SPEC_;
                                }
                                break;

                        case PRINTF_STATE_LENGTH_SHORT:
                                if(*fmt == 'h') {
                                        length = PRINTF_LENGTH_SHORT_SHORT;
                                        state = PRINTF_STATE_SPEC;
                                }
                                else {
                                        goto PRINTF_STATE_SPEC_;
                                }
                                break;

                        case PRINTF_STATE_LENGTH_LONG:
                                if(*fmt == 'l') {
                                        length = PRINTF_LENGTH_LONG_LONG;
                                        state = PRINTF_STATE_SPEC;
                                }
                                else {
                                        goto PRINTF_STATE_SPEC_;
                                }
                                break;

                        case PRINTF_STATE_SPEC:
			PRINTF_STATE_SPEC_:
                                switch(*fmt) {
                                        case 'c' : putc((char)va_arg(args, int));
                                                   break;

                                        case 's' : puts(va_arg(args, const char*)); break;

                                        case '%': putc('%');
                                                  break;

                                        case 'd':
                                        case 'i': base10 = 10; sign = true;
                                                  printf_number(&args, length, sign, base10);
                                                  break;

                                        case 'u': base10 = 10, sign = false;
                                                  printf_number(&args, length, sign, base10);
                                                  break;

                                        case 'X':
                                        case 'x':
                                        case 'p': base10 = 16; sign = false;
                                                  printf_number(&args, length, sign, base10);
                                                  break;

                                        case 'o': base10 = 8; sign = false;
                                                  printf_number(&args, length, sign, base10);
                                                  break;

                                        default : break;
                                }
                                state = PRINTF_STATE_NORMAL;
                                length = PRINTF_LENGTH_DEFAULT;
                                base10 = 10;
                                sign = false;
                                break;
                }
                fmt++;
        }
        va_end(args);
}
