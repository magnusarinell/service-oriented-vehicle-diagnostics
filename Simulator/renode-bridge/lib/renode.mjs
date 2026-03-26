/**
 * renode.mjs — Renode connection lifecycle.
 *
 * Polls the Renode Robot server with a "version" ping every few seconds.
 * Exposes `renodeReady` flag and the resolved fault_flags RAM address.
 */

import { callXmlRpc } from './rpc.mjs'
import { config } from '../config.mjs'
import { broadcast } from './broadcast.mjs'
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

export function getFaultFlagsAddr() { return _faultFlagsAddr }

export async function startRenodeWatchdog() {
  async function ping() {
    try {
      const r = await callXmlRpc('ExecuteCommand', ['version', config.RENODE_MACHINE])
      const wasReady = renodeReady
      renodeReady = r.status === 'PASS'
      if (renodeReady && !wasReady) {
        console.log('[renode-bridge] Renode connected')
        _faultFlagsAddr = resolveFaultFlagsAddr()
        if (_faultFlagsAddr !== undefined) {
          console.log(`[renode-bridge] fault_flags @ 0x${_faultFlagsAddr.toString(16)}`)
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
