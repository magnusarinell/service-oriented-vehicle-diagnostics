/**
 * http.mjs — REST convenience endpoints.
 */

import { enqueueRpc } from '../lib/rpc.mjs'
import { renodeReady, getFaultFlagsAddr, readFaultFlags } from '../lib/renode.mjs'
import { broadcast } from '../lib/broadcast.mjs'
import { config } from '../config.mjs'

export default async function httpRoutes(fastify) {
  fastify.get('/health', async () => ({
    status: 'ok',
    renodeReady,
    faultFlagsAddr: getFaultFlagsAddr()
      ? `0x${getFaultFlagsAddr().toString(16)}`
      : null,
  }))

  // Fault flags as a number — polled by SovdServer
  fastify.get('/ecu/door-ecu/fault-flags', async () => ({
    faultFlags: renodeReady ? (await readFaultFlags()) : 0,
    ready: renodeReady,
  }))

  fastify.post('/inject-fault', async (req, reply) => {
    const { code } = req.body ?? {}
    const bit = config.FAULT_BITS[code]
    if (bit === undefined) return reply.code(400).send({ error: `Unknown fault code: ${code}` })
    const addr = getFaultFlagsAddr()
    if (!addr) return reply.code(503).send({ error: 'Renode not connected or map file not found' })
    const current = parseInt(
      (await enqueueRpc('ExecuteCommand',
        [`sysbus ReadDoubleWord 0x${addr.toString(16)}`, config.RENODE_MACHINE])).output?.trim() ?? '0',
      16) || 0
    const next = current | (1 << bit)
    await enqueueRpc('ExecuteCommand',
      [`sysbus WriteDoubleWord 0x${addr.toString(16)} ${next}`, config.RENODE_MACHINE])
    broadcast({ type: 'door_ecu_state', faultFlags: next, ready: true })
    return { ok: true, faultFlags: next }
  })

  fastify.post('/restore', async (_req, reply) => {
    const addr = getFaultFlagsAddr()
    if (!addr) return reply.code(503).send({ error: 'Renode not connected' })
    await enqueueRpc('ExecuteCommand',
      [`sysbus WriteDoubleWord 0x${addr.toString(16)} 0`, config.RENODE_MACHINE])
    broadcast({ type: 'door_ecu_state', faultFlags: 0, ready: true })
    return { ok: true }
  })
}
