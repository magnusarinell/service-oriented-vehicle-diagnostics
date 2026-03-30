/* sys_arch.c -- NO_SYS=1 port: no OS, no semaphores needed */
#include "lwip/opt.h"
#include "arch/sys_arch.h"
#include "arch/cc.h"
#include "lwip/sys.h"

/* Bare-metal critical section stubs.
 * On Cortex-M4 a real implementation would use PRIMASK/BASEPRI,
 * but for Renode simulation no interrupt masking is needed. */
sys_prot_t sys_arch_protect(void)
{
    return 0;
}

void sys_arch_unprotect(sys_prot_t pval)
{
    (void)pval;
}

/* sys_now() -- return current time in milliseconds.
 * Cortex-M4 SysTick counter placeholder; update when real timer is wired up. */
u32_t sys_now(void)
{
    /* SysTick current-value register decrements from reload value. */
    static volatile u32_t tick_ms = 0;
    return tick_ms;
}
