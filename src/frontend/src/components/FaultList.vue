<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useEcu } from '@/composables/useEcu'
import type { SovdFault } from '@/types/sovd'

const props = defineProps<{ ecuId: string }>()

const { loading, error, fetchFaults, clearFaults } = useEcu(props.ecuId)
const faults = ref<SovdFault[]>([])
const clearMessage = ref<string | null>(null)

async function load() {
  const result = await fetchFaults()
  faults.value = result ?? []
}

async function handleClear() {
  await clearFaults()
  clearMessage.value = 'Fault codes cleared'
  await load()
  setTimeout(() => (clearMessage.value = null), 3000)
}

const severityClass: Record<string, string> = {
  high:   'bg-red-900/60 text-red-300 border-red-700',
  medium: 'bg-yellow-900/60 text-yellow-300 border-yellow-700',
  low:    'bg-blue-900/60 text-blue-300 border-blue-700',
}

const statusClass: Record<string, string> = {
  active:   'text-red-400',
  pending:  'text-yellow-400',
  inactive: 'text-muted-foreground',
}

onMounted(load)
</script>

<template>
  <div class="space-y-4">
    <div class="flex items-center justify-between">
      <h2 class="text-lg font-semibold">Fault Codes (DTCs)</h2>
      <button
        class="px-3 py-1.5 text-sm rounded-md bg-destructive text-destructive-foreground hover:bg-destructive/80 disabled:opacity-50 transition-colors"
        :disabled="loading || faults.length === 0"
        @click="handleClear"
      >
        Clear All Faults
      </button>
    </div>

    <p v-if="clearMessage" class="text-sm text-green-400">{{ clearMessage }}</p>
    <p v-if="error" class="text-sm text-destructive">{{ error }}</p>

    <div v-if="loading" class="text-muted-foreground text-sm animate-pulse">Loading…</div>

    <div v-else-if="faults.length === 0" class="text-muted-foreground text-sm py-4 text-center border border-dashed border-border rounded-lg">
      No fault codes recorded
    </div>

    <div v-else class="space-y-2">
      <div
        v-for="fault in faults"
        :key="fault.code"
        class="flex items-start gap-3 rounded-lg border p-3"
        :class="severityClass[fault.severity] ?? 'border-border bg-card'"
      >
        <span class="font-mono font-bold text-base w-16 shrink-0">{{ fault.code }}</span>
        <div class="flex-1 min-w-0">
          <p class="text-sm leading-snug">{{ fault.description }}</p>
          <p class="text-xs mt-0.5 capitalize" :class="statusClass[fault.status]">
            {{ fault.status }}
          </p>
        </div>
        <span class="text-xs capitalize opacity-70 shrink-0">{{ fault.severity }}</span>
      </div>
    </div>
  </div>
</template>
