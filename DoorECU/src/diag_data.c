#include "diag_data.h"

/* Placed in known RAM -- Renode writes here for fault injection.
 * The .map file will contain the resolved address under the symbol
 * "door_ecu_fault_flags". renode-bridge reads that at startup. */
volatile uint32_t door_ecu_fault_flags = 0;

volatile door_ecu_data_t door_ecu_sensors = {
    .lock_state   = 1,   /* locked   */
    .window_pos   = 100, /* fully up */
    .mirror_angle = 0,
    .door_ajar    = 0,   /* closed   */
};

void diag_data_init(void)
{
    door_ecu_fault_flags = 0;
}
