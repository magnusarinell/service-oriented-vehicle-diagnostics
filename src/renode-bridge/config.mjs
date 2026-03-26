/**
 * config.mjs — all configuration, sourced from environment variables.
 */

export const config = {
  PORT:                   parseInt(process.env.BRIDGE_PORT          ?? '8787'),
  RENODE_HOST:            process.env.RENODE_HOST                   ?? '127.0.0.1',
  RENODE_PORT:            parseInt(process.env.RENODE_ROBOT_PORT    ?? '55555'),
  RENODE_MACHINE:         process.env.RENODE_MACHINE                ?? 'ecm',
  RENODE_PING_INTERVAL_MS:parseInt(process.env.RENODE_PING_MS       ?? '3000'),
  /** Path to the firmware .map file produced by the K64F build. */
  ECM_MAP_FILE:           process.env.ECM_MAP_FILE
                          ?? 'src/ECM/firmware/build/ecm-firmware.map',
  /** Fault bit definitions — must match diag_data.h */
  FAULT_BITS: {
    P0100: 0,  // bit 0
    P0200: 1,  // bit 1
    P0300: 2,  // bit 2 — "cut cable"
  },
}
