
MEMORY
{
    RAM (rwx) : ORIGIN = 0 * 1024, LENGTH = 4 * 1024
}

SECTIONS
{
    .text : {
        /* secure boot */
        *.o (.boot-sec-low)
        /* make sure secure boot doesnt exceeds this limit */
        . = 0xEC;

        /* normal world code */
        . = 2048; /* NORMAL_START, RPi2 magic limit :(*/
        *.o (.boot-norm)
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

    .stack ALIGN(8) : {
        __tmp_stack_bottom__ = .;
        . += 512;
        __tmp_stack_top__ = .;
    }
    /* crap */
    /DISCARD/ : {
        *(.note*)
        *(.comment)
    }
}
