#pragma once
#include <stdint.h>

/* Fault injection flags
 * Renode injects faults by writing to this address via:
 *   sysbus WriteDoubleWord <FAULT_FLAGS_ADDR> <value>
 *
 * Bit 0 = B1001 (Door Latch Sensor Fault)
 * Bit 1 = B1002 (Window Motor Over-Current)
 * Bit 2 = B1003 (CAN Bus Cable Fault -- "cut cable")
 *
 * The symbol address is resolved at link time and published in the .map file.
 * renode-bridge reads the .map file to find the address at startup.
 */
extern volatile uint32_t door_ecu_fault_flags;

/* Live sensor data
 * Values can also be overwritten by Renode for simulation scenarios.
 */
typedef struct {
    uint8_t  lock_state;       /* 0 = unlocked, 1 = locked   */
    uint8_t  window_pos;       /* % open  -- default 100     */
    int8_t   mirror_angle;     /* degrees -- default 0       */
    uint8_t  door_ajar;        /* 0 = closed, 1 = ajar       */
} door_ecu_data_t;

extern volatile door_ecu_data_t door_ecu_sensors;

void diag_data_init(void);
