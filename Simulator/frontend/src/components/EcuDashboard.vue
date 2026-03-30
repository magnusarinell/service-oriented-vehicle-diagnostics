<script setup lang="ts">
import { ref } from 'vue'
import FaultList from './FaultList.vue'
import DataPanel from './DataPanel.vue'
import OperationPanel from './OperationPanel.vue'
import CapabilityBadges from './CapabilityBadges.vue'

defineProps<{ ecuId: string }>()

type Tab = 'data' | 'faults' | 'operations'
const activeTab = ref<Tab>('data')

const tabs: { id: Tab; label: string }[] = [
  { id: 'data',       label: 'Live Data' },
  { id: 'faults',     label: 'DTCs' },
  { id: 'operations', label: 'Operations' },
]
</script>

<template>
  <div class="rounded-xl border border-border bg-card shadow-sm overflow-hidden">
    <!-- SOVD Explorer header -->
    <div class="px-6 py-4 border-b border-border bg-muted/30">
      <div class="flex items-start justify-between gap-4">
        <div>
          <div class="flex items-center gap-2 mb-1">
            <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">SOVD Explorer</span>
          </div>
          <div class="flex items-center gap-2 font-mono text-sm">
            <span class="font-semibold text-foreground">{{ ecuId }}</span>
            <span class="text-muted-foreground">·</span>
            <span class="text-muted-foreground text-xs">/api/v1/ecu/{{ ecuId }}</span>
          </div>
        </div>
        <CapabilityBadges :ecu-id="ecuId" class="mt-1 shrink-0" />
      </div>
    </div>

    <!-- Tab bar -->
    <div class="flex border-b border-border">
      <button
        v-for="tab in tabs"
        :key="tab.id"
        class="px-5 py-3 text-sm font-medium transition-colors relative"
        :class="activeTab === tab.id
          ? 'text-foreground after:absolute after:bottom-0 after:left-0 after:right-0 after:h-0.5 after:bg-primary'
          : 'text-muted-foreground hover:text-foreground'"
        @click="activeTab = tab.id"
      >
        {{ tab.label }}
      </button>
    </div>

    <!-- Tab content -->
    <div class="p-6">
      <DataPanel      v-if="activeTab === 'data'"        :ecu-id="ecuId" />
      <FaultList      v-else-if="activeTab === 'faults'"     :ecu-id="ecuId" />
      <OperationPanel v-else-if="activeTab === 'operations'" :ecu-id="ecuId" />
    </div>
  </div>
</template>
