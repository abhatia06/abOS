ASM=nasm
SRC_DIR=bootloader
SRC_DIR2=kernel
BUILD_DIR=build

.PHONY: all floppy_image kernel bootloader clean always run

floppy_image: $(BUILD_DIR)/main_floppy.img

#
#Floppy Image
#
$(BUILD_DIR)/main_floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/os-image.bin bs=512 count=2880
	cp $(BUILD_DIR)/boot.bin $(BUILD_DIR)/main_floppy.img
	dd if=$(BUILD_DIR)/boot.bin of=$(BUILD_DIR)/os-image.bin bs=512 count=1 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/os-image.bin bs=512 seek=1 conv=notrunc
	truncate -s 1440k $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main.bin: $(SRC_DIR)/main.s 
	$(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin


#
#bootloader
#
bootloader: $(BUILD_DIR)/bootloader.bin

$(BUILD_DIR)/bootloader.bin: always
	$(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin


kernel: $(BUILD_DIR)/kernel.bin

#
#kernel
#
$(BUILD_DIR)/kernel.bin: always
	$(ASM) $(SRC_DIR2)/kernel.s -f bin -o $(BUILD_DIR)/kernel.bin


#
#always
#
always:
	mkdir -p $(BUILD_DIR)


clean:
	rm -rf $(BUILD_DIR)/*
