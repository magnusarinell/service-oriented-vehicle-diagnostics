/* ethernetif.c — K64F ENET driver stub for Renode EthernetBridge
 *
 * In Renode the K64F ENET peripheral is emulated; EthernetBridge connects
 * it to the host TAP interface so packets flow to/from the Docker network.
 *
 * A real driver would program the K64F ENET DMA descriptors.  For the
 * purposes of this firmware prototype the functions are declared but kept
 * minimal — Renode's built-in ENET model handles the actual frame
 * transmission when cpu execution reaches the register writes.
 */
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"
#include <string.h>

/* K64F ENET base address */
#define ENET_BASE  0x400C0000UL

/* Dummy MAC address — override in production */
static const uint8_t k64f_mac[6] = { 0x02, 0xEC, 0x4F, 0x00, 0x00, 0x01 };

static err_t k64f_link_output(struct netif *netif, struct pbuf *p)
{
    (void)netif;
    /* TODO: write pbuf chain to K64F ENET TX descriptor ring */
    (void)p;
    return ERR_OK;
}

err_t ethernetif_init_board(struct netif *netif)
{
    netif->hwaddr_len = ETH_HWADDR_LEN;
    memcpy(netif->hwaddr, k64f_mac, ETH_HWADDR_LEN);
    netif->mtu        = 1500;
    netif->flags      = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    netif->output     = etharp_output;
    netif->linkoutput = k64f_link_output;
    return ERR_OK;
}

/* Called from main loop to poll for received frames */
void ethernetif_input(struct netif *netif)
{
    /* TODO: check K64F ENET RX descriptor ring and call netif->input() */
    (void)netif;
}
