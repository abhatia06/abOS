#pragma once

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;  // int is usually 32-bits (https://learn.microsoft.com/en-us/cpp/c-language/type-int?view=msvc-170)
typedef signed long int int64_t;    // 32-bit uses long long int to define 64-bit, because long int not long enough. 64-bit uses long int
typedef unsigned long long int uint64_t; // for some reason, 64-bit also uses long long int for uint64_t????? I don't really get why but ok ig


typedef uint8_t bool;
#define true 1
#define false 0
