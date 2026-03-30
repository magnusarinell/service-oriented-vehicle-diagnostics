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
  <div class="rounded-xl border border-border bg-card shadow-sm overflow-hidden">
    <div class="px-4 py-3 border-b border-border bg-muted/30 flex items-center gap-2">
      <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">Door ECU</span>
      <span class="text-[10px] font-mono text-muted-foreground">NXP FRDM-K64F</span>
      <div class="ml-auto flex items-center gap-1.5">
        <span
          class="w-2 h-2 rounded-full transition-colors"
          :class="renode.ready.value ? 'bg-green-500 shadow-[0_0_5px_#22c55e]' : renode.connected.value ? 'bg-yellow-400' : 'bg-gray-300'"
        />
        <span class="text-[10px] text-muted-foreground">
          {{ renode.ready.value ? 'running' : renode.connected.value ? 'bridge connected' : 'offline' }}
        </span>
      </div>
    </div>

    <div class="flex gap-0 items-stretch">
      <!-- PCB chip -->
      <div class="flex-1 p-4 flex flex-col gap-3">
        <!-- IC body -->
        <div class="rounded border-2 border-gray-600 bg-gray-800 p-3 text-center relative">
          <div class="absolute -top-px left-1/2 -translate-x-1/2 w-4 h-1.5 bg-gray-800 border border-gray-600 rounded-b-sm" />
          <p class="text-green-400 font-mono text-[9px] tracking-widest uppercase">MK64FN1M0VLL12</p>
          <p class="text-white font-semibold text-xs mt-0.5">Door Control Module</p>
          <div class="flex items-center justify-center gap-1.5 mt-2">
            <span
              class="w-2 h-2 rounded-full border border-black/20 transition-colors"
              :class="renode.ready.value ? 'bg-green-400 shadow-[0_0_6px_#4ade80]' : 'bg-gray-700'"
            />
            <span class="font-mono text-[9px]"
              :class="renode.ready.value ? 'text-green-400' : 'text-gray-600'">
              {{ renode.ready.value ? 'RUNNING' : 'HALTED' }}
            </span>
          </div>
        </div>

        <!-- PCB deco -->
        <div class="flex items-center justify-between px-1">
          <div class="flex flex-col gap-0.5">
            <div v-for="i in 3" :key="i" class="w-5 h-1.5 bg-gray-700 border border-gray-600 rounded-sm" />
          </div>
          <div class="flex gap-0.5">
            <div v-for="i in 3" :key="i" class="w-1.5 h-3.5 bg-gray-600 rounded-sm border border-gray-500" />
          </div>
          <div class="w-4 h-4 rounded-full bg-gray-700 border border-gray-600" />
        </div>

        <!-- Connector pins (bottom) -->
        <div class="flex gap-0.5 justify-center">
          <div v-for="n in 14" :key="n" class="w-2 h-4 bg-yellow-400 border border-yellow-600 rounded-b-sm" />
        </div>
      </div>

      <!-- Fault injection -->
      <div class="w-44 shrink-0 border-l border-border bg-muted/10 p-3 flex flex-col gap-2">
        <p class="text-[10px] font-bold text-muted-foreground uppercase tracking-wider">Fault Injection</p>
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
          class="mt-auto w-full px-2 py-1 text-[10px] rounded border border-gray-200 text-gray-400 hover:border-gray-400 hover:text-gray-600 disabled:opacity-40 transition-colors"
          :disabled="!renode.ready.value"
          @click="renode.clearFaults()"
        >
          Clear faults
        </button>
      </div>
    </div>
  </div>
</template>
