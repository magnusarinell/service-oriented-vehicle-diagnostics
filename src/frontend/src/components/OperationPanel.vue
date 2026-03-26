<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useEcu } from '@/composables/useEcu'
import type { SovdOperation, SovdOperationResult } from '@/types/sovd'

const props = defineProps<{ ecuId: string }>()

const { loading, error, fetchOperations, executeOperation } = useEcu(props.ecuId)
const operations = ref<SovdOperation[]>([])
const results = ref<Record<string, SovdOperationResult>>({})
const executing = ref<string | null>(null)

async function load() {
  const result = await fetchOperations()
  operations.value = result ?? []
}

async function handleExecute(op: SovdOperation) {
  executing.value = op.id
  const result = await executeOperation(op.id)
  if (result) results.value[op.id] = result
  executing.value = null
}

onMounted(load)
</script>

<template>
  <div class="space-y-4">
    <h2 class="text-lg font-semibold">Operations</h2>

    <p v-if="error" class="text-sm text-destructive">{{ error }}</p>
    <div v-if="loading" class="text-muted-foreground text-sm animate-pulse">Loading…</div>

    <div v-else-if="operations.length === 0" class="text-muted-foreground text-sm py-4 text-center border border-dashed border-border rounded-lg">
      No operations available
    </div>

    <div v-else class="space-y-3">
      <div
        v-for="op in operations"
        :key="op.id"
        class="rounded-lg border border-border bg-card p-4 space-y-2"
      >
        <div class="flex items-start justify-between gap-2">
          <div>
            <p class="font-medium">{{ op.name }}</p>
            <p class="text-sm text-muted-foreground">{{ op.description }}</p>
          </div>
          <button
            class="shrink-0 px-3 py-1.5 text-sm rounded-md bg-primary text-primary-foreground hover:bg-primary/80 disabled:opacity-50 transition-colors"
            :disabled="executing === op.id"
            @click="handleExecute(op)"
          >
            <span v-if="executing === op.id">Running…</span>
            <span v-else>Execute</span>
          </button>
        </div>

        <div v-if="results[op.id]" class="rounded-md px-3 py-2 text-sm"
          :class="results[op.id].status === 'ok' ? 'bg-green-900/40 text-green-300' : 'bg-destructive/20 text-destructive-foreground'">
          <span class="font-mono font-semibold uppercase">{{ results[op.id].status }}</span>
          <span v-if="results[op.id].message"> — {{ results[op.id].message }}</span>
        </div>
      </div>
    </div>
  </div>
</template>
