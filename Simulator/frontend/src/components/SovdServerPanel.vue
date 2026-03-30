<script setup lang="ts">
import { ref, onMounted, onUnmounted, nextTick } from 'vue'

interface LogEntry { ts: string; level: 'INFO' | 'WARN' | 'ERRO' | string; cat: string; msg: string }

const entries  = ref<LogEntry[]>([])
const follow   = ref(true)
const logEl    = ref<HTMLElement | null>(null)
const connected = ref(false)
let   es: EventSource | null = null

const LEVEL_CLASS: Record<string, string> = {
  INFO: 'text-gray-500',
  WARN: 'text-yellow-600',
  ERRO: 'text-red-600',
}

function scroll() {
  if (follow.value && logEl.value)
    logEl.value.scrollTop = logEl.value.scrollHeight
}

function connect() {
  es = new EventSource('/api/v1/logs/stream')
  es.onopen = () => { connected.value = true }
  es.onmessage = (ev) => {
    try {
      entries.value.push(JSON.parse(ev.data))
      if (entries.value.length > 500) entries.value.splice(0, entries.value.length - 500)
      nextTick(scroll)
    } catch { /* ignore */ }
  }
  es.onerror = () => {
    connected.value = false
    es?.close()
    setTimeout(connect, 3000)
  }
}

onMounted(connect)
onUnmounted(() => es?.close())
</script>

<template>
  <div class="rounded-xl border border-border bg-card shadow-sm overflow-hidden flex flex-col">
    <div class="px-4 py-2.5 border-b border-border bg-muted/30 flex items-center gap-2 shrink-0">
      <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">SOVD Server</span>
      <span class="text-[10px] font-mono text-muted-foreground">:5000</span>
      <span class="ml-auto w-1.5 h-1.5 rounded-full shrink-0"
            :class="connected ? 'bg-green-500' : 'bg-red-400'" />
      <label class="flex items-center gap-1 text-[10px] text-muted-foreground cursor-pointer select-none ml-1">
        <input type="checkbox" v-model="follow" class="w-3 h-3 accent-primary" />
        follow
      </label>
    </div>

    <div
      ref="logEl"
      class="flex-1 overflow-y-auto font-mono text-[11px] leading-relaxed px-3 py-2 bg-gray-950 min-h-0"
      style="height: 840px;"
    >
      <div
        v-for="(e, i) in entries"
        :key="i"
        class="flex gap-2 hover:bg-white/5 px-1 rounded"
      >
        <span class="shrink-0 text-gray-600">{{ e.ts }}</span>
        <span class="shrink-0 w-9 font-bold" :class="LEVEL_CLASS[e.level] ?? 'text-gray-400'">{{ e.level }}</span>
        <span class="shrink-0 text-blue-400 max-w-[120px] truncate" :title="e.cat">{{ e.cat }}</span>
        <span class="text-gray-300 break-all">{{ e.msg }}</span>
      </div>
      <div v-if="entries.length === 0" class="text-gray-600 italic py-4 text-center">
        Waiting for log entries…
      </div>
    </div>
  </div>
</template>
