<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { useEcu } from '@/composables/useEcu'
import type { SovdDataItem, SovdFault } from '@/types/sovd'

const props = defineProps<{ ecuId: string }>()
const ecu = useEcu(props.ecuId)

const data   = ref<SovdDataItem[]>([])
const faults = ref<SovdFault[]>([])
const opMsg  = ref<string | null>(null)

let timer: ReturnType<typeof setInterval> | null = null

async function refresh() {
  const [d, f] = await Promise.all([ecu.fetchData(), ecu.fetchFaults()])
  if (d) data.value   = d
  if (f) faults.value = f
}

async function injectFault(code: string, description: string, severity: string) {
  opMsg.value = null
  const r = await ecu.executeOperation('inject_fault', { parameters: { code, description, severity } })
  opMsg.value = r?.message ?? null
  await refresh()
  setTimeout(() => (opMsg.value = null), 3000)
}

async function clearFaults() {
  opMsg.value = null
  await ecu.clearFaults()
  opMsg.value = 'Faults cleared'
  await refresh()
  setTimeout(() => (opMsg.value = null), 3000)
}

const INJECTABLE = [
  { code: 'P0100', description: 'Mass Air Flow Circuit Malfunction', severity: 'high' },
  { code: 'P0200', description: 'Injector Circuit Malfunction',       severity: 'medium' },
  { code: 'P0300', description: 'Random/Multiple Cylinder Misfire',   severity: 'high' },
]

const severityClass: Record<string, string> = {
  high:   'bg-red-50 border-red-300 text-red-700',
  medium: 'bg-yellow-50 border-yellow-300 text-yellow-700',
  low:    'bg-blue-50 border-blue-300 text-blue-700',
}

function isActive(code: string) {
  return faults.value.some(f => f.code === code)
}

onMounted(() => { refresh(); timer = setInterval(refresh, 4000) })
onUnmounted(() => { if (timer) clearInterval(timer) })
</script>

<template>
  <div>
    <p class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest mb-2">
      Zone Controller <span class="font-normal font-mono text-[10px]">vsomeip3 · 0x1111</span>
    </p>

    <div class="rounded-lg border border-border bg-card overflow-hidden">

      <!-- Live data -->
      <div class="px-4 py-3 border-b border-border grid grid-cols-2 gap-x-4 gap-y-1">
        <div
          v-for="item in data"
          :key="item.id"
          class="flex items-baseline justify-between"
        >
          <span class="text-[10px] text-muted-foreground font-mono truncate">{{ item.name }}</span>
          <span class="text-[11px] font-semibold font-mono ml-2 shrink-0">{{ item.value }}<span class="text-[9px] font-normal text-muted-foreground ml-0.5">{{ item.unit }}</span></span>
        </div>
        <div v-if="data.length === 0" class="col-span-2 text-[10px] text-muted-foreground italic py-1">
          {{ ecu.error.value ? ecu.error.value : 'Loading…' }}
        </div>
      </div>

      <!-- Fault injection -->
      <div class="px-4 py-3 border-b border-border">
        <p class="text-[10px] font-bold text-muted-foreground uppercase tracking-widest mb-2">Fault Injection</p>
        <div class="flex flex-wrap gap-1.5">
          <button
            v-for="f in INJECTABLE"
            :key="f.code"
            class="px-2.5 py-1 rounded border text-[10px] font-mono font-bold transition-all"
            :class="isActive(f.code)
              ? (severityClass[f.severity] ?? 'bg-gray-100 border-gray-300 text-gray-700')
              : 'bg-white border-gray-200 text-gray-500 hover:border-gray-400 hover:text-gray-700'"
            :title="f.description"
            @click="injectFault(f.code, f.description, f.severity)"
          >
            <span
              class="inline-block w-1.5 h-1.5 rounded-full mr-1 align-middle"
              :class="isActive(f.code) ? 'bg-current' : 'bg-gray-300'"
            />{{ f.code }}
          </button>
          <button
            class="px-2.5 py-1 rounded border text-[10px] border-gray-200 text-gray-400 hover:border-gray-400 hover:text-gray-600 transition-colors ml-auto"
            @click="clearFaults"
          >
            Clear all
          </button>
        </div>
        <p v-if="opMsg" class="text-[10px] mt-1.5 text-green-600">{{ opMsg }}</p>
      </div>

      <!-- Active faults -->
      <div class="px-4 py-3">
        <p class="text-[10px] font-bold text-muted-foreground uppercase tracking-widest mb-2">
          Active Faults <span class="font-normal">({{ faults.length }})</span>
        </p>
        <div v-if="faults.length === 0" class="text-[10px] text-muted-foreground italic">No active faults</div>
        <div v-else class="space-y-1">
          <div
            v-for="f in faults"
            :key="f.code"
            class="flex items-center gap-2 text-[10px]"
          >
            <span
              class="px-1.5 py-0.5 rounded font-mono font-bold border"
              :class="severityClass[f.severity] ?? 'bg-gray-100 border-gray-300 text-gray-700'"
            >{{ f.code }}</span>
            <span class="text-muted-foreground truncate">{{ f.description }}</span>
            <span class="ml-auto text-[9px] text-muted-foreground shrink-0">{{ f.status }}</span>
          </div>
        </div>
      </div>

    </div>
  </div>
</template>

