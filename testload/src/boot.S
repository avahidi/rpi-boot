
#include "defs.h"

    .section .boot, "ax"
    .global __die
    .global __cpuid
    .global __sev
    .global __reset


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
    ldr r0, =__vectors
    mcr p15, 0, r0, c12, c0, 0

    /* SCTLR: enable I and C caches */
    mrc 15, 0, r0, c1, c0, 0
    orr r0, r0, #0x0004
    orr r0, r0, #0x1000
    mcr 15, 0, r0, c1, c0, 0

    /* set stack based on what CPU this is: */
    mrc p15, 0, r1, c0, c0, 5
    ands r1, r1, #3

    ldr r0, =__stack_top__
    lsl r2, r1, #9 /* 512 bytes per cpu ? */
    sub sp, r0, r2

    /* bypass global init if secondary core */
    cmp r1, #0
    bne init_end


init_start:
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


init_end:
    /* jump to main */
    bl main
    b __die

__die:
    wfe
    b __die


__cpuid:
    mrc p15, 0, r0, c0, c0, 5
    ands r0, r0, #3
    mov pc, lr

__sev:
    sev
    mov pc, lr
