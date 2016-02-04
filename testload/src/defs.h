
#ifndef __DEFS_H__
#define __DEFS_H__


#define GPIO_BASE 0x3F200000
#define MBOX_BASE 0x40000080

#define RAM_START 0x00080000
#define RAM_SIZE (1024 * 16)


#ifdef __ASSEMBLER__

#else /* __ASSEMBLER__ */

#include <stdint.h>
#define adr_t uint32_t
#define size_t uint32_t

#define __naked __attribute__((naked))

extern int __cpuid();
extern void __sev();
extern void __reset;

#endif /* __ASSEMBLER__ */

#endif /* __DEFS_H__ */
