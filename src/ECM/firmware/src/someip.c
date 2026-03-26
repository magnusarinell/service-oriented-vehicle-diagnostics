#include "someip.h"
#include "diag_data.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* ── JSON helpers (no heap alloc — fixed buffers) ──────────────────────── */

static int build_capabilities(char *buf, int max)
{
    return snprintf(buf, max,
        "[{\"name\":\"data\",\"description\":\"Read live sensor data\",\"category\":\"read\"},"
        "{\"name\":\"faults\",\"description\":\"Read and clear fault codes\",\"category\":\"read-write\"},"
        "{\"name\":\"operations\",\"description\":\"Execute diagnostic routines\",\"category\":\"execute\"}]");
}

static int build_data(char *buf, int max)
{
    return snprintf(buf, max,
        "[{\"id\":\"coolant_temp\",\"name\":\"Coolant Temperature\",\"value\":\"%d\",\"unit\":\"°C\"},"
        "{\"id\":\"battery_voltage\",\"name\":\"Battery Voltage\",\"value\":\"%.1f\",\"unit\":\"V\"},"
        "{\"id\":\"engine_rpm\",\"name\":\"Engine RPM\",\"value\":\"%u\",\"unit\":\"rpm\"},"
        "{\"id\":\"vehicle_speed\",\"name\":\"Vehicle Speed\",\"value\":\"%u\",\"unit\":\"km/h\"}]",
        (int)ecm_sensors.coolant_temp_c,
        (double)ecm_sensors.battery_mv / 1000.0,
        (unsigned)ecm_sensors.engine_rpm,
        (unsigned)ecm_sensors.vehicle_speed_kmh);
}

static int build_faults(char *buf, int max)
{
    int len = 0;
    len += snprintf(buf + len, max - len, "[");
    int first = 1;
    if (ecm_fault_flags & (1u << 0)) {
        len += snprintf(buf + len, max - len,
            "%s{\"code\":\"P0100\",\"description\":\"Mass or Volume Air Flow Circuit Malfunction\","
            "\"severity\":\"high\",\"status\":\"active\"}",
            first ? "" : ",");
        first = 0;
    }
    if (ecm_fault_flags & (1u << 1)) {
        len += snprintf(buf + len, max - len,
            "%s{\"code\":\"P0200\",\"description\":\"Injector Circuit Malfunction\","
            "\"severity\":\"medium\",\"status\":\"active\"}",
            first ? "" : ",");
        first = 0;
    }
    if (ecm_fault_flags & (1u << 2)) {
        len += snprintf(buf + len, max - len,
            "%s{\"code\":\"P0300\",\"description\":\"Random Misfire Detected — cable fault\","
            "\"severity\":\"high\",\"status\":\"active\"}",
            first ? "" : ",");
    }
    len += snprintf(buf + len, max - len, "]");
    return len;
}

static int build_operations(char *buf, int max)
{
    return snprintf(buf, max,
        "[{\"id\":\"reset\",\"name\":\"ECU Reset\",\"description\":\"Perform a soft reset of the ECU\"},"
        "{\"id\":\"self_test\",\"name\":\"Self-Test\",\"description\":\"Run internal ECU self-diagnostics\"}]");
}

/* ── SOME/IP framing ───────────────────────────────────────────────────── */

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

/* ── TCP receive callback ──────────────────────────────────────────────── */

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
        ecm_fault_flags = 0;
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

/* ── Public init ──────────────────────────────────────────────────────── */

void someip_init(void)
{
    struct tcp_pcb *pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, SOMEIP_PORT);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, on_accept);
    printf("[ECM] SOME/IP listening on port %u\n", SOMEIP_PORT);
}
