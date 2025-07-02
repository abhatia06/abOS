// This is basically a copy of stdio.c and stdio.h, but it is a "lite" version for the prekernel to use. It has very basic and limited
// functionality, hence why it is called "lite". Is this file needed? Not really.. I could've probably gotten away with recompiling stdio.c, and putting it in 
// the other linker script. At worse, I would likely have to add macros to global_addresses to save the global variables for the screenbuffer, screex, and screeny, 
// but whatever. Who cares, this doesn't take up a whole lot space.

#include "stdint.h"
#include "printlite.h"

const unsigned WIDTH = 80;
const unsigned HEIGHT = 25;
const uint8_t DEFAULT_COLOR = 0x07;

uint8_t* screenBuffer = (uint8_t*)0xB8000;
int screenX = 0, screenY = 0;

void printchar(int x, int y, char c) {
	screenBuffer[2 * (y * WIDTH + x)] = c;
}

void colorassign(int x, int y, uint8_t color) {
	screenBuffer[2 * (y * WIDTH + x) + 1] = color;
}

void clear() {
        for(int y = 0; y < HEIGHT; y++) {
                for(int x = 0; x < WIDTH; x++) {
                        printchar(x, y, '\0');
                        colorassign(x, y, DEFAULT_COLOR);
                }
        }

        screenX = 0;
        screenY = 0;
}

void printC(char c) {
        switch(c) {
                case '\n' :
                        screenX = 0;
                        screenY++;
                        break;

		default :
			printchar(screenX, screenY, c);
		        screenX++;
			break;
	}
}

void printS(const char* str) {
	while(*str) {
		printC(*str);
		str++;
	}
}

