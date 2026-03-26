import { ref, readonly, onUnmounted } from 'vue'
import { type DoorEcuState, type FaultCode, type BridgeMessage, parseFaultBits } from '@/types/renode'

const WS_URL = '/ws/renode'

export function useRenode() {
  const connected  = ref(false)
  const ready      = ref(false)
  const faultFlags = ref(0)
  const faultBits  = ref(parseFaultBits(0))
  const cableCut   = ref(false)

  let ws: WebSocket | null = null
  let pingTimer: ReturnType<typeof setInterval> | null = null

  function connect() {
    const url = WS_URL.startsWith('ws') ? WS_URL
      : `${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}${WS_URL}`
    ws = new WebSocket(url)

    ws.onopen = () => {
      connected.value = true
      pingTimer = setInterval(() => ws?.send(JSON.stringify({ type: 'ping' })), 10_000)
    }

    ws.onclose = () => {
      connected.value = false
      ready.value = false
      if (pingTimer) clearInterval(pingTimer)
      // Reconnect after 3 s
      setTimeout(connect, 3_000)
    }

    ws.onmessage = (ev) => {
      let msg: BridgeMessage
      try { msg = JSON.parse(ev.data) } catch { return }

      if (msg.type === 'renode_connected')    { ready.value = true }
      if (msg.type === 'renode_disconnected') { ready.value = false }
      if (msg.type === 'door_ecu_state') {
        ready.value      = msg.ready
        faultFlags.value = msg.faultFlags
        faultBits.value  = parseFaultBits(msg.faultFlags)
        cableCut.value   = faultBits.value.B1003
      }
    }
  }

  function send(msg: object) {
    if (ws?.readyState === WebSocket.OPEN) ws.send(JSON.stringify(msg))
  }

  function cutCable()       { send({ type: 'cut_cable' }) }
  function restoreCable()   { send({ type: 'restore_cable' }) }
  function clearFaults()    { send({ type: 'clear_faults' }) }
  function injectFault(code: FaultCode) { send({ type: 'inject_fault', code }) }

  connect()
  onUnmounted(() => { ws?.close(); if (pingTimer) clearInterval(pingTimer) })

  return {
    connected:  readonly(connected),
    ready:      readonly(ready),
    faultFlags: readonly(faultFlags),
    faultBits:  readonly(faultBits),
    cableCut:   readonly(cableCut),
    cutCable,
    restoreCable,
    clearFaults,
    injectFault,
  }
}
