#pragma once

// based off OSDev ELF tutorial: https://wiki.osdev.org/ELF_Tutorial

#include "../stdint.h"

typedef uint16_t Elf32_Half;    // unsigned 16-bit int
typedef uint32_t Elf32_Off;     // unsigned offset
typedef uint32_t Elf32_Addr;    // unsigned address
typedef uint32_t Elf32_Word;    // unsigned int
typedef int32_t Elf32_Sword;    // signed int
typedef uint64_t Elf32_Xword;   // unsigned 64-bit int
typedef int64_t Elf32_Sxword;   // signed 64-bit int

#define ELF_NIDENT 16

enum ELF_type {
        ET_NONE,
        ET_REL,
        ET_EXEC
};

// will really only have to deal with program headers & elf headers for loading in files I believe

// ELF elf headers:


// Section header table:
//

// Program header table:

typedef struct {
        uint8_t e_ident[ELF_NIDENT];
        Elf32_Half e_type;
        Elf32_Half e_machine;           // EM_386
        Elf32_Word e_version;           // either 0 or 1 for invalid/valid
        Elf32_Addr e_entry;             // starting virtual address
        Elf32_Off e_phoff;              // program header table's file offset in bytes
        Elf32_Off e_shoff;              // section header table's file offset in bytes
        Elf32_Word e_flags;             // processor-specific flags
        Elf32_Half e_ehsize;            // ELF header's size in bytes
        Elf32_Half e_phentsize;         // program header's size in bytes
        Elf32_Half e_phnum;             // number of entries in program header table
        Elf32_Half e_shentsize;         // section header's size in bytes
        Elf32_Half e_shnum;             // number of entries in section header table
        Elf32_Half e_shstrndx;          //
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

// ELF section header:

typedef struct {
        Elf32_Word sh_name;
        Elf32_Word sh_type;
        Elf32_Word sh_flags;
        Elf32_Addr sh_addr;
        Elf32_Off sh_offset;
        Elf32_Word sh_size;
        Elf32_Word sh_link;
        Elf32_Word sh_info;
        Elf32_Word sh_addralign;
        Elf32_Word sh_entsize;
} Elf32_Shdr;

// need to do more research on these functions below
bool elf_check_file(Elf32_Ehdr* hdr);
bool elf_check_supported(Elf32_Ehdr* hdr);
static inline void* elf_load_rel(Elf32_Ehdr* hdr);
void* elf_load_file(void* file);
void* load_segment_to_memory(Elf32_Phdr* p_hdr, void* mem_segment, int elf_fd);
