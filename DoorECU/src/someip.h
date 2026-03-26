#pragma once
#include <stdint.h>
#include "lwip/tcp.h"

/* SOME/IP service identifiers — must match CDA/src/EcuSomeIpClient.h */
#define SOMEIP_SERVICE_ID   0x1111u
#define SOMEIP_INSTANCE_ID  0x0001u
#define SOMEIP_PORT         30509u   /* standard vsomeip SD unicast port */

#define METHOD_GET_CAPABILITIES   0x0001u
#define METHOD_READ_DATA          0x0002u
#define METHOD_READ_DATA_ITEM     0x0003u
#define METHOD_GET_FAULTS         0x0004u
#define METHOD_CLEAR_FAULTS       0x0005u
#define METHOD_GET_OPERATIONS     0x0006u
#define METHOD_EXECUTE_OPERATION  0x0007u

/* SOME/IP message header (network byte order) */
typedef struct __attribute__((packed)) {
    uint16_t service_id;
    uint16_t method_id;
    uint32_t length;         /* length of payload + 8 (rest of header) */
    uint16_t client_id;
    uint16_t session_id;
    uint8_t  protocol_ver;   /* 0x01 */
    uint8_t  interface_ver;  /* 0x01 */
    uint8_t  msg_type;       /* 0x00 = REQUEST, 0x80 = RESPONSE */
    uint8_t  return_code;    /* 0x00 = OK */
} someip_hdr_t;

#define SOMEIP_HDR_LEN  sizeof(someip_hdr_t)

void someip_init(void);
