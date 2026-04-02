<script setup lang="ts">
import SovdExplorer         from '@/components/SovdExplorer.vue'
import SovdServerPanel      from '@/components/SovdServerPanel.vue'
import SomeIpGatewayPanel   from '@/components/SomeIpGatewayPanel.vue'
import ZoneControllerCard   from '@/components/ZoneControllerCard.vue'

const ECU_ID = 'zone-controller'
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
      <div class="shrink-0 flex flex-col p-6 pt-4 min-h-0" style="width: 860px;">
        <SovdExplorer class="flex-1 min-h-0" />
      </div>

      <!-- Right: SOVD Server + ECUs -->
      <div class="w-[640px] shrink-0 overflow-y-auto bg-muted/5 p-4 space-y-2">
        <!-- HPC: SOVD Server + D-Bus + SomeIpGateway -->
        <div class="rounded-xl border border-border shadow-sm overflow-hidden">
          <div class="px-4 py-2 bg-muted/30 border-b border-border">
            <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">HPC</span>
          </div>
          <div class="divide-y divide-border">
            <!-- SOVD Server box -->
            <div class="p-4">
              <SovdServerPanel />
            </div>
            <!-- D-Bus box -->
            <div class="px-4 py-2.5 bg-muted/20 flex items-center gap-3">
              <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">D-Bus</span>
              <div class="flex-1 border-t border-dashed border-border/50" />
            </div>
            <!-- SomeIpGateway box -->
            <SomeIpGatewayPanel />
          </div>
        </div>

        <!-- SOME/IP connection -->
        <div class="flex items-center gap-2 px-2 py-1 opacity-40">
          <div class="flex-1 border-t border-dashed border-gray-400" />
          <span class="text-[10px] font-mono text-gray-500 uppercase tracking-wider shrink-0">SOME/IP</span>
          <div class="flex-1 border-t border-dashed border-gray-400" />
        </div>

        <ZoneControllerCard :ecu-id="ECU_ID" />


      </div>
    </main>
  </div>
</template>
