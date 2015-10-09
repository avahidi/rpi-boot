#ifndef __DEFS_H__
#define __DEFS_H__

#define UART_BASE 0x3f201000
#define WDOG_BASE 0x3f100018


#define RAM_SIZE  (2 * 1024)
#define RAM_START (2 * 1024)

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

#endif /* __ASSEMBLER__ */

#endif /* __DEFS_H__ */
