/**
 * broadcast.mjs — fan-out a message to all connected WebSocket clients.
 */

/** @type {Set<import('@fastify/websocket').WebSocket>} */
const _clients = new Set()

export function addClient(ws)    { _clients.add(ws) }
export function removeClient(ws) { _clients.delete(ws) }

export function broadcast(msg) {
  const text = JSON.stringify(msg)
  for (const ws of _clients) {
    try { ws.send(text) } catch { _clients.delete(ws) }
  }
}
