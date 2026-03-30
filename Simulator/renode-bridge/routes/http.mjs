/**
 * http.mjs — REST convenience endpoints.
 */

import { enqueueRpc } from '../lib/rpc.mjs'
import { renodeReady, getFaultFlagsAddr, readFaultFlags, getSensorsAddr, readSensors, writeSensorByte } from '../lib/renode.mjs'
import { broadcast } from '../lib/broadcast.mjs'
import { config } from '../config.mjs'

// Sensor struct offsets (must match DoorECU/src/diag_data.h)
const SENSOR_OFFSET = { lock_state: 0, window_pos: 1, mirror_angle: 2, door_ajar: 3 }

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

  // Sensor data — polled by SovdServer ReadDataAsync
  fastify.get('/ecu/door-ecu/sensors', async (_req, reply) => {
    if (!renodeReady) return reply.code(503).send({ error: 'Renode not connected' })
    const sensors = await readSensors()
    if (!sensors) return reply.code(503).send({ error: 'Sensor address not resolved' })
    return sensors
  })

  // Write a single sensor field by name
  fastify.post('/ecu/door-ecu/sensors/:field', async (req, reply) => {
    const { field } = req.params
    const offset = SENSOR_OFFSET[field]
    if (offset === undefined) return reply.code(400).send({ error: `Unknown sensor field: ${field}` })
    if (!renodeReady || !getSensorsAddr()) return reply.code(503).send({ error: 'Renode not connected' })
    const { value } = req.body ?? {}
    if (value === undefined || value === null) return reply.code(400).send({ error: 'value required' })
    await writeSensorByte(offset, Number(value))
    return { ok: true, field, value: Number(value) }
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
