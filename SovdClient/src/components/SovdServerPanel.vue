<script setup lang="ts">
import { ref, onMounted, onUnmounted, nextTick } from 'vue'

interface LogEntry { ts: string; level: 'INFO' | 'WARN' | 'ERRO' | string; cat: string; msg: string }

const entries  = ref<LogEntry[]>([])
const logEl    = ref<HTMLElement | null>(null)
let   es: EventSource | null = null

function scroll() {
  if (logEl.value)
    logEl.value.scrollTop = logEl.value.scrollHeight
}

function connect() {
  es = new EventSource('/api/v1/logs/stream')
  es.onopen = () => {}
  es.onmessage = (ev) => {
    try {
      entries.value.push(JSON.parse(ev.data))
      if (entries.value.length > 500) entries.value.splice(0, entries.value.length - 500)
      nextTick(scroll)
    } catch { /* ignore */ }
  }
  es.onerror = () => {
    es?.close()
    setTimeout(connect, 3000)
  }
}

onMounted(connect)
onUnmounted(() => es?.close())
</script>

<template>
  <div>
    <p class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest mb-2">SOVD Server <span class="font-normal text-[10px] font-mono">:5000</span></p>
    <div class="rounded-lg overflow-hidden border border-border">
      <div
        ref="logEl"
        class="overflow-y-auto overflow-x-hidden font-mono text-[11px] leading-relaxed px-3 py-2 bg-gray-950"
        style="height: 400px;"
      >
        <div
          v-for="(e, i) in entries"
          :key="i"
          class="flex gap-2 hover:bg-white/5 px-1 rounded whitespace-nowrap"
        >
          <span class="shrink-0 text-blue-400 w-[120px] truncate" :title="e.cat">{{ e.cat }}</span>
          <span class="text-gray-300">{{ e.msg }}</span>
        </div>
        <div v-if="entries.length === 0" class="text-gray-600 italic py-4 text-center">
          Waiting for log entries…
        </div>
      </div>
    </div>
  </div>
</template>
