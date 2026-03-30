<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useEcu } from '@/composables/useEcu'
import type { SovdCapability } from '@/types/sovd'

const props = defineProps<{ ecuId: string }>()

const { fetchCapabilities } = useEcu(props.ecuId)
const capabilities = ref<SovdCapability[]>([])

const categoryClass: Record<string, string> = {
  'read':       'bg-blue-50 text-blue-700 border-blue-200',
  'read-write': 'bg-purple-50 text-purple-700 border-purple-200',
  'execute':    'bg-green-50 text-green-700 border-green-200',
}

onMounted(async () => {
  capabilities.value = (await fetchCapabilities()) ?? []
})
</script>

<template>
  <div class="flex flex-wrap gap-2">
    <span
      v-for="cap in capabilities"
      :key="cap.name"
      class="inline-flex items-center gap-1 rounded-full border px-2.5 py-0.5 text-xs font-medium"
      :class="categoryClass[cap.category] ?? 'bg-muted text-muted-foreground border-border'"
      :title="cap.description"
    >
      {{ cap.name }}
    </span>
  </div>
</template>
