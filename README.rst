
mini-PI bootloader
==================

The mini-pi bootloader is a minimal serial bootloader for loading the initial bare metal code into Raspberry-Pi 2.

**Who should use this?** People who are are experimenting with bare metal code.

If you are playing with bare metal code and don't have access to a JTAG debugger/programmer, you will need to update the sdcard boot partition every time you change your code.
The mini-pi bootloader allows you to update the sdcard once and perform all subsequence programming via the serial interfaces (UART0). 

**Who should not use this?** 

 1. Practically everybody ;)
 2. If you are running Linux you don't need this
 3. If you are running u-boot you have a much more powerful tool and won't need this either
 4. If your code is too large for serial download you can't use mini-pi


Usage
-----
The project structure is

 * bootloader - this is the minimal bootloader that should be written to your sdcard
 * programmer - this is the PC side software for uploading your bare metal code
 * testload - this is an example bare metal project (RPi2 "act" LED blinker, to be eaxct)

For a quick start

 1. insert working sdcard (e.g. Raspbian)
 2. "make copy" (assuming Ubuntu 15.04 or higher)
 3. insert sdcard into RPi2 and switch power on
 4. connect RPi2 UART0 to /dev/ttyUSB0
 5. "make upload"

Now if you change the testload project, you only need to power cycles and run "make upload" again to test your new code.


Memory layout
~~~~~~~~~~~~~
The bootloader is loaded at 0x8000 but relocates itself to 0x0800 to allow user binaries to be loaded to the default 0x8000 address.

