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
  { id: 'faults',     label: 'Faults' },
  { id: 'operations', label: 'Operations' },
]
</script>

<template>
  <div class="rounded-xl border border-border bg-card shadow-lg overflow-hidden">
    <!-- Header -->
    <div class="px-6 py-4 border-b border-border flex items-center justify-between gap-4">
      <div>
        <h1 class="font-bold text-xl">{{ ecuId }}</h1>
        <CapabilityBadges :ecu-id="ecuId" class="mt-1" />
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
      <DataPanel      v-if="activeTab === 'data'"       :ecu-id="ecuId" />
      <FaultList      v-else-if="activeTab === 'faults'"    :ecu-id="ecuId" />
      <OperationPanel v-else-if="activeTab === 'operations'" :ecu-id="ecuId" />
    </div>
  </div>
</template>
