#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include <stdint.h>

/* With NO_SYS=1, lwip/sys.h already typedefs sys_sem_t, sys_mutex_t and
 * sys_mbox_t as u8_t.  Only define sys_thread_t which lwip/sys.h does not
 * provide. */
typedef int sys_thread_t;

#endif /* LWIP_ARCH_SYS_ARCH_H */
