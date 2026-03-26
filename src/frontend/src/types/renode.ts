// Types for Renode/ECM bridge communication

export interface EcmState {
  faultFlags: number
  ready: boolean
}

export type FaultCode = 'P0100' | 'P0200' | 'P0300'

export interface EcmFaultBits {
  P0100: boolean  // bit 0 — MAF circuit
  P0200: boolean  // bit 1 — Injector circuit
  P0300: boolean  // bit 2 — cable fault / random misfire
}

export function parseFaultBits(flags: number): EcmFaultBits {
  return {
    P0100: (flags & (1 << 0)) !== 0,
    P0200: (flags & (1 << 1)) !== 0,
    P0300: (flags & (1 << 2)) !== 0,
  }
}

export type BridgeMessage =
  | { type: 'renode_connected' }
  | { type: 'renode_disconnected' }
  | { type: 'ecm_state'; faultFlags: number; ready: boolean }
  | { type: 'pong' }

export type ClientMessage =
  | { type: 'ping' }
  | { type: 'cut_cable' }
  | { type: 'restore_cable' }
  | { type: 'inject_fault'; code: FaultCode }
  | { type: 'clear_faults' }
