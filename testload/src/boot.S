
#include "defs.h"

    .section .boot, "ax"
    .global exit

__vectors:
    b __reset
    b __die
    b __die
    b __die
    b __die
    nop
    b __die
    b __die


__reset:
    /* set stack at the end of RAM */
    ldr sp, =(RAM_SIZE + RAM_START)

    ldr r0, =__vectors
    mcr p15, 0, r0, c12, c0, 0

    /* SCTLR: enable I and C caches */
    mrc 15, 0, r0, c1, c0, 0
    orr r0, r0, #0x0004
    orr r0, r0, #0x1000
    mcr 15, 0, r0, c1, c0, 0

    /* clear bss */
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0
    mov r3, #0
    b 2f
1:
    stmia r0!, {r2-r3}
2:
    cmp r1, r0
    bhi 1b


    /* jump to main */
    bl main
    b exit

__die:

exit:
    wfi
    b exit
