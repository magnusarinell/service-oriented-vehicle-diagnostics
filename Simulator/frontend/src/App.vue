<script setup lang="ts">
import SovdExplorer from '@/components/SovdExplorer.vue'
import TopologyView from '@/components/TopologyView.vue'

const ECU_ID = 'door-ecu'
</script>

<template>
  <div class="min-h-screen bg-background">
    <!-- Top bar -->
    <header class="border-b border-border bg-card px-6 py-3 flex items-center gap-3">
      <span class="font-semibold text-foreground tracking-wide text-sm">SOVD Diagnostics</span>
      <span class="text-muted-foreground text-xs font-mono">service-oriented vehicle diagnostics</span>
    </header>

    <main class="max-w-5xl mx-auto px-6 py-8 space-y-6">
      <SovdExplorer :ecu-id="ECU_ID" />
      <TopologyView />
    </main>
  </div>
</template>
