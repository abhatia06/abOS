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
.PHONY: all floppy_image kernel bootloader clean always run

floppy_image: $(BUILD_DIR)/main_floppy.img

#
# disk image
#
$(BUILD_DIR)/main_floppy.img: bootloader kernel
        dd if=/dev/zero of=$(BUILD_DIR)/os-image.img bs=512 count=2880
        @echo "drive size:" && stat -c%s $(BUILD_DIR)/os-image.img
        #Loads the bootloader (boot.bin) into sector 0
        dd if=$(BUILD_DIR)/boot.bin of=$(BUILD_DIR)/os-image.img bs=512 count=1 conv=notrunc
        @echo "BOOT1 BIN size:" && stat -c%s $(BUILD_DIR)/boot.bin
        #Loads actual 2nd stage bootloader (yeaah my bootloader is getting really messy now)
        dd if=$(BUILD_DIR)/bootstage2.bin of=$(BUILD_DIR)/os-image.img bs=512 seek=1 conv=notrunc
        @echo "BOOT STAGE 2 SIZE: " && stat -c%s $(BUILD_DIR)/bootstage2.bin
        #Loads the 2nd stage bootloader into sector 2
        dd if=$(BUILD_DIR)/boot2.bin of=$(BUILD_DIR)/os-image.img bs=512 seek=2 conv=notrunc
        @echo "BOOT2 BIN size:" && stat -c%s $(BUILD_DIR)/boot2.bin
        #Loads the kernel into sector 3 (and prekernel sector 23)
        dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/os-image.img bs=512 seek=3 count=$(KERNEL_SECTORS) conv=notrunc
        dd if=$(BUILD_DIR)/prekernel.bin of=$(BUILD_DIR)/os-image.img bs=512 seek=23 count=$(PREKERNEL_SECTORS) conv=notrunc
        @echo $(KERNEL_SECTORS)
        @echo $(PREKERNEL_SECTORS)

$(BUILD_DIR)/main.bin: $(SRC_DIR)/main.s
        $(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin


#
#bootloader
#
bootloader: $(BUILD_DIR)/boot.bin

$(BUILD_DIR)/boot.bin: always
        $(ASM) $(SRC_DIR)/boot.s -f bin -o $(BUILD_DIR)/boot.bin
        $(ASM) $(SRC_DIR)/boot2.s -f bin -o $(BUILD_DIR)/boot2.bin
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
        $(LD) -m elf_i386 -T link.ld -o $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.o $(BUILD_DIR)/kernelC.o $(BUILD_DIR)/stdio.o $(BUILD_DIR)/x86.o $(BUILD_DIR)/pic.o $(BUILD_DIR)/exceptions.o $(BUILD_DIR)/idt_stubs.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/physical_memory_manager.o $(BUILD_DIR)/string.o $(BUILD_DIR)/virtual_memory_manager.o
        $(LD) -m elf_i386 -T kernelLink.ld -o $(BUILD_DIR)/prekernel.elf $(BUILD_DIR)/prekernel.o $(BUILD_DIR)/printlite.o $(BUILD_DIR)/virtual_memory_manager.o $(BUILD_DIR)/physical_memory_manager.o $(BUILD_DIR)/string.o

#
# The --oformat binary that directly links
# files together and puts them into binary is not actually good, as it COMPLETELY ignores the .bss sections.
# my stack is INITIALIZED in the .bss section of the kernel.s code. Therefore, by linking with --oformat meant that
# I didn't have a stack.
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
        @echo "Kernel ELF size:" && stat -c%s $(BUILD_DIR)/kernel.elf
        objcopy -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin
        @echo "Kernel BIN size:" && stat -c%s $(BUILD_DIR)/kernel.bin
        objcopy -O binary $(BUILD_DIR)/prekernel.elf $(BUILD_DIR)/prekernel.bin


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
