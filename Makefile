ASM=nasm
SRC_DIR=bootloader
SRC_DIR2=kernel
SRC_DIR3=interrupts
SRC_DIR4=memory
BUILD_DIR=build
CCOMP=gcc
LD = ld
CFLAGS=-m32 -march=i386 -mgeneral-regs-only -ffreestanding -fno-pic -fno-pie -nostdlib -fno-stack-protector -mpreferred-stack-boundary=2 -fno-builtin -ffunction-sections -fdata-sections -O0 -Wall -c -g

KERNEL_SECTORS= $(shell echo $$(( ( $(shell stat -c%s $(BUILD_DIR)/kernel.bin ) + 511 ) / 512 )))
PREKERNEL_SECTORS = $(shell echo $$(( ( $(shell stat -c%s $(BUILD_DIR)/prekernel.bin ) + 511 ) / 512 )))
.PHONY: all disk_image kernel bootloader disk clean always run

disk_image: $(BUILD_DIR)/os-image.img

# Disk Image 
# format_disk creates a 1.44MB disk image called os-image.img that is formatted like so:
# BOOTBLOCK | SUPERBLOCK | INODE BITMAP | DATA BITMAP | INODE TABLE . . . | DATA BLOCKS . . . |
$(BUILD_DIR)/os-image.img: bootloader kernel disk
        $(BUILD_DIR)/format_disk


disk: $(BUILD_DIR)/format_disk

$(BUILD_DIR)/format_disk: format_disk.c
        gcc -std=c17 -Wall -Wextra -Wpedantic -o $(BUILD_DIR)/format_disk format_disk.c

$(BUILD_DIR)/main.bin: $(SRC_DIR)/main.s
        $(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin


#
#bootloader
#
bootloader: $(BUILD_DIR)/boot.bin

$(BUILD_DIR)/boot.bin: always
        $(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/newboot.bin
        $(ASM) $(SRC_DIR)/bootstage2.s -f bin -o $(BUILD_DIR)/bootstage2.bin


kernel: $(BUILD_DIR)/kernel.bin

#
#kernel
#
$(BUILD_DIR)/kernel.elf: always
        $(ASM) $(SRC_DIR2)/kernel.s -f elf32 -o $(BUILD_DIR)/kernel.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/kernel.c -o $(BUILD_DIR)/kernelC.o
        $(ASM) $(SRC_DIR2)/x86.s -f elf32 -o $(BUILD_DIR)/x86.o
        $(ASM) $(SRC_DIR2)/$(SRC_DIR3)/idt.s -f elf32 -o $(BUILD_DIR)/idt_stubs.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR3)/idt.c -o $(BUILD_DIR)/idt.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR3)/pic.c -o $(BUILD_DIR)/pic.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR3)/exceptions.c -o $(BUILD_DIR)/exceptions.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/stdio.c -o $(BUILD_DIR)/stdio.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR4)/physical_memory_manager.c -o $(BUILD_DIR)/physical_memory_manager.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/util/string.c -o $(BUILD_DIR)/string.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR4)/virtual_memory_manager.c -o $(BUILD_DIR)/virtual_memory_manager.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/prekernel.c -o $(BUILD_DIR)/prekernel.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/printlite.c -o $(BUILD_DIR)/printlite.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/memory/malloc.c -o $(BUILD_DIR)/malloc.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR3)/syscalls.c -o $(BUILD_DIR)/syscalls.o
        $(CCOMP) $(CFLAGS) build/test.c -o build/elftesting.o
        $(LD) -m elf_i386 -T programlink.ld -o build/bin/elftesting.bin --oformat binary build/elftesting.o build/stdlib.o
        $(LD) -m elf_i386 -T link.ld -o $(BUILD_DIR)/bin/kernel.bin --oformat binary $(BUILD_DIR)/kernel.o $(BUILD_DIR)/kernelC.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/x86.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/exceptions.o $(BUILD_DIR)/idt_stubs.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/physical_memory_manager.o $(BUILD_DIR)/string.o $(BUILD_DIR)/virtual_memory_manager.o $(BUILD_DIR)/syscalls.o $(BUILD_DIR)/malloc.o
        $(LD) -m elf_i386 -T kernelLink.ld -o $(BUILD_DIR)/bin/prekernel.bin --oformat binary $(BUILD_DIR)/prekernel.o $(BUILD_DIR)/virtual_memory_manager.o $(BUILD_DIR)/physical_memory_manager.o $(BUILD_DIR)/string.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/x86.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/exceptions.o $(BUILD_DIR)/idt_stubs.o


#
# this is useless as I only recently found out that you can directly link to .bin instead of .elf
# and it'll work just fine (I don't know why I thought it wouldn't work??? Maybe it fixed something
# randomly in the past?)
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
#        @echo "Kernel ELF size:" && stat -c%s $(BUILD_DIR)/kernel.elf
#        objcopy -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/bin/kernel.bin
#        @echo "Kernel BIN size:" && stat -c%s $(BUILD_DIR)/kernel.bin
#        objcopy -O binary $(BUILD_DIR)/prekernel.elf $(BUILD_DIR)/bin/prekernel.bin


#
#always
#
always:
        mkdir -p $(BUILD_DIR)


clean:
        rm -rf $(BUILD_DIR)/*

run:
        qemu-system-i386 -m 128M -drive format=raw,file=build/os-image.img,if=ide,index=0,media=disk

runDebug:
        qemu-system-i386 -m 128M -drive format=raw,file=build/os-image.img,if=ide,index=0,media=disk -s -S
