#include "stdint.h"
#ifndef x86_H
#define x86_H

void _x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotient, uint32_t* remainderOut);

void _x86_Video_WriteCharTeletype(char c, uint8_t page, uint8_t pos);

#endif
