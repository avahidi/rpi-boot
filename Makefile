# -*- Makefile -*-

DIRS = programmer testload bootloader

all:
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d ; done
clean:
	set -e ; for d in $(DIRS); do $(MAKE) -C $$d clean ; done

#

upload: all
	make -C testload upload

copy: all
	make -C bootloader copy

test: all
	make -C programmer test

#

openocd:
	make -C jtag openocd

jlink:
	make -C jtag jlink

gdb:
	make -C testload # ensure testload is updated since this is what we will upload
	make -C jtag gdb
