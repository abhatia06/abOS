# Because I do not have a lot of knowledge working with Makefiles (besides 1 course I took in university), a LOT of this code is from Nanobyte's OS. There are some changes
# I made, though, and I have slowly come around to understanding this Makefile code.

ASM=nasm
SRC_DIR=bootloader
SRC_DIR2=kernel
SRC_DIR3=interrupts
BUILD_DIR=build
CCOMP=gcc
LD = ld

# The CFLAGS below are for compiling my kernel.c. For some reason, GCC assumes code is aligned to 16-bytes, because most modern OS, like Windows and Linux are. I'm 
# not making a normal OS, and I'm certainly not on 64-bit, so I have to use the 32-bit version, which is 4 bytes, so we use -mpreferred-stack-boundary=2. The other
# flags are there for other reasons, which I will explain later (im too lazy).
CFLAGS=-m32 -march=i386 -mgeneral-regs-only -ffreestanding -fno-pic -fno-pie -nostdlib -fno-stack-protector -mpreferred-stack-boundary=2 -fno-builtin -ffunction-sections -fdata-sections -O0 -Wall -c -g

KERNEL_SECTORS= $(shell echo $$(( ( $(shell stat -c%s $(BUILD_DIR)/kernel.bin ) + 511 ) / 512 )))

.PHONY: all floppy_image kernel bootloader clean always run

floppy_image: $(BUILD_DIR)/main_floppy.img

#
#Floppy Image
#
$(BUILD_DIR)/main_floppy.img: bootloader kernel
        dd if=/dev/zero of=$(BUILD_DIR)/os-image.img bs=512 count=2880
        #Loads the bootloader (boot.bin) into sector 0
        dd if=$(BUILD_DIR)/boot.bin of=$(BUILD_DIR)/os-image.img bs=512 count=1 conv=notrunc
        #Loads the 2nd stage bootloader into sector 1
        dd if=$(BUILD_DIR)/boot2.bin of=$(BUILD_DIR)/os-image.img bs=512 seek=1 conv=notrunc
        #Loads the kernel into sector 2
        dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/os-image.img bs=512 seek=2 count=$(KERNEL_SECTORS) conv=notrunc

        $echo $(KERNEL_SECTORS)        # completely useless, but for in the future, whenever such issues, arise, I will know to update the 
                                       # number of sectors that we're reading from disk onto memory in boot.s

$(BUILD_DIR)/main.bin: $(SRC_DIR)/main.s
        $(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin


#
#bootloader
#
bootloader: $(BUILD_DIR)/boot.bin

$(BUILD_DIR)/boot.bin: always
        $(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin
        $(ASM) $(SRC_DIR)/boot2.s -f bin -o $(BUILD_DIR)/boot2.bin


kernel: $(BUILD_DIR)/kernel.bin

#
#kernel.
# Side note: The makefile is starting to get a bit big, and I've seen other OS projects split the task of making everything into separate makefiles
# like one in the kernel, one in the bootloader, etc. I might do that too soon enough, and I'll also place link loaders in each one there too, so
# this main makefile, in the src folder, won't be so cluttered and large.
#
$(BUILD_DIR)/kernel.elf: always
        $(ASM) $(SRC_DIR2)/kernel.s -f elf32 -o $(BUILD_DIR)/kernel.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/kernel.c -o $(BUILD_DIR)/kernelC.o
        $(ASM) $(SRC_DIR2)/x86.s -f elf32 -o $(BUILD_DIR)/x86.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/stdio.c -o $(BUILD_DIR)/stdio.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR3)/pic.c -o $(BUILD_DIR)/pic.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR3)/exceptions.c -o $(BUILD_DIR)/exceptions.o
        $(ASM) $(SRC_DIR2)/$(SRC_DIR3)/idt.s -f elf32 -o $(BUILD_DIR)/idt_stubs.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR3)/idt.c -o $(BUILD_DIR)/idt.o
        $(CCOMP) $(CFLAGS) $(SRC_DIR2)/$(SRC_DIR4)/virtual_memory_manager.c -o $(BUILD_DIR)/virtual_memory_manager.o
        $(LD) -m elf_i386 -T link.ld -o $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.o $(BUILD_DIR)/kernelC.o $(BUILD_DIR)/x86.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/exceptions.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/idt_stubs.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/virtual_memory_manager.o

#
# This part is SUPER necessary. The --oformat binary that directly links
# files together and puts them into binary is not actually good, as it COMPLETELY ignores the .bss sections.
# my stack is INITIALIZED in the .bss section of the kernel.s code. Therefore, by linking with --oformat meant that
# I didn't have a stack at all, and my code just happened to have been working this entire time!
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
        objcopy -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin

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
        # -s -S are flags that let us use GDB along with our QEMU emulator for debugging. 
        qemu-system-i386 -m 128M -drive format=raw,file=build/os-image.img,if=ide,index=0,media=disk -s -S 
        
