

CROSS_COMPILE ?= arm-none-eabi-
DEV ?= /dev/ttyUSB0

SRC= $(wildcard src/*.c) $(wildcard src/*.S)
LD= src/linker.ld
OUT=build/testload

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

upload: all
	../programmer/build/prog.exe -v -o $(DEV) -i $(OUT).bin -a 0x00008000

#

build:
	mkdir build

clean:
	rm -rf build

