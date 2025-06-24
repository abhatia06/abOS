// Admittedly, this header file is very different from how I usually do my header files. However, since I only use this file in exceptions.c, it doesn't really matter.

#pragma once

// Case keys. The keyboard port does not differentiate between capital or lowercase letters. We must do that ourselves
#define LSHIFT_MAKE     0x2A
#define LSHIFT_BREAK    0xAA
#define RSHIFT_MAKE     0x36
#define RSHIFT_BREAK    0xB6
#define LCTRL_MAKE      0x1D
#define LCTRL_BREAK     0x9D
#define ENTER           0x1C 
#define CAPSLOCK	0x3A

typedef struct {
        uint8_t key;
        bool shift;
        bool ctrl;
        bool caps;
} key_info_t;

// I was too lazy to make another keymap but this time replace the scan codes for the numbers 1-9 be for their respective shift keys. So, instead, I abused the fact
// that every int in ASCII (1-9) begins with 3 (0x30 = 0, 0x31 = 1, 0x32 = 2, you get the pattern). So, all we have to do is just take the number we have, subtract 
// 0x30, and then compare it to an array, which is num_row_shifts. And then boom, it works.
const uint8_t* num_row_shifts = (const uint8_t*)")!@#$%^&*(";

// Mapping scan codes to their respective characters. Find more info at: https://wiki.osdev.org/PS/2_Keyboard
char keymap[128] = {
	[0x00] = 0,
	[0x01] = 27,
	[0x02] = '1',
	[0x03] = '2',
	[0x04] = '3',
	[0x05] = '4',
	[0x06] = '5',
	[0x07] = '6',
	[0x08] = '7',
	[0x09] = '8',
	[0x0A] = '9',
	[0x0B] = '0',
	[0x0C] = '-',
	[0x0D] = '=',
	[0x0E] = '\b',	// my printf implementation doesn't support \b or \t, so we'll have to implement them (booorrrinnggg!!)
	[0x0F] = '\t',
	[0x10] = 'q',
	[0x11] = 'w',
	[0x12] = 'e',
	[0x13] = 'r',
	[0x14] = 't',
	[0x15] = 'y',
	[0x16] = 'u',
	[0x17] = 'i',
	[0x18] = 'o',
	[0x19] = 'p',
	[0x1A] = '[',
	[0x1B] = ']',
	[0x1C] = '\n',	// enter key
	[0x1D] = 0,	// left control, we manage that
	[0x1E] = 'a',
	[0x1F] = 's',
	[0x20] = 'd',
	[0x21] = 'f',
	[0x22] = 'g',
	[0x23] = 'h',
	[0x24] = 'j',
	[0x25] = 'k',
	[0x26] = 'l',
	[0x27] = ';',
	[0x28] = '\'',
	[0x29] = '`',
	[0x2A] = 0,	// left shift, we'll manage that
	[0x2B] = '\\',
	[0x2C] = 'z',
	[0x2D] = 'x',
	[0x2E] = 'c',
	[0x2F] = 'v',
	[0x30] = 'b',
	[0x31] = 'n',
	[0x32] = 'm',
	[0x33] = ',',
	[0x34] = '.',
	[0x35] = '/',
	[0x36] = 0, 	//Right shift
	[0x37] = '*',
	[0x38] = 0, 	// Left Alt
	[0x39] = ' ',
	[0x3A] = 0, 	// caps lock
	
	// There are more, but they're either function keys, or break keys, and we don't really need them
};

