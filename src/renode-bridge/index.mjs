/**
 * index.mjs — Fastify entry point for renode-bridge.
 */

import Fastify from 'fastify'
import fastifyWebSocket from '@fastify/websocket'
import { config } from './config.mjs'
import { startRenodeWatchdog } from './lib/renode.mjs'
import wsRoute   from './routes/ws.mjs'
import httpRoutes from './routes/http.mjs'

const fastify = Fastify({ logger: { level: 'info' } })

await fastify.register(fastifyWebSocket)
await fastify.register(wsRoute)
await fastify.register(httpRoutes)

await fastify.listen({ port: config.PORT, host: '0.0.0.0' })
console.log(`[renode-bridge] Listening on port ${config.PORT}`)

await startRenodeWatchdog()
