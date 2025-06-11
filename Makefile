# Because I do not have a lot of knowledge working with Makefiles (besides 1 course I took in university), a LOT of this code is from Nanobyte's OS. There are some changes
# I made, though, and I have slowly come around to understanding this Makefile code.

ASM=nasm
SRC_DIR=bootloader
SRC_DIR3=kernel
BUILD_DIR=build
KERNEL_SECTORS= $(shell echo $$(( ( $(shell stat -c%s $(BUILD_DIR)/kernel.bin ) + 511 ) / 512 )))

.PHONY: all floppy_image kernel bootloader clean always run

floppy_image: $(BUILD_DIR)/main_floppy.img

#
#Floppy Image
#
$(BUILD_DIR)/main_floppy.img: bootloader kernel
        dd if=/dev/zero of=$(BUILD_DIR)/os-image.img bs=512 count=2880

        # Bootloader (boot.bin) loaded into sector 0 
        dd if=$(BUILD_DIR)/boot.bin of=$(BUILD_DIR)/os-image.img bs=512 count=1 conv=notrunc

        #2nd Stage Bootloader (boot2.bin) loaded into sector 1
        dd if=$(BUILD_DIR)/boot2.bin of=$(BUILD_DIR)/os-image.img bs=512 seek=1 conv=notrunc

        #Kernel (kernel.bin) loaded into sector 3
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


kernel: $(BUILD_DIR)/kernel.bin

#
#kernel
#
$(BUILD_DIR)/kernel.bin: always
        $(ASM) $(SRC_DIR3)/kernel.s -f bin -o $(BUILD_DIR)/kernel.bin


#
#always
#
always:
        mkdir -p $(BUILD_DIR)


clean:
        rm -rf $(BUILD_DIR)/*
