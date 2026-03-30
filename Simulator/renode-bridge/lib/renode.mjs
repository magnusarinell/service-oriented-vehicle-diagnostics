/**
 * renode.mjs — Renode connection lifecycle.
 *
 * Polls the Renode Robot server with a "version" ping every few seconds.
 * Exposes `renodeReady` flag and the resolved fault_flags RAM address.
 */

import http from 'node:http'
import { config } from '../config.mjs'
import { broadcast } from './broadcast.mjs'
import { callXmlRpc, enqueueRpc } from './rpc.mjs'
import fs from 'node:fs'

export let renodeReady = false

/**
 * Resolve the ecm_fault_flags address from the firmware .map file.
 * Returns undefined if the map file is not found yet.
 */
export function resolveFaultFlagsAddr() {
  try {
    const mapPath = config.DOOR_ECU_MAP_FILE
    const text = fs.readFileSync(mapPath, 'utf8')
    // GNU ld map: lines like "  0x00000000200001a0  door_ecu_fault_flags"
    const m = text.match(/0x([0-9a-fA-F]+)\s+door_ecu_fault_flags/)
    if (m) return parseInt(m[1], 16)
  } catch {
    // map file not available yet
  }
  return undefined
}

let _faultFlagsAddr = undefined
let _sensorsAddr = undefined

export function getFaultFlagsAddr() { return _faultFlagsAddr }
export function getSensorsAddr()    { return _sensorsAddr }

/**
 * Resolve door_ecu_sensors base address from the .map file.
 * Struct layout (matches diag_data.h):
 *   offset 0: lock_state  (uint8_t)
 *   offset 1: window_pos  (uint8_t, 0-100 %)
 *   offset 2: mirror_angle (int8_t, degrees)
 *   offset 3: door_ajar   (uint8_t, 0/1)
 */
export function resolveSensorsAddr() {
  try {
    const text = fs.readFileSync(config.DOOR_ECU_MAP_FILE, 'utf8')
    const m = text.match(/0x([0-9a-fA-F]+)\s+door_ecu_sensors/)
    if (m) return parseInt(m[1], 16)
  } catch { }
  return undefined
}

async function readWord(addr) {
  const hex = `0x${addr.toString(16)}`
  const r = await enqueueRpc('ExecuteCommand',
    [`sysbus ReadDoubleWord ${hex}`, config.RENODE_MACHINE])
  const raw = r.return?.trim() || r.output?.trim() || '0'
  return parseInt(raw, 16) || 0
}

async function writeWord(addr, value) {
  const hex = `0x${addr.toString(16)}`
  const r = await enqueueRpc('ExecuteCommand',
    [`sysbus WriteDoubleWord ${hex} ${value}`, config.RENODE_MACHINE])
  if (r.status !== 'PASS')
    console.warn(`[renode-bridge] writeWord ${hex}=${value} status=${r.status} err=${r.error ?? r.output}`)
}

/**
 * Read the door_ecu_sensors struct as one 32-bit word.
 * Struct layout (little-endian, matches diag_data.h):
 *   byte 0: lock_state   (0 = unlocked, 1 = locked)
 *   byte 1: window_pos   (0-100 %)
 *   byte 2: mirror_angle (signed degrees)
 *   byte 3: door_ajar    (0/1)
 */
export async function readSensors() {
  if (!_sensorsAddr) return null
  const word = await readWord(_sensorsAddr)
  const lock_state   =  (word       ) & 0xFF
  const window_pos   =  (word >>>  8) & 0xFF
  const mirror_raw   =  (word >>> 16) & 0xFF
  const door_ajar    =  (word >>> 24) & 0xFF
  const mirror_angle = mirror_raw > 127 ? mirror_raw - 256 : mirror_raw
  return { lock_state, window_pos, mirror_angle, door_ajar }
}

/**
 * Write a single byte field of door_ecu_sensors via a word read-modify-write.
 * offset: 0=lock_state, 1=window_pos, 2=mirror_angle, 3=door_ajar
 */
export async function writeSensorByte(offset, value) {
  if (!_sensorsAddr) return
  const current = await readWord(_sensorsAddr)
  const shift   = offset * 8
  const mask    = ~(0xFF << shift) >>> 0
  const next    = ((current & mask) | ((value & 0xFF) << shift)) >>> 0
  await writeWord(_sensorsAddr, next)
}

export async function readFaultFlags() {
  if (!_faultFlagsAddr) return 0
  const hex = `0x${_faultFlagsAddr.toString(16)}`
  const r = await callXmlRpc('ExecuteCommand',
    [`sysbus ReadDoubleWord ${hex}`, config.RENODE_MACHINE])
  const raw = r.return?.trim() || r.output?.trim() || '0'
  return parseInt(raw, 16) || 0
}

export async function startRenodeWatchdog() {
  async function ping() {
    try {
      const r = await callXmlRpc('ExecuteCommand', ['version', config.RENODE_MACHINE])
      const wasReady = renodeReady
      renodeReady = r.status === 'PASS'
      if (renodeReady && !wasReady) {
        console.log('[renode-bridge] Renode connected')
        _faultFlagsAddr = resolveFaultFlagsAddr()
        _sensorsAddr    = resolveSensorsAddr()
        if (_faultFlagsAddr !== undefined) {
          console.log(`[renode-bridge] fault_flags @ 0x${_faultFlagsAddr.toString(16)}`)
        }
        if (_sensorsAddr !== undefined) {
          console.log(`[renode-bridge] sensors     @ 0x${_sensorsAddr.toString(16)}`)
        }
        broadcast({ type: 'renode_connected' })
      } else if (!renodeReady && wasReady) {
        console.warn('[renode-bridge] Renode disconnected')
        broadcast({ type: 'renode_disconnected' })
      }
    } catch {
      if (renodeReady) {
        renodeReady = false
        broadcast({ type: 'renode_disconnected' })
      }
    }
  }

  await ping()
  setInterval(ping, config.RENODE_PING_INTERVAL_MS)
}
