#include "stdio.h"
#include "x86.h"
#include <stdarg.h>
#include <stdbool.h>
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "interrupts/exceptions.h"
#include "memory/physical_memory_manager.h"

const unsigned VGA_WIDTH = 80;
const unsigned VGA_HEIGHT = 25;
const uint8_t DEFAULT_COLOR = 0x07;	// light gray color

uint8_t* g_ScreenBuffer = (uint8_t*)0xB8000;
int g_ScreenX = 0, g_ScreenY = 0;

void putchar(int x, int y, char c) {
        g_ScreenBuffer[2 * (y * VGA_WIDTH + x)] = c;
}

void putcolor(int x, int y, uint8_t color) {
        g_ScreenBuffer[2 * (y * VGA_WIDTH + x) + 1] = color;
}

int getchr(int x, int y) {
        return g_ScreenBuffer[2 * (y * VGA_WIDTH + x)];
}

uint8_t getcolor(int x, int y) {
        return g_ScreenBuffer[2 * (y * VGA_WIDTH + x) + 1];
}

void setcursor(int x, int y) {
        int pos = y * VGA_WIDTH + x;

        _x86_outb(0x3D4, 0x0F);
        _x86_outb(0x3D5, (uint8_t)(pos & 0xFF));
        _x86_outb(0X3D4, 0X0E);
        _x86_outb(0x3D5, (uint8_t)((pos >> 8) & 0XFF));
}

void scrollback(int lines) {
        for(int y = lines; y < VGA_HEIGHT; y++) {
                for(int x = 0; x < VGA_WIDTH; x++) {
                        putchar(x, y - lines, getchr(x, y));
                        putcolor(x, y - lines, getcolor(x, y));
                }
        }

        for(int y = VGA_HEIGHT - lines; y < VGA_HEIGHT; y++) {
                for(int x = 0; x < VGA_WIDTH; x++) {
                        putchar(x, y, '\0');
                        putcolor(x, y, DEFAULT_COLOR);
                }
        }

        g_ScreenY -= lines;
}

void clrscr() {
        for(int y = 0; y < VGA_HEIGHT; y++) {
                for(int x = 0; x < VGA_WIDTH; x++) {
                        putchar(x, y, '\0');
                        putcolor(x, y, DEFAULT_COLOR);
                }
        }

        g_ScreenX = 0;
        g_ScreenY = 0;
        setcursor(g_ScreenX, g_ScreenY);
}


void putc(char c) {
        switch(c) {
                case '\n' :
                        g_ScreenX = 0;
                        g_ScreenY++;
                        break;

                case '\t' :
                        for(int i = 0; i < 4 - (g_ScreenX % 4); i++) {
                                putc(' ');
                        }
                        break;

                case '\r' :
                        g_ScreenX = 0;
                        break;

		case '\b' : // in most printf cases, \b is supposed to be non-destructive, but ours is destructive b/c of our keyboard handler
                	if(g_ScreenX > 0) {
                        	g_ScreenX--;
                	}
                	else if(g_ScreenY > 0) { // if we're NOT at the first (0th) row, and we're at column 0 of the current row we're at, then go to the previous row
                        	g_ScreenY--;
                        	g_ScreenX = VGA_WIDTH - 1; // start at the very last column (might change this, though)
                	}
                	putchar(g_ScreenX, g_ScreenY, ' ');
                	break;
		
                default :
                        putchar(g_ScreenX, g_ScreenY, c);
                        g_ScreenX++;
                        break;
        }

        if(g_ScreenX >= VGA_WIDTH) {
                g_ScreenY++;
                g_ScreenX = 0;
        }
        if(g_ScreenY >= VGA_HEIGHT) {
                scrollback(1);
        }

        setcursor(g_ScreenX, g_ScreenY);
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

// EVERYTHING BELOW THIS COMMENT IS TEMPORARY. I WILL LIKELY MOVE IT ALL TO THEIR OWN RESPECTIVE FILES, BUT FOR  NOW, I AM LAZY.
char* readline() {
        input_pos = 0;
        input_buffer[0] = '\0';
        input_ready = false;

        while(!input_ready) {
                // loop here basically and wait (yes, we're basically doing polling lol)
        }
        return input_buffer;
}

typedef struct SMAP_entry {
        uint64_t base_address;
        uint64_t length;
        uint32_t type;
        uint32_t acpi;
}__attribute__((packed)) SMAP_entry_t;

// Eventually, once we enter user mode, I might make this a program, because why not?
void print_physical_memory() {
    uint32_t num_entries = *(uint32_t *)0x9000;
    SMAP_entry_t *entry = (SMAP_entry_t *)0x9004;

    for (uint32_t i = 0; i < num_entries; i++, entry++) {
        kprintf("Entry %u: base=0x%llx length=0x%llx type=%u",
            i,
            (unsigned long long)entry->base_address,
            (unsigned long long)entry->length,
            entry->type);

        switch (entry->type) {
            case 1: kprintf(" (Available)\r\n"); break;
            case 2: kprintf(" (Reserved)\r\n"); break;
            case 3: kprintf(" (ACPI Reclaim)\r\n"); break;
            case 4: kprintf(" (ACPI NVS Memory)\r\n"); break;
            default: kprintf(" (Reserved)\r\n"); break;
        }

	// Open up the regions that are considered type 1 (or available) by the SMAP
	if(entry->type == 1) {
        	initialize_memory_region((uint32_t)entry->base_address, (uint32_t)entry->length);
        }
    }
     
    kprintf("\r\n");
    kprintf("Total memory in bytes: ");
    entry--;
    // This basically does 0xfffc0000 + 0x40000 (from entry 5). This is equal to 0x100000000. But the limit for 32-bit
    // is 4 gigabytes is 0xffffffff, not 0x100000000, so we do length-1.
    uint64_t total_mem = entry->base_address+entry->length-1;
    kprintf("0x%llx\r\n", total_mem);
    // TODO: Print out memory manager block info (pages and whatnot) ((requires physical memory manager, cuz how tf would I know how many 4kb pages/frames I have lol))
    kprintf("Maximum number of blocks: %d\r\n", max_blocks);
    kprintf("Used blocks: %d\r\n", used_blocks);
}
