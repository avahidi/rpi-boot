
JTAG ?= jlink
CROSS_COMPILE ?= arm-none-eabi-

all:


openocd:
	-killall openocd
	@echo "Using interface JTAG=$(JTAG) with openocd"
	@openocd -f interface/$(JTAG).cfg -f openocd.cfg

jlink:
ifeq ($(JTAG),jlink)
	# Speed could go much higher if you use a proper adapter
	#JLinkExe -device cortex-a7 -speed 100 -if jtag -jtagconf -1,-1 -autoconnect 1 -CommanderScript jlink.cfg
	JLinkGDBServer -if jtag -cpu Cortex-A_R  -device Cortex-A7 -speed 400 -port 3333
else
	@echo "I can only do this if JTAG is jlink :("
	@exit 20
endif

gdb:
	@$(CROSS_COMPILE)gdb -x gdb_upload.cmd ../testload/build/testload.elf
#	-killall openocd
