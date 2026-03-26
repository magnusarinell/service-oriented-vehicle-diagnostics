/* lwipopts.h — minimal config for ECM firmware */
#pragma once

#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_TCP                    1
#define LWIP_UDP                    0
#define LWIP_DHCP                   0
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_IPV4                   1
#define LWIP_ICMP                   1
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (16 * 1024)
#define MEMP_NUM_TCP_PCB            4
#define MEMP_NUM_TCP_PCB_LISTEN     2
#define MEMP_NUM_PBUF               16
#define PBUF_POOL_SIZE              8
#define TCP_MSS                     1460
#define TCP_WND                     (4 * TCP_MSS)
#define TCP_SND_BUF                 (4 * TCP_MSS)
#define LWIP_STATS                  0
#define LWIP_TIMEVAL_PRIVATE        0
