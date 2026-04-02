/**
 * ws.mjs — WebSocket route.
 *
 * Messages from client -> bridge:
 *   { type: "cut_cable" }          -- set B1003 bit in door_ecu_fault_flags
 *   { type: "restore_cable" }      -- clear B1003 bit
 *   { type: "inject_fault", code: "B1001"|"B1002"|"B1003" }
 *   { type: "clear_faults" }       -- clear all bits
 *   { type: "ping" }               -- responds with pong
 *
 * Messages bridge -> client:
 *   { type: "renode_connected" | "renode_disconnected" }
 *   { type: "door_ecu_state", faultFlags: number, ready: boolean }
 *   { type: "pong" }
 */

import { addClient, removeClient, broadcast } from '../lib/broadcast.mjs'
import { enqueueRpc } from '../lib/rpc.mjs'
import { renodeReady, getFaultFlagsAddr, readFaultFlags } from '../lib/renode.mjs'
import { config } from '../config.mjs'

async function writeFaultFlags(value) {
  const addr = getFaultFlagsAddr()
  if (!addr) return false
  const hex = `0x${addr.toString(16)}`
  await enqueueRpc('ExecuteCommand',
    [`sysbus WriteDoubleWord ${hex} ${value}`, config.RENODE_MACHINE])
  return true
}



let _currentFlags = 0

async function pushState(ws) {
  _currentFlags = await readFaultFlags()
  const msg = JSON.stringify({ type: 'door_ecu_state', faultFlags: _currentFlags, ready: renodeReady })
  ws ? ws.send(msg) : broadcast({ type: 'door_ecu_state', faultFlags: _currentFlags, ready: renodeReady })
}

async function setFaultBit(bitIndex, set) {
  _currentFlags = await readFaultFlags()
  if (set) {
    _currentFlags |= (1 << bitIndex)
  } else {
    _currentFlags &= ~(1 << bitIndex)
  }
  await writeFaultFlags(_currentFlags)
  broadcast({ type: 'door_ecu_state', faultFlags: _currentFlags, ready: renodeReady })
}

export default async function wsRoute(fastify) {
  fastify.get('/ws/renode', { websocket: true }, (socket) => {
    addClient(socket)

    // Send current state to newly connected client
    pushState(socket)

    socket.on('message', async (raw) => {
      let msg
      try { msg = JSON.parse(raw.toString()) } catch { return }

      switch (msg.type) {
        case 'ping':
          socket.send(JSON.stringify({ type: 'pong' }))
          break

        case 'cut_cable':
          await setFaultBit(config.FAULT_BITS.B1003, true)
          break

        case 'restore_cable':
          await setFaultBit(config.FAULT_BITS.B1003, false)
          break

        case 'inject_fault': {
          const bit = config.FAULT_BITS[msg.code]
          if (bit !== undefined) await setFaultBit(bit, true)
          break
        }

        case 'clear_faults':
          await writeFaultFlags(0)
          _currentFlags = 0
          broadcast({ type: 'door_ecu_state', faultFlags: 0, ready: renodeReady })
          break

        default:
          break
      }
    })

    socket.on('close', () => removeClient(socket))
  })
}

// Periodic state push to all clients (every 2 s)
setInterval(() => { if (renodeReady) pushState(null) }, 2000)
