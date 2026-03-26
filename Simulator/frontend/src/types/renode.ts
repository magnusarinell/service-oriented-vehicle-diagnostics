// Types for Renode/DoorECU bridge communication

export interface DoorEcuState {
  faultFlags: number
  ready: boolean
}

export type FaultCode = 'B1001' | 'B1002' | 'B1003'

export interface DoorEcuFaultBits {
  B1001: boolean  // bit 0 — Door Latch
  B1002: boolean  // bit 1 — Window Motor
  B1003: boolean  // bit 2 — CAN Cable Fault
}

export function parseFaultBits(flags: number): DoorEcuFaultBits {
  return {
    B1001: (flags & (1 << 0)) !== 0,
    B1002: (flags & (1 << 1)) !== 0,
    B1003: (flags & (1 << 2)) !== 0,
  }
}

export type BridgeMessage =
  | { type: 'renode_connected' }
  | { type: 'renode_disconnected' }
  | { type: 'door_ecu_state'; faultFlags: number; ready: boolean }
  | { type: 'pong' }

export type ClientMessage =
  | { type: 'ping' }
  | { type: 'cut_cable' }
  | { type: 'restore_cable' }
  | { type: 'inject_fault'; code: FaultCode }
  | { type: 'clear_faults' }
