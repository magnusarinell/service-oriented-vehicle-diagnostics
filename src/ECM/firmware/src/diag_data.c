#include "diag_data.h"

/* Placed in known RAM — Renode writes here for fault injection.
 * The .map file will contain the resolved address under the symbol
 * "ecm_fault_flags". renode-bridge reads that at startup. */
volatile uint32_t ecm_fault_flags = 0;

volatile ecm_sensor_data_t ecm_sensors = {
    .coolant_temp_c    = 82,
    .battery_mv        = 12600,
    .engine_rpm        = 0,
    .vehicle_speed_kmh = 0,
};

void diag_data_init(void)
{
    ecm_fault_flags = 0;
}
