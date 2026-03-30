<script setup lang="ts">
import SovdExplorer       from '@/components/SovdExplorer.vue'
import SovdServerPanel    from '@/components/SovdServerPanel.vue'
import ZoneControllerCard from '@/components/ZoneControllerCard.vue'
import DoorEcuCard        from '@/components/DoorEcuCard.vue'

const ECU_ID = 'door-ecu'
</script>

<template>
  <div class="min-h-screen bg-background flex flex-col">
    <header class="border-b border-border bg-card px-6 py-3 flex items-center gap-3 shrink-0">
      <span class="font-semibold text-foreground tracking-wide text-sm">SOVD Diagnostics</span>
      <span class="text-muted-foreground text-xs font-mono">service-oriented vehicle diagnostics</span>
    </header>

    <main class="flex-1 flex gap-0 overflow-hidden" style="height: calc(100vh - 49px);">
      <!-- Left: SOVD Explorer -->
      <div class="shrink-0 overflow-y-auto p-6" style="width: 860px;">
        <SovdExplorer :ecu-id="ECU_ID" />
      </div>

      <!-- Right: SOVD Server + ECUs -->
      <div class="w-[640px] shrink-0 border-l border-border overflow-y-auto bg-muted/5 p-4 space-y-4">
        <SovdServerPanel />
        <ZoneControllerCard />
        <DoorEcuCard />
      </div>
    </main>
  </div>
</template>
