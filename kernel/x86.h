#pragma once

#include "stdint.h"

void _x86_div64_32(uint64_t number, uint32_t divisor, uint64_t* quotient, uint32_t* remainderOut);
void _x86_Video_WriteCharTeletype(char c, uint8_t page, uint8_t pos);
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);
void io_wait();
