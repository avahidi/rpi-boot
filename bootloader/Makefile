
CROSS_COMPILE ?= arm-none-eabi-

SRC= $(wildcard src/*.c) $(wildcard src/*.S)
LD= src/linker.ld
OUT=build/bootld
BOOT = /media/$(USER)/boot

CFLAGS += -mcpu=cortex-a7
#CFLAGS += -mcpu=arm1176jz-s

CFLAGS += -std=c99 -T $(LD)
CFLAGS += -nostartfiles -nostdlib -ffreestanding -Os -fno-common


#

all: $(OUT).dis $(OUT).bin $(OUT).img
	ls -l $(OUT).*

$(OUT).dis: $(OUT).elf
	@$(CROSS_COMPILE)objdump -z -D -x -t -r $(OUT).elf > $(OUT).dis

$(OUT).bin: $(OUT).elf
	@$(CROSS_COMPILE)objcopy -O binary $(OUT).elf $(OUT).bin

$(OUT).elf: build Makefile $(SRC) $(LD)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(SRC) -o $(OUT).elf

$(OUT).img: $(OUT).bin
	cp $(OUT).bin $(OUT).img

# copy to sdcard. this assumes you are using Ubuntu and 
# have an standard RPi OS on the SD card.
copy: all
	stat $(BOOT)/config.txt > /dev/null || exit 20
	-mv $(BOOT)/kernel.img $(BOOT)/kernel.img_OLD
	-mv $(BOOT)/kernel7.img $(BOOT)/kernel7.img_OLD
	-cp -n $(BOOT)/config.txt $(BOOT)/config.txt.OLD
	cat $(BOOT)/config.txt.OLD | grep -v "kernel=" | grep -v "kernel_old=" > $(BOOT)/config.txt
	echo "kernel_old=1" >> $(BOOT)/config.txt

	cp $(OUT).img $(BOOT)/kernel7.img
	sync
	sync
	-umount $(BOOT)
	sync

#

build:
	mkdir build

clean:
	rm -rf build

