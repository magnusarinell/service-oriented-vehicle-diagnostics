<script setup lang="ts">
import { useRenode } from '@/composables/useRenode'
import type { FaultCode } from '@/types/renode'

const renode = useRenode()

const FAULT_DEFS: Array<{ code: FaultCode; label: string; desc: string }> = [
  { code: 'B1001', label: 'B1001', desc: 'Door Latch Sensor Fault' },
  { code: 'B1002', label: 'B1002', desc: 'Window Motor Over-Current' },
  { code: 'B1003', label: 'B1003', desc: 'CAN Bus Cable Fault' },
]

const anyFaultActive = () =>
  renode.faultBits.value.B1001 || renode.faultBits.value.B1002 || renode.faultBits.value.B1003
</script>

<template>
  <section class="rounded-xl border border-border bg-card shadow-sm overflow-hidden">
    <!-- Header -->
    <div class="px-6 py-3 border-b border-border bg-muted/30 flex items-center justify-between">
      <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">Vehicle Network Topology</span>
      <div class="flex items-center gap-2 text-xs">
        <span
          class="w-2 h-2 rounded-full inline-block transition-colors"
          :class="renode.ready.value ? 'bg-green-500' : renode.connected.value ? 'bg-yellow-400' : 'bg-gray-300'"
        />
        <span class="text-muted-foreground">
          {{ renode.ready.value ? 'Renode running' : renode.connected.value ? 'Bridge connected' : 'Bridge offline' }}
        </span>
      </div>
    </div>

    <!-- Topology canvas -->
    <div class="p-6 overflow-x-auto">
      <div class="flex items-center gap-5 min-w-max">

        <!-- ── Fault Injection panel (outside ECU) ───────────────────── -->
        <div class="self-stretch flex flex-col rounded-lg border border-dashed border-gray-300 bg-gray-50 p-4 w-56">
          <p class="text-[11px] font-bold text-gray-400 uppercase tracking-widest mb-3">Fault Injection</p>
          <div class="space-y-2 flex-1">
            <button
              v-for="f in FAULT_DEFS"
              :key="f.code"
              class="w-full flex items-center gap-2.5 px-3 py-2 rounded-md border text-xs transition-all disabled:opacity-40 text-left"
              :class="renode.faultBits.value[f.code]
                ? 'bg-red-50 border-red-300 text-red-700 font-semibold'
                : 'bg-white border-gray-200 text-gray-500 hover:border-gray-400 hover:text-gray-700'"
              :disabled="!renode.ready.value"
              @click="renode.injectFault(f.code)"
            >
              <span
                class="w-2 h-2 rounded-full shrink-0 transition-colors"
                :class="renode.faultBits.value[f.code] ? 'bg-red-500' : 'bg-gray-300'"
              />
              <span class="font-mono font-bold">{{ f.label }}</span>
              <span class="text-[10px] opacity-70 leading-tight">{{ f.desc }}</span>
            </button>
          </div>
          <button
            class="mt-3 w-full px-3 py-1.5 text-xs rounded-md border border-gray-200 text-gray-400 hover:border-gray-400 hover:text-gray-600 disabled:opacity-40 transition-colors"
            :disabled="!renode.ready.value"
            @click="renode.clearFaults()"
          >
            Clear all faults
          </button>
        </div>

        <!-- ── Arrow: injection → ECU ────────────────────────────────── -->
        <div class="flex items-center shrink-0">
          <div
            class="w-8 border-t-2 border-dashed transition-colors"
            :class="anyFaultActive() ? 'border-red-400' : 'border-gray-300'"
          />
          <svg
            class="w-3 h-3 shrink-0 transition-colors"
            :class="anyFaultActive() ? 'text-red-400' : 'text-gray-300'"
            fill="currentColor" viewBox="0 0 8 8"
          >
            <path d="M1 1L7 4L1 7V1z" />
          </svg>
        </div>

        <!-- ── Door ECU (PCB chip box) ───────────────────────────────── -->
        <div class="shrink-0 flex flex-col items-center">
          <!-- PCB board -->
          <div class="rounded-lg border-2 border-gray-600 bg-gray-800 p-4 w-52">
            <!-- IC chip body -->
            <div class="relative rounded border border-gray-500 bg-gray-900 px-3 py-3 text-center mb-4">
              <!-- IC notch -->
              <div class="absolute -top-px left-1/2 -translate-x-1/2 w-5 h-1.5 bg-gray-900 border border-gray-500 rounded-b-sm" />
              <p class="text-green-400 font-mono text-[10px] tracking-widest uppercase">NXP FRDM-K64F</p>
              <p class="text-white font-bold text-sm mt-0.5">Door ECU</p>
              <!-- Status LED -->
              <div class="flex items-center justify-center gap-1.5 mt-2.5">
                <span
                  class="w-2.5 h-2.5 rounded-full border border-black/20 transition-colors"
                  :class="renode.ready.value ? 'bg-green-400 shadow-[0_0_6px_#4ade80]' : 'bg-gray-700'"
                />
                <span class="font-mono text-[10px] tracking-wide"
                  :class="renode.ready.value ? 'text-green-400' : 'text-gray-600'">
                  {{ renode.ready.value ? 'RUNNING' : 'HALTED' }}
                </span>
              </div>
            </div>
            <!-- PCB decorative components (capacitors / resistors) -->
            <div class="flex items-center justify-between px-1 mb-3">
              <div class="flex flex-col gap-0.5">
                <div v-for="i in 3" :key="i" class="w-6 h-2 bg-gray-700 border border-gray-600 rounded-sm" />
              </div>
              <div class="flex gap-1">
                <div v-for="i in 3" :key="i" class="w-2 h-4 bg-gray-600 rounded-sm border border-gray-500" />
              </div>
              <div class="w-6 h-6 rounded-full bg-gray-700 border border-gray-600" />
            </div>
            <p class="text-center text-gray-600 font-mono text-[9px] tracking-[0.2em] uppercase">Renode Emulation</p>
          </div>
          <!-- Edge connector pins (OBD / diagnostic connector) -->
          <div class="flex gap-0.5 mt-0.5 px-4">
            <div
              v-for="n in 16"
              :key="n"
              class="w-[11px] h-5 bg-yellow-400 border border-yellow-600 rounded-b-sm"
            />
          </div>
          <p class="text-[9px] text-gray-400 font-mono tracking-widest uppercase mt-1">ECU Connector</p>
        </div>

        <!-- ── SOME/IP bus line ─────────────────────────────────────── -->
        <div class="flex flex-col items-center gap-1 shrink-0">
          <span class="text-[10px] text-gray-400 font-mono uppercase tracking-wider">SOME/IP</span>
          <div class="flex items-center gap-1">
            <div
              class="w-10 h-0.5 transition-colors"
              :class="renode.faultBits.value.B1003 ? 'border-t-2 border-dashed border-red-400 bg-transparent' : 'bg-gray-400'"
            />
            <!-- Break / connected indicator -->
            <template v-if="renode.faultBits.value.B1003">
              <svg class="w-4 h-4 text-red-500 shrink-0" fill="none" stroke="currentColor" stroke-width="2.5" viewBox="0 0 24 24">
                <path stroke-linecap="round" stroke-linejoin="round" d="M6 18L18 6M6 6l12 12" />
              </svg>
            </template>
            <template v-else>
              <div class="w-2 h-2 rounded-full bg-gray-400 shrink-0" />
            </template>
            <div
              class="w-10 h-0.5 transition-colors"
              :class="renode.faultBits.value.B1003 ? 'border-t-2 border-dashed border-red-400 bg-transparent' : 'bg-gray-400'"
            />
          </div>
          <span
            v-if="renode.faultBits.value.B1003"
            class="text-[10px] text-red-500 font-mono font-bold"
          >
            B1003 ✕
          </span>
        </div>

        <!-- ── Zone Controller (PCB chip box) ───────────────────────── -->
        <div class="shrink-0 flex flex-col items-center">
          <p class="text-[9px] text-gray-400 font-mono tracking-widest uppercase mb-1">ECU Connector</p>
          <!-- Edge connector pins (facing DoorECU side) -->
          <div class="flex gap-0.5 mb-0.5 px-4">
            <div
              v-for="n in 14"
              :key="n"
              class="w-[11px] h-5 bg-yellow-400 border border-yellow-600 rounded-t-sm"
            />
          </div>
          <!-- PCB board -->
          <div class="rounded-lg border-2 border-slate-500 bg-slate-800 p-4 w-48">
            <div class="relative rounded border border-slate-600 bg-slate-900 px-3 py-3 text-center mb-4">
              <div class="absolute -top-px left-1/2 -translate-x-1/2 w-5 h-1.5 bg-slate-900 border border-slate-600 rounded-b-sm" />
              <p class="text-blue-400 font-mono text-[10px] tracking-widest uppercase">vsomeip</p>
              <p class="text-white font-bold text-sm mt-0.5">Zone Controller</p>
              <span class="inline-block mt-2 text-[10px] px-2 py-0.5 bg-slate-700 text-slate-300 rounded font-mono">
                docker only
              </span>
            </div>
            <!-- PCB decorative components -->
            <div class="flex items-center justify-between px-1 mb-3">
              <div class="flex flex-col gap-0.5">
                <div v-for="i in 3" :key="i" class="w-5 h-2 bg-slate-700 border border-slate-600 rounded-sm" />
              </div>
              <div class="flex gap-1">
                <div v-for="i in 3" :key="i" class="w-2 h-4 bg-slate-600 rounded-sm border border-slate-500" />
              </div>
              <div class="w-5 h-5 rounded-full bg-slate-700 border border-slate-600" />
            </div>
            <p class="text-center text-slate-600 font-mono text-[9px] tracking-[0.2em] uppercase">CAN + LIN Bus</p>
          </div>
        </div>

      </div>
    </div>
  </section>
</template>
