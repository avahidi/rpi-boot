
#include "defs.h"

    .section .boot-norm, "ax"
    .global __smc


    /*
     * This is the normal boot initialization code.
     *
     * Since we don't have any real protection mechanisms it could be placed
     * right after the TrustZone boot code. But the GPU bootloader uses the
     * area after 0x100 for the devicetree so this code was moved to ~2048.
     *
     * This behavior is not very well documented and demonstrates why we
     * need the ARM Trusted Firmware project.
     */

__vectors:
    b __reset
    b __undef
    b __swi
    b __pabort
    b __dabort
    nop
    b __irq
    b __fiq

__undef:
__swi:
__pabort:
__dabort:
__irq:
__fiq:
    /* just die ... */
__die:
    wfe
    b __die

__reset:
    /* do not disturb, please */
    cpsid ifa

    /* primary CPU ? */
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #3
    cmp r0, #0
    bne __boot_secondary

    /* set temp stack */
    ldr sp, =__tmp_stack_top__

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
    b __die


__boot_secondary: /* CPU# in r0 */

    /* clear mailbox 3 of this CPU */
    ldr r5, =0x4000008c
    mov r3, #0
    str r3, [r5, r0, lsl #4]
    dsb

    /* wait for mailbox to be non-zero */
    ldr r5, =0x400000cc
1:
    wfe

    ldr r4, [r5, r0, lsl #4]
    cmp r4, r3
    beq 1b

    str r4, [r5, r0, lsl #4]
    dsb

    /* switch to correct PSR */
    mov r0, #1
    ldr r1, =cpu0_psr
    ldr r1, [r1]
    smc #0

    /* and jump to the entry */
    bx r4

    /* just a monitor call wrapper */
__smc:
    smc #0
    mov pc, lr
