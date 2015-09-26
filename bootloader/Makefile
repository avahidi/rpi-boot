
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

all: $(OUT).dis $(OUT).bin
	ls -l $(OUT).*

$(OUT).dis: $(OUT).elf
	@$(CROSS_COMPILE)objdump -z -D -x -t -r $(OUT).elf > $(OUT).dis

$(OUT).bin: $(OUT).elf
	@$(CROSS_COMPILE)objcopy -O binary $(OUT).elf $(OUT).bin

$(OUT).elf: build Makefile $(SRC) $(LD)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(SRC) -o $(OUT).elf

# copy to sdcard
copy: all
	stat $(BOOT)/config.txt > /dev/null || exit 20
	cp $(BOOT)/config.txt $(BOOT)/config.txt.OLD	
	grep -v "kernel=" < $(BOOT)/config.txt.OLD > $(BOOT)/config.txt
	echo "kernel=bootld.bin" >> $(BOOT)/config.txt
	cp $(OUT).bin $(BOOT)
	sync

#

build:
	mkdir build

clean:
	rm -rf build

