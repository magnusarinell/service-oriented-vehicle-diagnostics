#include "someip.h"
#include "diag_data.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* JSON helpers (no heap alloc -- fixed buffers) */

static int build_capabilities(char *buf, int max)
{
    return snprintf(buf, max,
        "[{\"name\":\"data\",\"description\":\"Read live door sensor data\",\"category\":\"read\"},"
        "{\"name\":\"faults\",\"description\":\"Read and clear door fault codes\",\"category\":\"read-write\"},"
        "{\"name\":\"operations\",\"description\":\"Execute door control routines\",\"category\":\"execute\"}]");
}

static int build_data(char *buf, int max)
{
    return snprintf(buf, max,
        "[{\"id\":\"lock_state\",\"name\":\"Door Lock State\",\"value\":\"%s\",\"unit\":\"\"},"
        "{\"id\":\"window_pos\",\"name\":\"Window Position\",\"value\":\"%u\",\"unit\":\"%%\"},"
        "{\"id\":\"mirror_angle\",\"name\":\"Mirror Angle\",\"value\":\"%d\",\"unit\":\"deg\"},"
        "{\"id\":\"door_ajar\",\"name\":\"Door Ajar Sensor\",\"value\":\"%s\",\"unit\":\"\"}]",
        door_ecu_sensors.lock_state ? "locked" : "unlocked",
        (unsigned)door_ecu_sensors.window_pos,
        (int)door_ecu_sensors.mirror_angle,
        door_ecu_sensors.door_ajar ? "true" : "false");
}

static int build_faults(char *buf, int max)
{
    int len = 0;
    len += snprintf(buf + len, max - len, "[");
    int first = 1;
    if (door_ecu_fault_flags & (1u << 0)) {
        len += snprintf(buf + len, max - len,
            "%s{\"code\":\"B1001\",\"description\":\"Door Latch Sensor Fault\","
            "\"severity\":\"high\",\"status\":\"active\"}",
            first ? "" : ",");
        first = 0;
    }
    if (door_ecu_fault_flags & (1u << 1)) {
        len += snprintf(buf + len, max - len,
            "%s{\"code\":\"B1002\",\"description\":\"Window Motor Over-Current\","
            "\"severity\":\"medium\",\"status\":\"active\"}",
            first ? "" : ",");
        first = 0;
    }
    if (door_ecu_fault_flags & (1u << 2)) {
        len += snprintf(buf + len, max - len,
            "%s{\"code\":\"B1003\",\"description\":\"CAN Bus Cable Fault\","
            "\"severity\":\"high\",\"status\":\"active\"}",
            first ? "" : ",");
    }
    len += snprintf(buf + len, max - len, "]");
    return len;
}

static int build_operations(char *buf, int max)
{
    return snprintf(buf, max,
        "[{\"id\":\"unlock_door\",\"name\":\"Unlock Door\",\"description\":\"Send unlock command to door latch actuator\"},"
        "{\"id\":\"reset_window\",\"name\":\"Reset Window\",\"description\":\"Run window motor re-calibration routine\"}]");
}

/* SOME/IP framing */

static void send_response(struct tcp_pcb *pcb,
                           const someip_hdr_t *req_hdr,
                           const char *payload, uint16_t payload_len)
{
    someip_hdr_t resp;
    resp.service_id    = req_hdr->service_id;
    resp.method_id     = req_hdr->method_id;
    resp.length        = lwip_htonl(8u + payload_len);
    resp.client_id     = req_hdr->client_id;
    resp.session_id    = req_hdr->session_id;
    resp.protocol_ver  = 0x01;
    resp.interface_ver = 0x01;
    resp.msg_type      = 0x80; /* RESPONSE */
    resp.return_code   = 0x00;

    tcp_write(pcb, &resp, SOMEIP_HDR_LEN, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
    tcp_write(pcb, payload, payload_len, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
}

/* TCP receive callback */

static err_t on_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    (void)arg; (void)err;
    if (!p) {
        tcp_close(pcb);
        return ERR_OK;
    }

    if (p->len < (int)SOMEIP_HDR_LEN) {
        pbuf_free(p);
        return ERR_OK;
    }

    someip_hdr_t hdr;
    memcpy(&hdr, p->payload, SOMEIP_HDR_LEN);
    pbuf_free(p);

    uint16_t method = lwip_ntohs(hdr.method_id);

    static char json_buf[2048];
    int json_len = 0;

    switch (method) {
    case METHOD_GET_CAPABILITIES:  json_len = build_capabilities(json_buf, sizeof json_buf); break;
    case METHOD_READ_DATA:         json_len = build_data(json_buf, sizeof json_buf);         break;
    case METHOD_GET_FAULTS:        json_len = build_faults(json_buf, sizeof json_buf);       break;
    case METHOD_CLEAR_FAULTS:
        door_ecu_fault_flags = 0;
        json_len = snprintf(json_buf, sizeof json_buf, "[]");
        break;
    case METHOD_GET_OPERATIONS:    json_len = build_operations(json_buf, sizeof json_buf);   break;
    case METHOD_READ_DATA_ITEM:
    case METHOD_EXECUTE_OPERATION:
        json_len = snprintf(json_buf, sizeof json_buf, "{}");
        break;
    default:
        return ERR_OK;
    }

    tcp_recved(pcb, (u16_t)SOMEIP_HDR_LEN);
    send_response(pcb, &hdr, json_buf, (uint16_t)json_len);
    return ERR_OK;
}

static err_t on_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    (void)arg; (void)err;
    tcp_recv(newpcb, on_recv);
    return ERR_OK;
}

/* Public init */

void someip_init(void)
{
    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, SOMEIP_PORT);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, on_accept);
    printf("[DoorECU] SOME/IP listening on port %u\n", SOMEIP_PORT);
}
