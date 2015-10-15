
#include "defs.h"

    .section .boot-norm, "ax"
    .global __smc


    /*
     * This is the normal boot initialization code.
     * It is placed somewhere around 0x800 because it for some reason
     * doesn't work anywhere else due to undocumented boot ROM operation.
     *
     * A very personal note: stupidies like this is way  we need the 
     * ARM Trused Firmware project. Standard boot is the only way to go!
     */

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
    /* do not disturb, please */
    cpsid ifa

    /* set stack at the end of RAM */
    ldr sp, =(RAM_SIZE + RAM_START)

    /* set interrupt vector base, just in case */
    ldr r0, =__vectors
    mcr p15, 0, r0, c12, c0, 0

    /* clear bss */
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0
    mov r3, #0
    b 2f
1:  stmia r0!, {r2-r3}
2:  cmp r1, r0
    bhi 1b

    /* jump to main */
    bl main
    b sleeping

__die:
    /* just go to sleep */
sleeping:
    wfi
    b sleeping

    /* just a monitor call wrapper */
__smc:
    smc #0
    mov pc, lr
