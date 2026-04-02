import { ref, readonly } from 'vue'
import { type FaultCode, type BridgeMessage, parseFaultBits } from '@/types/renode'

const WS_URL = '/ws/renode'

// ── Singleton state (module-level, shared across all component instances) ──
const _connected  = ref(false)
const _ready      = ref(false)
const _faultFlags = ref(0)
const _faultBits  = ref(parseFaultBits(0))
const _cableCut   = ref(false)

let _ws: WebSocket | null = null
let _pingTimer: ReturnType<typeof setInterval> | null = null
let _initialized = false

function connect() {
  const url = WS_URL.startsWith('ws') ? WS_URL
    : `${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}${WS_URL}`
  _ws = new WebSocket(url)

  _ws.onopen = () => {
    _connected.value = true
    _pingTimer = setInterval(() => _ws?.send(JSON.stringify({ type: 'ping' })), 10_000)
  }

  _ws.onclose = () => {
    _connected.value = false
    _ready.value = false
    if (_pingTimer) clearInterval(_pingTimer)
    setTimeout(connect, 3_000)
  }

  _ws.onmessage = (ev) => {
    let msg: BridgeMessage
    try { msg = JSON.parse(ev.data) } catch { return }

    if (msg.type === 'renode_connected')    { _ready.value = true }
    if (msg.type === 'renode_disconnected') { _ready.value = false }
    if (msg.type === 'door_ecu_state') {
      _ready.value      = msg.ready
      _faultFlags.value = msg.faultFlags
      _faultBits.value  = parseFaultBits(msg.faultFlags)
      _cableCut.value   = _faultBits.value.B1003
    }
  }
}

function send(msg: object) {
  if (_ws?.readyState === WebSocket.OPEN) _ws.send(JSON.stringify(msg))
}

function cutCable()       { send({ type: 'cut_cable' }) }
function restoreCable()   { send({ type: 'restore_cable' }) }
function clearFaults()    { send({ type: 'clear_faults' }) }
function injectFault(code: FaultCode) { send({ type: 'inject_fault', code }) }

// Start the WebSocket connection once at module load time
if (!_initialized) {
  _initialized = true
  connect()
}

// ── Composable (returns shared singleton state) ──────────────────────────
export function useRenode() {
  return {
    connected:  readonly(_connected),
    ready:      readonly(_ready),
    faultFlags: readonly(_faultFlags),
    faultBits:  readonly(_faultBits),
    cableCut:   readonly(_cableCut),
    cutCable,
    restoreCable,
    clearFaults,
    injectFault,
  }
}
