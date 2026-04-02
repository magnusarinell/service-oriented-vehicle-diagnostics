<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useEcu } from '@/composables/useEcu'
import type { SovdDataItem } from '@/types/sovd'

const props = defineProps<{ ecuId: string }>()

const { loading, error, fetchData } = useEcu(props.ecuId)
const items = ref<SovdDataItem[]>([])

async function load() {
  const result = await fetchData()
  items.value = result ?? []
}

onMounted(load)
</script>

<template>
  <div class="space-y-4">
    <div class="flex items-center justify-between">
      <h2 class="text-lg font-semibold">Live Data</h2>
      <button
        class="px-3 py-1.5 text-sm rounded-md bg-accent text-accent-foreground hover:bg-accent/80 disabled:opacity-50 transition-colors"
        :disabled="loading"
        @click="load"
      >
        Refresh
      </button>
    </div>

    <p v-if="error" class="text-sm text-destructive">{{ error }}</p>
    <div v-if="loading" class="text-muted-foreground text-sm animate-pulse">Loading…</div>

    <div v-else class="grid grid-cols-2 gap-3 sm:grid-cols-4">
      <div
        v-for="item in items"
        :key="item.id"
        class="rounded-lg border border-border bg-card p-4 flex flex-col gap-1"
      >
        <span class="text-xs text-muted-foreground">{{ item.name }}</span>
        <span class="text-2xl font-bold tabular-nums">
          {{ item.value }}<span class="text-sm font-normal text-muted-foreground ml-1">{{ item.unit }}</span>
        </span>
      </div>
    </div>
  </div>
</template>
