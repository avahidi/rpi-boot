The mini-Pi programmer
======================

The mini-Pi programmer is a minimal serial programmer for the mini-Pi bootloader.


Usage
-----

The programmer is invoked with the following parameters
::

    prog.exe [flags] <-o serialdevice> [-i input file] [-a address]

Where the flags are

+-------+-----------------------------------+
| Flag  | Meaning                           |
+=======+===================================+
| -v    | verify writes                     |
+-------+-----------------------------------+
| -d    | enable debugging                  |
+-------+-----------------------------------+
| -h    | show help                         |
+-------+-----------------------------------+
| -n    | dont start code after upload      |
+-------+-----------------------------------+
| -W    | enable watchdog                   |
+-------+-----------------------------------+
| -H    | start in HYP mode                 |
+-------+-----------------------------------+
| -S    | start in secure mode              |
+-------+-----------------------------------+

Example
::

    prog.exe -o /dev/ttyUSB0 -i testload.bin -a 0x00008000 -W -v


Will load testload.bin to position 0x00008000 and start it with the watchdog enabled.


Input format
~~~~~~~~~~~~

The input file should be a plain ARM32 binary (not an ELF file), with the given start address.
See the testload folder for an example.

The interactive programmer
--------------------------

If the input file is omitted, the programmer will turn into an interactive programmer.
It allows you to send command directly to the bootloader. This can for example be used to
program Raspberry Pi2 I/O register without writing any code.


When in interactive mode, the programmer adds the appropriate fields such as checksum for you:
::

    build/prog.exe -o /dev/ttyUSB0
    > ?
    >> sending... (3)
    00000000: 3f 01 c0                                           ?..
    << received... (17)
    00000000: 24 0f 6d 69 6e 69 70 69 20 62 6f 6f 74 6c 64 2e    $.minipi bootld.
    00000010: 75                                                 u


If you need to enter binary data, you may use the \xx shorthand where xx is the hexadecimal representation of a byte:
::

    > g\04
    >> sending... (4)
    00000000: 67 02 04 93                                        g...
    << received... (7)
    00000000: 24 05 00 10 00 00 c7                               $......
    
    > 


