
#include "defs.h"

    .code 32
    .section .boot-sec, "ax"

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


__secpabort:
__secdabort:
__secirq:
__secfiq:
__secundef:
    b sleeping

__secmon:
    /* SCR ^= r0 -> what security?? */
    mrc p15, 0, r1, c1, c1, 0
    eor r0, r1, r0
    mcr p15, 0, r0, c1, c1, 0
    movs pc, lr

__secreset:
    /* do not disturb, please */
    cpsid ifa

    /* set MVBAR and VBAR */
    ldr r0, =__secvectors
    mcr p15, 0, r0, c12, c0, 1
    mcr p15, 0, r0, c12, c0, 0

    /* Read SCR, clear IRQ, FIQ, EA, nET, AW HCE, and FW  */
    mrc p15, 0, r1, c1, c1, 0
    bic r1, r1, #0x4E
    orr r1, r1, #0x30
    orr r1, r1, #0x1000
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
    mov r0, #1
    smc 0

    /* extract CPU id from MPIDR */
    mov r4, #NORMAL_BOOT
    mrc p15, 0, r0, c0, c0, 5
    ands r0, r0, #3

    /* primary ? */
    beq user_boot

secondary_boot:
    /* clear mailbox 3 of this CPU */
    ldr r5, =0x4000008c
    mov r3, #0
    str r3, [r5, r0, lsl #4]

    /* wait for mailbox to be non-zero */
    ldr r5, =0x400000cc
1:
    wfe
    ldr r4, [r5, r0, lsl #4]
    cmp r4, r3
    beq 1b

    str r4, [r5, r0, lsl #4]
    /* jump to user boot address (from mailbox or normal __vectors on primary) */
user_boot:
    bx r4


    /* wait user */
sleeping:
    wfi
    b sleeping

