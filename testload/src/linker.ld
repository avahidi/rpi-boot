
MEMORY
{
    RAM (rwx) : ORIGIN = 0x0008000, LENGTH = 16 * 1024
}

SECTIONS
{
    .text : {
        *.o (.boot)
        *.o (.text)
    } > RAM

    .rodata ALIGN(8) : {
        *.o (.rodata)
        *.o (.rodata.*)
        . = ALIGN(8);
        __rodata_end__ = .;
    } > RAM

    .data : {
        __data_start__ = .;
        *.o  (.data)
        . = ALIGN(8);
        __data_end__ = .;
    } > RAM

    .bss ALIGN(8) : {
        __bss_start__ = .;
        * (.bss*)
        . = ALIGN(8);
        __bss_end__ = .;
    } > RAM

    .stack ALIGN(16) : {
        __stack_bottom__ = .;
        . += 4 * 512;
        __stack_top__ = .;
    }

    /* crap */
    /DISCARD/ : {
        *(.note*)
        *(.comment)
    }
}
