
CROSS_COMPILE ?= arm-none-eabi-
JTAG ?= jlink
# JTAG ?= ftdi/dp_busblaster

all:


openocd:
	-killall openocd
	@echo "Using interface JTAG=$(JTAG) with openocd"
	openocd -f interface/$(JTAG).cfg -f openocd.cfg

gdb:
	@$(CROSS_COMPILE)gdb -x gdb_upload.cmd ../testload/build/testload.elf
#	-killall openocd


break:
	@$(CROSS_COMPILE)gdb -x gdb_break.cmd

