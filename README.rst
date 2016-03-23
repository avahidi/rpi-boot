
mini-Pi bootloader
==================

The mini-pi bootloader is a minimal serial bootloader for loading the initial bare metal code into Raspberry-Pi 2.
It supports booting into the normal & **secure world** as well as the **hypervisor mode**.

Who should use this?
--------------------

People who are experimenting with bare metal code.

 1. You need a simple way to write and run baremetal normal/secure world or hypervisor mode code.
 2. You don't have access to a JTAG debugger/programmer and you are getting tired of the SD-card juggling.

**Who should not use this?**

 1. Practically everybody ;)
 2. If you are running Linux you don't need this
 3. If you are running u-boot you have a much more powerful tool and won't need this either
 4. If your code is too large for serial download you can't use mini-pi


Usage
-----
The project structure is

 * bootloader/ - this is the minimal bootloader that should be written to your sdcard
 * programmer/ - this is the PC side software for uploading your bare metal code
 * testload/ - this is an example bare metal project (LED blink for RPi2, to be exact)

For a quick start

 #. insert working sdcard with the correct partitions (e.g. Raspbian)
 #. "make copy" (assuming Ubuntu 15.04 or higher)
 #. insert sdcard into RPi2 and switch power on
 #. connect RPi2 UART0 to /dev/ttyUSB0
 #. "make upload"
 #. Now if you change the testload project, you only need to power cycles and run "make upload" again to test your new code.


If you want to copy the bootloader manually, this is what you need to do

  #. "make"  (build the project)
  #. copy bootloader/build/bootld.img to the boot partition as kernel7.img
  #. rename old kernel7.img to kernel7.img_OLD (repeat for kernel.img)
  #. add "kernel_old=1" to config.txt on the same partition.
  #. comment out any lines starting with "kernel=" in the same file

The programmer
--------------
For information about the programmer, see programmer/README.rst

A simple bare metal program for testing is included, see testload/README.rst.

JTAG
----

On RPi2 JTAG signals are initially turned off and must be enabled by configuring GPIO.
Hence to use a JTAG debugger you first need to program the board.
The bootloader solves this annoying "chicken and the egg" problem for you by configuring
the GPIO pins to JTAG mode.

There are some other issues with JTAG on RPi2, refer to these resources for more information:

* https://github.com/guancio/kth-on-rpi2/blob/master/rpi2-port/Guide/Guide.pdf
* https://github.com/dwelch67/raspberrypi/tree/master/armjtag/rpi2
* https://gist.github.com/tonosaman/62a31e7991a41edb19c5
* http://sysprogs.com/VisualKernel/tutorials/raspberry/jtagsetup

Technical details
-----------------

For the technical details see bootloader/README.rst
