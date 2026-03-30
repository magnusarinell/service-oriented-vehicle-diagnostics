#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* Compiler hints for packing */
#define PACK_STRUCT_FIELD(x)  x
#define PACK_STRUCT_STRUCT    __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

/* Platform-specific diagnostic output */
#define LWIP_PLATFORM_DIAG(x) do { printf x; } while (0)
#define LWIP_PLATFORM_ASSERT(x) do { } while (0)

/* sys_prot_t must be defined here (in arch/cc.h) because with NO_SYS=1
 * lwip/sys.h never includes arch/sys_arch.h, yet still uses sys_prot_t. */
typedef uint32_t sys_prot_t;

#endif /* LWIP_ARCH_CC_H */
