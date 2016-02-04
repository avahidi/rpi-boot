#ifndef __DEFS_H__
#define __DEFS_H__

#define GPIO_BASE 0x3F200000
#define UART_BASE 0x3f201000
#define WDOG_BASE 0x3f100018


#define RAM_SIZE  (4 * 1024)
#define RAM_START (0 * 1024)

#define NORMAL_BOOT (RAM_START + 0x800)

#define CNTFRQ 19200000

#define PSR_MASK_IRQ 0x80
#define PSR_MASK_FIQ 0x40


#ifdef __ASSEMBLER__

#else /* __ASSEMBLER__ */

#include <stdint.h>

#define adr_t uint32_t
#define size_t uint32_t


#define __packed __attribute__((packed))
#define __naked __attribute__((naked))
#define __weak __attribute__((weak))

extern uint32_t __smc(uint32_t op, uint32_t data);
extern uint32_t cpu0_psr;

#endif /* __ASSEMBLER__ */

#endif /* __DEFS_H__ */
