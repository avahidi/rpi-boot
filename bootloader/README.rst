
The mini-Pi bootloader
======================

This the mini-pi bootloader, a minimal serial bootloader for RPi2 (quad Cortex-A7). 
It was designed with simplicity in mind, and requires approximately 2KB of memory.


The Watchdog
------------

The bootloader can enable the 16 second watchdog before entering user code.
Assuming user code doesn't disable the watchdog, this will allow you to re-program the device after 16 seconds without a need for a power cycle.

See the START command for more information


Memory layout
-------------

+---------------+----------------+
| Address       | Use            |
+===============+================+
| 0x8000-....   | user code area |
+---------------+----------------+
| 0x1000-0x8000 | *UNUSED*       |
+---------------+----------------+
| 0x0800-0x1000 | bootloader     |
+---------------+----------------+
| 0x0100-0x0800 | RPi2 internal  |
+---------------+----------------+
| 0x0000-0x0100 | secure boot    |
+---------------+----------------+

If no other instructions are given, the loaded code is assumed to start at 0x8000.


Multi-core bringup
------------------

CPU power-on is performed by writing the requested entry point to the 3rd mailbox of the target CPU.
Essentially, this works similar to the original RPi2 boot ROM, with the following minor differences:

 #. Target CPU is always awaken in Normal Supervisor mode.
 #. R0 and R1 contain no valid information at boot
 #. You must emit a CPU event (SEV) after writing to the mailbox (and don't forget the synchronization barrier)




-------



Bootloader protocol
-------------------

Normally you should not care about this since the programmer handles this for you, but just for the sake of completeness here is the protocol we use to talk to the bootloader:

Command / response format
~~~~~~~~~~~~~~~~~~~~~~~~~

The general command and response format is 

:COMMAND: [command] [N = payload length] [N-1 bytes of data] [checksum]
:RESPONSE: [response code] [N = payload length] [ N-1 bytes of data] [checksum].


Note that

 #. The data field is optional
 #. Each field except data is a single byte long
 #. The checksum is the additive inverse, i.e. the sum of all elements including the checksum should be zero.
 #. The length field includes data and the checksum.
 #. Data order is LSB first ( e.g. 0x123 is represented as [0x23, 0x01] )

The response code is one of the following

 * $ -  success, no error
 * ! - general error
 * u - unknown error 
 * p - command parameter error
 * f - command format error
 * s - command size error


The ABOUT command
~~~~~~~~~~~~~~~~~

The ABOUT command is used to identify the board and has the following format:

:COMMAND: ? 1 [checksum]
:RESPONSE: $ 15 "minipi bootld" [checksum]
 

The REGISTER command
~~~~~~~~~~~~~~~~~~~~

The bootloader has five different registers, one of which is used for addressing

 0. R0, this corresponds to targets R0 at boot
 1. R1, this corresponds to targets R0 at boot
 2. PSR, this corresponds to targets CPSR at boot
 3. PC, this corresponds to targets PC at boot
 4. ADR, this is the address used in read and write commands
 
You may read a register using the REGISTER command

:COMMAND: g 2 [register number] [checksum]
:RESPONSE: $ 5 [32-bit register value] [checksum]
 
You may also write to a register

:COMMAND: g [3-6] [register number] [1-4 bytes of register content] [checksum]
:RESPONSE: $ 5 [32-bit register value] [checksum]


The WRITE command
~~~~~~~~~~~~~~~~~

The write commands writes the given data to the position of the ADR register.

:COMMAND: w [N+1] [data] [checksum]
:RESPONSE: $ 1 [checksum]

Note that the ADR register is increased automatically after each write

The READ command
~~~~~~~~~~~~~~~~

The read commands reads a number of bytes from the position of the ADR register

:COMMAND: r [2] [N] [checksum]
:RESPONSE: $ [1+N] [N bytes of data] [checksum]

As with  WRITE, the ADR register is increased automatically after each read


The START command
~~~~~~~~~~~~~~~~~


Start the guest

:COMMAND: s [2] [options] [checksum]
:RESPONSE: $ [1] [checksum]

The bits in option have the following meaning

 * Bit 0: enable the watchdog
 * Bit 1: start execution in Secure World
 * Bit 2: start execution in HYP mode (this overrides any previous writes to PSR)
 
Note that

  #. Execution starts at the address in the PC register (see REGISTER command)
  #. Execution starts in the mode specified by the PSR register (see REGISTER command)  
  #. If successful, you will not be able to execute any more commands after this
  #. If the watchdog is enabled, the device will reboot back into the bootloader after 16s.
