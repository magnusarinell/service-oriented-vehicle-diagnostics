<script setup lang="ts">
import SovdExplorer       from '@/components/SovdExplorer.vue'
import SovdServerPanel    from '@/components/SovdServerPanel.vue'
import ZoneControllerCard from '@/components/ZoneControllerCard.vue'
import DoorEcuCard        from '@/components/DoorEcuCard.vue'

const ECU_ID = 'door-ecu'
</script>

<template>
  <div class="min-h-screen bg-background flex flex-col">
    <header class="px-6 py-4 shrink-0 flex items-center gap-3">
      <img src="/favicon.svg" alt="" class="w-7 h-7" />
      <div>
        <h1 class="font-bold text-foreground tracking-wide text-lg leading-tight">SOVD Diagnostics</h1>
        <p class="text-muted-foreground text-xs font-mono">service-oriented vehicle diagnostics</p>
      </div>
    </header>

    <main class="flex-1 flex gap-0 overflow-hidden" style="height: calc(100vh - 72px);">
      <!-- Left: SOVD Explorer -->
      <div class="shrink-0 overflow-y-auto p-6 pt-4" style="width: 860px;">
        <SovdExplorer :ecu-id="ECU_ID" />
      </div>

      <!-- Right: SOVD Server + ECUs -->
      <div class="w-[640px] shrink-0 overflow-y-auto bg-muted/5 p-4 space-y-2">
        <SovdServerPanel />

        <!-- Communication: D-Bus / HTTP REST -->
        <div class="flex items-center gap-2 px-2 py-1">
          <div class="flex-1 border-t border-dashed border-gray-300" />
          <span class="text-[10px] font-mono text-gray-400 uppercase tracking-wider shrink-0">HTTP · SOVD API</span>
          <div class="flex-1 border-t border-dashed border-gray-300" />
        </div>

        <ZoneControllerCard />

        <!-- Communication: SOME/IP -->
        <div class="flex items-center gap-2 px-2 py-1">
          <div class="flex-1 border-t border-dashed border-gray-300" />
          <span class="text-[10px] font-mono text-gray-400 uppercase tracking-wider shrink-0">SOME/IP</span>
          <div class="flex-1 border-t border-dashed border-gray-300" />
        </div>

        <DoorEcuCard />
      </div>
    </main>
  </div>
</template>
