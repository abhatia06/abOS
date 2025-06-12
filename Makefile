# Because I do not have a lot of knowledge working with Makefiles (besides 1 course I took in university), a LOT of this code is from Nanobyte's OS. There are some changes
# I made, though, and I have slowly come around to understanding this Makefile code.

ASM=nasm
SRC_DIR=bootloader
SRC_DIR3=kernel
BUILD_DIR=build
CCOMP=gcc
LD = ld

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

$(BUILD_DIR)/main.bin: $(SRC_DIR)/main.s
        $(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin


#
#bootloader
#
bootloader: $(BUILD_DIR)/boot.bin

$(BUILD_DIR)/boot.bin: always
        $(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin
        $(ASM) $(SRC_DIR)/boot2.s -f bin -o $(BUILD_DIR)/boot2.bin


#
#kernel
#
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
        $(ASM) $(SRC_DIR3)/kernel.s -f elf32 -o $(BUILD_DIR)/kernel.o
        $(CCOMP) -m32 -ffreestanding -fno-pic -fno-pie -nostdlib -c $(SRC_DIR3)/kernel.c -o $(BUILD_DIR)/kernelC.o
        $(LD) -m elf_i386 -T link.ld -o $(BUILD_DIR)/kernel.bin --oformat binary $(BUILD_DIR)/kernel.o $(BUILD_DIR)/kernelC.o


#
#always
#
always:
        mkdir -p $(BUILD_DIR)


clean:
        rm -rf $(BUILD_DIR)/*
