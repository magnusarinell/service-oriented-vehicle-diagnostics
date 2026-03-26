#pragma once
#include <stdint.h>

/* ── Fault injection flags ──────────────────────────────────────────────────
 * Renode injects faults by writing to this address via:
 *   sysbus WriteDoubleWord <FAULT_FLAGS_ADDR> <value>
 *
 * Bit 0 = P0100 (MAF circuit malfunction)
 * Bit 1 = P0200 (Injector circuit malfunction)
 * Bit 2 = P0300 (Random misfire — "cut cable")
 *
 * The symbol address is resolved at link time and published in the .map file.
 * renode-bridge reads the .map file to find the address at startup.
 */
extern volatile uint32_t ecm_fault_flags;

/* ── Live sensor data ───────────────────────────────────────────────────────
 * Values can also be overwritten by Renode for simulation scenarios.
 */
typedef struct {
    int16_t  coolant_temp_c;      /* °C  — default 82  */
    uint16_t battery_mv;          /* mV  — default 12600 */
    uint16_t engine_rpm;          /* rpm — default 0   */
    uint16_t vehicle_speed_kmh;   /* km/h — default 0  */
} ecm_sensor_data_t;

extern volatile ecm_sensor_data_t ecm_sensors;

void diag_data_init(void);
