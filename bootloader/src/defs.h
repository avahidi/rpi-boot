#ifndef __DEFS_H__
#define __DEFS_H__

#define GPIO_BASE 0x3F200000
#define UART_BASE 0x3f201000
#define WDOG_BASE 0x3f100018


#define RAM_SIZE  (0 * 1024)
#define RAM_START (4 * 1024)

#define SECURE_BOOT 0
#define ATAGS_START 0x100
#define NORMAL_BOOT 0x800

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

extern uint32_t __smc(uint32_t r0);
#endif /* __ASSEMBLER__ */

#endif /* __DEFS_H__ */
