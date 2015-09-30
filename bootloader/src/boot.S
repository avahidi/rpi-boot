
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
    /* do not disturb, please */
    cpsid ifa

    /* see if we need to relocate */
    ldr r0, =__vectors @@ should be here
    adr r1, __vectors  @@ but we are here
    cmp r0, r1
    beq 1f

    ldr r2, =RAM_SIZE    
    add r2, r0, r2
2:
    ldmia r1!, {r3-r6}
    stmia r0!, {r3-r6}
    cmp r2, r0
    bhi 2b

    @@ now jump to the intended address
    dsb sy
    isb 

    ldr r0, =2f
    bx r0
2:

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
