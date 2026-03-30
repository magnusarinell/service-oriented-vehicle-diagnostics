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
    <p v-if="error" class="text-sm text-destructive">{{ error }}</p>
    <div v-if="loading" class="text-muted-foreground text-sm animate-pulse">Loading…</div>

    <div v-else-if="operations.length === 0"
      class="text-muted-foreground text-sm py-4 text-center border border-dashed border-border rounded-lg">
      No operations available
    </div>

    <div v-else class="space-y-3">
      <div
        v-for="op in operations"
        :key="op.id"
        class="rounded-lg border border-border bg-card overflow-hidden"
      >
        <!-- URL bar — Vector SOVD Explorer style -->
        <div class="flex items-center gap-2 px-4 py-2.5 bg-muted/60 border-b border-border">
          <span class="shrink-0 text-[11px] font-bold px-1.5 py-0.5 rounded bg-green-100 text-green-700 border border-green-200 font-mono">
            POST
          </span>
          <code class="text-xs text-muted-foreground font-mono truncate">
            /api/v1/ecu/{{ ecuId }}/operations/{{ op.id }}/execute
          </code>
        </div>

        <!-- Body -->
        <div class="p-4 space-y-3">
          <div class="flex items-start justify-between gap-3">
            <div>
              <p class="font-medium text-sm">{{ op.name }}</p>
              <p class="text-sm text-muted-foreground mt-0.5">{{ op.description }}</p>
            </div>
            <button
              class="shrink-0 px-3 py-1.5 text-sm rounded-md bg-primary text-primary-foreground hover:bg-primary/80 disabled:opacity-50 transition-colors font-mono"
              :disabled="executing === op.id"
              @click="handleExecute(op)"
            >
              <span v-if="executing === op.id">Running…</span>
              <span v-else>▶ Execute</span>
            </button>
          </div>

          <!-- Request body preview -->
          <div>
            <p class="text-[11px] font-semibold text-muted-foreground uppercase tracking-wide mb-1">Request body</p>
            <pre class="text-xs bg-muted rounded px-3 py-2 font-mono text-muted-foreground border border-border">{}</pre>
          </div>

          <!-- Response -->
          <div v-if="results[op.id]">
            <p class="text-[11px] font-semibold text-muted-foreground uppercase tracking-wide mb-1">Response</p>
            <div
              class="rounded px-3 py-2 text-sm border font-mono"
              :class="results[op.id].status === 'ok'
                ? 'bg-green-50 text-green-700 border-green-200'
                : 'bg-red-50 text-red-700 border-red-200'"
            >
              <span class="font-bold text-xs uppercase">{{ results[op.id].status }}</span>
              <span v-if="results[op.id].message" class="ml-2 font-sans font-normal text-xs">{{ results[op.id].message }}</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>
