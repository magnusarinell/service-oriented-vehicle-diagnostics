import { ref, readonly } from 'vue'

export interface GatewayEvent {
  ts: string
  direction: '→' | '←'
  method: string
  detail: string
}

// Singleton — shared across all component instances
const _events      = ref<GatewayEvent[]>([])
const _callCount   = ref(0)
const _replyCount  = ref(0)
const _lastMethod  = ref<string | null>(null)
const _pulsing     = ref(false)

let _pulseTimer: ReturnType<typeof setTimeout> | null = null
let _es: EventSource | null = null
let _initialized = false

function pulse() {
  _pulsing.value = true
  if (_pulseTimer) clearTimeout(_pulseTimer)
  _pulseTimer = setTimeout(() => { _pulsing.value = false }, 600)
}

function connect() {
  _es = new EventSource('/api/v1/logs/stream')
  _es.onmessage = (ev) => {
    try {
      const entry = JSON.parse(ev.data)
      if (entry.cat !== 'DbusEcuGateway') return

      const msg: string = entry.msg ?? ''
      const isOut = msg.startsWith('→')
      const isIn  = msg.startsWith('←')
      if (!isOut && !isIn) return

      const direction = (isOut ? '→' : '←') as '→' | '←'
      const parts = msg.slice(2).trim().split(' ')
      const method = parts[0] ?? ''
      const detail = parts.slice(1).join(' ')

      if (isOut) _callCount.value++
      else       _replyCount.value++

      _lastMethod.value = method
      pulse()

      _events.value.unshift({ ts: entry.ts, direction, method, detail })
      if (_events.value.length > 30) _events.value.length = 30
    } catch { /* ignore */ }
  }
  _es.onerror = () => {
    _es?.close()
    setTimeout(connect, 3000)
  }
}

if (!_initialized) {
  _initialized = true
  connect()
}

export function useGatewayEvents() {
  return {
    events:     readonly(_events),
    callCount:  readonly(_callCount),
    replyCount: readonly(_replyCount),
    lastMethod: readonly(_lastMethod),
    pulsing:    readonly(_pulsing),
  }
}
