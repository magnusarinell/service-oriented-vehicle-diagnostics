<script setup lang="ts">
import type { FaultCode } from '@/types/renode'

const props = defineProps<{
  faultBits: { P0100: boolean; P0200: boolean; P0300: boolean }
  ready: boolean
  connected: boolean
  cableCut: boolean
}>()

const emit = defineEmits<{
  cutCable: []
  restoreCable: []
  injectFault: [code: FaultCode]
  clearFaults: []
}>()

const FAULT_DEFS: Array<{ code: FaultCode; label: string; desc: string }> = [
  { code: 'P0100', label: 'P0100', desc: 'MAF Circuit Malfunction' },
  { code: 'P0200', label: 'P0200', desc: 'Injector Circuit Malfunction' },
  { code: 'P0300', label: 'P0300', desc: 'Random Misfire (cable fault)' },
]
</script>

<template>
  <div class="rounded-xl border border-border bg-card shadow-lg overflow-hidden">
    <!-- Header -->
    <div class="px-6 py-4 border-b border-border flex items-center justify-between gap-4">
      <div>
        <h2 class="font-bold text-xl">ECM — Engine Control Module</h2>
        <p class="text-xs text-muted-foreground mt-0.5">NXP FRDM-K64F · Renode emulation</p>
      </div>
      <!-- Connection status -->
      <div class="flex items-center gap-2 text-sm">
        <span
          class="inline-block w-2.5 h-2.5 rounded-full"
          :class="ready ? 'bg-green-500' : connected ? 'bg-yellow-500' : 'bg-red-500'"
        />
        <span class="text-muted-foreground">
          {{ ready ? 'Renode running' : connected ? 'Bridge connected' : 'Bridge offline' }}
        </span>
      </div>
    </div>

    <div class="p-6 space-y-6">

      <!-- Cable control -->
      <div class="flex items-center gap-4">
        <button
          v-if="!cableCut"
          class="px-4 py-2 rounded-lg bg-destructive text-destructive-foreground text-sm font-medium
                 hover:bg-destructive/80 transition-colors disabled:opacity-50"
          :disabled="!ready"
          @click="emit('cutCable')"
        >
          ✂ Cut Cable
        </button>
        <button
          v-else
          class="px-4 py-2 rounded-lg bg-green-600 text-white text-sm font-medium
                 hover:bg-green-500 transition-colors"
          @click="emit('restoreCable')"
        >
          ↩ Restore Cable
        </button>

        <div
          v-if="cableCut"
          class="flex items-center gap-2 text-sm text-destructive font-medium animate-pulse"
        >
          <span>⚠</span> Cable severed — P0300 active
        </div>
      </div>

      <!-- Fault injection -->
      <div>
        <p class="text-xs font-semibold text-muted-foreground uppercase tracking-wide mb-2">
          Fault Injection
        </p>
        <div class="flex flex-wrap gap-2">
          <button
            v-for="f in FAULT_DEFS"
            :key="f.code"
            class="px-3 py-1.5 rounded-md text-xs font-mono border transition-colors
                   disabled:opacity-40"
            :class="faultBits[f.code]
              ? 'bg-destructive/20 border-destructive text-destructive'
              : 'bg-muted border-border text-muted-foreground hover:border-foreground'"
            :disabled="!ready"
            :title="f.desc"
            @click="emit('injectFault', f.code)"
          >
            {{ f.label }}
            <span v-if="faultBits[f.code]" class="ml-1">●</span>
          </button>
          <button
            class="px-3 py-1.5 rounded-md text-xs border border-border text-muted-foreground
                   hover:border-foreground transition-colors disabled:opacity-40"
            :disabled="!ready"
            @click="emit('clearFaults')"
          >
            Clear all
          </button>
        </div>
      </div>

      <!-- Active fault list -->
      <div v-if="faultBits.P0100 || faultBits.P0200 || faultBits.P0300">
        <p class="text-xs font-semibold text-muted-foreground uppercase tracking-wide mb-2">
          Active in ECM
        </p>
        <ul class="space-y-1">
          <li
            v-for="f in FAULT_DEFS.filter(f => faultBits[f.code])"
            :key="f.code"
            class="flex items-center gap-2 text-sm"
          >
            <span class="font-mono text-destructive">{{ f.label }}</span>
            <span class="text-muted-foreground">{{ f.desc }}</span>
          </li>
        </ul>
      </div>
      <div v-else class="text-sm text-muted-foreground italic">No active faults in ECM</div>

    </div>
  </div>
</template>
