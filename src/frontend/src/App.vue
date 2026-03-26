<script setup lang="ts">
import { ref } from 'vue'
import EcuDashboard from '@/components/EcuDashboard.vue'
import EcmVisualizer from '@/components/ecm/EcmVisualizer.vue'

// Known ECUs — extend this list as more units are added
const knownEcus = ['ecu0']
const selectedEcu = ref(knownEcus[0])
</script>

<template>
  <div class="min-h-screen bg-background">
    <!-- Top bar -->
    <header class="border-b border-border px-6 py-3 flex items-center gap-4">
      <span class="font-semibold text-primary tracking-wide text-sm uppercase">SOVD Diagnostics</span>
      <div class="flex items-center gap-2 ml-auto">
        <label for="ecu-select" class="text-sm text-muted-foreground">ECU</label>
        <select
          id="ecu-select"
          v-model="selectedEcu"
          class="rounded-md border border-border bg-muted text-foreground text-sm px-2 py-1 focus:outline-none focus:ring-1 focus:ring-primary"
        >
          <option v-for="ecu in knownEcus" :key="ecu" :value="ecu">{{ ecu }}</option>
        </select>
      </div>
    </header>

    <!-- Split layout: ECM Visualizer (left) + SOVD Client (right) -->
    <main class="max-w-7xl mx-auto px-4 py-8 grid grid-cols-1 xl:grid-cols-2 gap-6 items-start">
      <EcmVisualizer />
      <EcuDashboard :key="selectedEcu" :ecu-id="selectedEcu" />
    </main>
  </div>
</template>
