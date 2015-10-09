
MEMORY
{
    RAM (rwx) : ORIGIN = 2 * 1024, LENGTH = 2 * 1024
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

    /* crap */
    /DISCARD/ : {
        *(.note*)
        *(.comment)
    }
}
