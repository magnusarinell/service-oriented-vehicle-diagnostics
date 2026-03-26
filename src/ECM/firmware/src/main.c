#include "diag_data.h"
#include "someip.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include <stdio.h>

/* Board-specific ethernet/netif bringup (implemented in port/ethernetif.c) */
extern void ethernetif_init_board(struct netif *netif);

static struct netif g_netif;

int main(void)
{
    diag_data_init();

    /* LwIP init */
    lwip_init();

    /* Add network interface — IP assigned by Renode EthernetBridge/DHCP or
       static via lwipopts.h defines. */
    ip4_addr_t ipaddr, netmask, gw;
    IP4_ADDR(&ipaddr,  172, 20, 0, 30);
    IP4_ADDR(&netmask, 255, 255, 0, 0);
    IP4_ADDR(&gw,      172, 20, 0, 1);

    netif_add(&g_netif, &ipaddr, &netmask, &gw,
              NULL, ethernetif_init_board, netif_input);
    netif_set_default(&g_netif);
    netif_set_up(&g_netif);

    printf("[ECM] Network up — %s\n", ip4addr_ntoa(&ipaddr));

    someip_init();

    printf("[ECM] Ready. fault_flags @ %p\n", (void *)&ecm_fault_flags);

    /* Main loop — drive LwIP timers and ethernet RX */
    while (1) {
        ethernetif_input(&g_netif);
        sys_check_timeouts();
    }
}
