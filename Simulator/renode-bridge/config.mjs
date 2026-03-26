/**
 * config.mjs — all configuration, sourced from environment variables.
 */

export const config = {
  PORT:                   parseInt(process.env.BRIDGE_PORT          ?? '8787'),
  RENODE_HOST:            process.env.RENODE_HOST                   ?? '127.0.0.1',
  RENODE_PORT:            parseInt(process.env.RENODE_ROBOT_PORT    ?? '55555'),
  RENODE_MACHINE:         process.env.RENODE_MACHINE                ?? 'door-ecu',
  RENODE_PING_INTERVAL_MS:parseInt(process.env.RENODE_PING_MS       ?? '3000'),
  /** Path to the firmware .map file produced by the K64F build. */
  DOOR_ECU_MAP_FILE:      process.env.DOOR_ECU_MAP_FILE
                          ?? 'DoorECU/build/door-ecu-firmware.map',
  /** Fault bit definitions -- must match DoorECU/src/diag_data.h */
  FAULT_BITS: {
    B1001: 0,  // bit 0 -- Door Latch Sensor Fault
    B1002: 1,  // bit 1 -- Window Motor Over-Current
    B1003: 2,  // bit 2 -- CAN Bus Cable Fault ("cut cable")
  },
}
