
#include "defs.h"

    .code 32
    .section .boot-sec-low, "ax"

    /*
     * This is the secure boot monitor and initialization code.
     * It has to fit somewhere below 0x100 (we choose 0xEC) otherwise it
     * will get overwritten by unknown and undocumented stuff.
     *
     * ( Yes, Eben Upton lied to you. Raspberry Pi is not open. )
     */

__secvectors:
    adr pc, __secreset
    adr pc, __secundef
    adr pc, __secmon
    adr pc, __secpabort
    adr pc, __secdabort
    nop
    adr pc, __secirq
    adr pc, __secfiq

__secirq:
__secfiq:
__secpabort:
__secdabort:
__secundef:

    /* just die */
__die:
    wfe
    b __die


/* the unsecure secure monitor, does as being told */
__secmon:
    cmp r0, #1
    beq change_mode

change_scr:
    /* SCR ^= r1 -> what security?? */
    mrc p15, 0, r0, c1, c1, 0
    eor r0, r0, r1
    mcr p15, 0, r0, c1, c1, 0
    movs pc, lr

change_mode:
    msr spsr_all, r1
    movs pc, lr

__secreset:
    /* do not disturb, please */
    cpsid ifa

#if 0
    /* for now, allow only one CPU */
    mrc p15, 0, r1, c0, c0, 5
    ands r1, r1, #3
    bne __die
#endif

    /* set MVBAR and VBAR */
    ldr r0, =__secvectors
    mcr p15, 0, r0, c12, c0, 1
    mcr p15, 0, r0, c12, c0, 0

    /* Update SCR: clear IRQ, FIQ, EA, nET. set AW, HCE, and FW  */
    mrc p15, 0, r1, c1, c1, 0
    bic r1, r1, #0x4E
    orr r1, r1, #0x130
    mcr p15, 0, r1, c1, c1, 0
    isb

    /* SCTLR: enable I and C caches */
    mrc 15, 0, r0, c1, c0, 0
    orr r0, r0, #0x0004
    orr r0, r0, #0x1000
    mcr 15, 0, r0, c1, c0, 0

    /* ACTLR: set SMP */
    mrc p15, 0, r0, c1, c0, 1
    orr r0, r0, #0x40
    mcr p15, 0, r0, c1, c0, 1

    /* CNTV_CTL enable ? */
    mov r0, #1
    mcr p15, 0, r0, c14, c3, 1

    /* NSACR = all COPs are non-sec */
    movw r1, #0x3fff
    movt r1, #6
    mcr p15, 0, r1, c1, c1, 2

    /* set CNTFRQ */
    ldr r0, =CNTFRQ
    mcr p15, 0, r0, c14, c0, 0

    /* switch to NS */
    mov r0, #0
    mov r1, #1
    smc #0

    mov pc, #NORMAL_BOOT
