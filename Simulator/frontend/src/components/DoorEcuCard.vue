<script setup lang="ts">
import { useRenode } from '@/composables/useRenode'
import type { FaultCode } from '@/types/renode'

const renode = useRenode()

const FAULT_DEFS: Array<{ code: FaultCode; label: string; desc: string }> = [
  { code: 'B1001', label: 'B1001', desc: 'Door Latch Sensor Fault' },
  { code: 'B1002', label: 'B1002', desc: 'Window Motor Over-Current' },
  { code: 'B1003', label: 'B1003', desc: 'CAN Bus Cable Fault' },
]
</script>

<template>
  <div class="flex gap-4 items-start">
    <!-- Door ECU black box -->
    <div class="flex-1">
      <p class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest mb-2">Door ECU</p>
      <div class="rounded-lg border border-gray-700 bg-gray-900 p-4">
        <div class="flex items-center justify-between mb-2">
          <p class="text-green-400 font-mono text-[10px] tracking-widest uppercase">NXP FRDM-K64F</p>
          <div class="flex items-center gap-1.5">
            <span
              class="w-2 h-2 rounded-full transition-colors"
              :class="renode.ready.value ? 'bg-green-500 shadow-[0_0_5px_#22c55e]' : renode.connected.value ? 'bg-yellow-400' : 'bg-gray-600'"
            />
            <span class="text-[10px] font-mono"
              :class="renode.ready.value ? 'text-green-400' : 'text-gray-600'">
              {{ renode.ready.value ? 'RUNNING' : 'HALTED' }}
            </span>
          </div>
        </div>
        <p class="text-[10px] text-gray-500 font-mono">Renode Emulation · SOME/IP</p>
      </div>
    </div>

    <!-- Fault Injection -->
    <div class="w-44 shrink-0">
      <p class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest mb-2">Fault Injection</p>
      <div class="space-y-1.5">
        <button
          v-for="f in FAULT_DEFS"
          :key="f.code"
          class="w-full flex items-center gap-2 px-2.5 py-1.5 rounded border text-[11px] transition-all disabled:opacity-40 text-left"
          :class="renode.faultBits.value[f.code]
            ? 'bg-red-50 border-red-300 text-red-700 font-semibold'
            : 'bg-white border-gray-200 text-gray-500 hover:border-gray-400 hover:text-gray-700'"
          :disabled="!renode.ready.value"
          :title="f.desc"
          @click="renode.injectFault(f.code)"
        >
          <span
            class="w-1.5 h-1.5 rounded-full shrink-0 transition-colors"
            :class="renode.faultBits.value[f.code] ? 'bg-red-500' : 'bg-gray-300'"
          />
          <span class="font-mono font-bold">{{ f.label }}</span>
        </button>
      </div>
      <button
        class="mt-2 w-full px-2 py-1 text-[10px] rounded border border-gray-200 text-gray-400 hover:border-gray-400 hover:text-gray-600 disabled:opacity-40 transition-colors"
        :disabled="!renode.ready.value"
        @click="renode.clearFaults()"
      >
        Clear faults
      </button>
    </div>
  </div>
</template>
