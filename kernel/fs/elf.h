#pragma once

//INCOMPLETE. I WILL WORK ON THIS IN THE FUTURE, FOR NOW, I WILL ONLY BE LOADING IN RAW BIN FILES 
// based off OSDev ELF tutorial: https://wiki.osdev.org/ELF_Tutorial

#include "../stdint.h"

typedef uint16_t Elf32_Half;	// unsigned 16-bit int
typedef uint32_t Elf32_Off;	// unsigned offset
typedef uint32_t Elf32_Addr;	// unsigned address
typedef uint32_t Elf32_Word; 	// unsigned int
typedef int32_t Elf32_Sword; 	// signed int 
typedef uint64_t Elf32_Xword;	// unsigned 64-bit int 
typedef int64_t Elf32_Sxword;	// signed 64-bit int

#define ELF_NIDENT 16

// will really only have to deal with program headers & elf headers for loading in files I believe

// ELF elf headers:

typedef struct {
	uint8_t e_ident[ELF_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff; 
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;

// ELF program header:

typedef struct {
        Elf32_Word p_type;
        Elf32_Off p_offset;
        Elf32_Addr p_vaddr;
        Elf32_Addr p_paddr;
        Elf32_Word p_filesz;
        Elf32_Word p_memsz;
        Elf32_Word p_flags;
        Elf32_Word p_align;
} Elf32_Phdr;
