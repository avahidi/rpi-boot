
The mini-Pi testload
====================

This is a test program for the mini-Pi bootloader. It demonstrate the following items

 #. Bare metal compilation (see the Makefile)
 #. Minimal bare metal boot on ARMv7-A (see src/boot.S)
 #. Memory layout (see src/linker.ld)
 #. How to use the programmer (see Makefile, target upload)


The program itself toggles GPIO 46 to blink the "active" LED.

