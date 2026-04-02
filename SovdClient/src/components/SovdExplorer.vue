<script setup lang="ts">
import { ref, computed, watch, onUnmounted } from 'vue'
import { useEcu } from '@/composables/useEcu'
import type { SovdFault, SovdDataItem, SovdOperation, SovdCapability } from '@/types/sovd'

const props = defineProps<{ ecuId: string }>()

const ecu = useEcu(props.ecuId)

// ── Tree node definition ─────────────────────────────────────────────
interface TreeNode {
  key: string
  label: string
  url: string
  method?: 'GET' | 'POST' | 'DELETE'
  description?: string
  canSubscribe?: boolean
  severity?: string
  badge?: string
  children?: TreeNode[]
  infoOnly?: boolean
  placeholder?: boolean
}

// ── Remote data ───────────────────────────────────────────────────────
const capabilities = ref<SovdCapability[]>([])
const dataItems     = ref<SovdDataItem[]>([])
const faults        = ref<SovdFault[]>([])
const operations    = ref<SovdOperation[]>([])

async function loadCapabilities() { const r = await ecu.fetchCapabilities(); if (r) capabilities.value = r }
async function loadData()         { const r = await ecu.fetchData();         if (r) dataItems.value   = r }
async function loadFaults()       {
  const r = await ecu.fetchFaults()
  if (r) {
    faults.value = r
    if (r.length > 0) expanded.value = { ...expanded.value, faults: true }
  }
}
async function loadOperations()   { const r = await ecu.fetchOperations();   if (r) operations.value  = r }

loadCapabilities(); loadData(); loadFaults(); loadOperations()

// ── Subscriptions (SSE simulated via polling + WS trigger) ───────────
// SOVD standard uses SSE; since this demo server doesn't implement it,
// we poll every 3 s AND react to bridge WebSocket pushes.
const subscriptions = ref<Record<string, boolean>>({})
const subTimers: Record<string, ReturnType<typeof setInterval>> = {}

const SUB_LOADERS: Record<string, () => Promise<void>> = {
  faults: loadFaults,
  data:   loadData,
}

function toggleSubscribe(key: string, e: Event) {
  e.stopPropagation()
  if (subscriptions.value[key]) {
    subscriptions.value = { ...subscriptions.value, [key]: false }
    clearInterval(subTimers[key]); delete subTimers[key]
  } else {
    subscriptions.value = { ...subscriptions.value, [key]: true }
    SUB_LOADERS[key]?.()
    subTimers[key] = setInterval(() => SUB_LOADERS[key]?.(), 3000)
  }
}

onUnmounted(() => Object.values(subTimers).forEach(t => clearInterval(t)))

// ── Tree (computed — reactive to loaded data) ────────────────────────
const base = computed(() => `/api/v1/ecu/${props.ecuId}`)

const tree = computed<TreeNode>(() => ({
  key:   'ecus',
  label: 'ecu',
  url:   '/api/v1/ecu',
  method: 'GET',
  children: [
    {
      key:    props.ecuId,
      label:  props.ecuId,
      url:    base.value,
      method: 'GET',
      children: [
        {
          key:   'capabilities',
          label: 'capabilities',
          url:   `${base.value}/capabilities`,
          method: 'GET',
        },
        {
          key:         'data',
          label:       'data',
          url:         `${base.value}/data`,
          method:      'GET',
          canSubscribe: true,
          children: dataItems.value.map(item => ({
            key:          `data.${item.id}`,
            label:        item.id,
            url:          `${base.value}/data/${item.id}`,
            method:       'GET' as const,
            description:  `${item.value} ${item.unit}`.trim(),
            canSubscribe: true,
          })),
        },
        {
          key:         'faults',
          label:       'faults',
          url:         `${base.value}/faults`,
          method:      'GET',
          canSubscribe: true,
          children: [
            ...faults.value.map(f => ({
              key:         `fault.${f.code}`,
              label:       f.code,
              url:         `${base.value}/faults`,
              description: f.description,
              badge:       f.status,
              severity:    f.severity,
              infoOnly:    true,
            })),
            {
              key:         'faults.clear',
              label:       'clear',
              url:         `${base.value}/faults`,
              method:      'DELETE' as const,
              description: 'Clear all stored fault codes',
            },
          ],
        },
        {
          key:    'operations',
          label:  'operations',
          url:    `${base.value}/operations`,
          method: 'GET',
          children: operations.value.map(op => ({
            key:         `op.${op.id}`,
            label:       op.id,
            url:         `${base.value}/operations/${op.id}/execute`,
            method:      'POST' as const,
            description: op.name,
          })),
        },
      ],
    },
    {
      key:         'apps',
      label:       'apps',
      url:         '/api/v1/apps',
      method:      'GET',
      placeholder: true,
    },
    {
      key:         'functions',
      label:       'functions',
      url:         '/api/v1/functions',
      method:      'GET',
      placeholder: true,
    },
  ],
}))

// ── Tree UI state ─────────────────────────────────────────────────────
// Collect all node keys to expand everything by default
function allKeys(node: TreeNode): string[] {
  return [node.key, ...(node.children ?? []).flatMap(allKeys)]
}
const expanded    = ref<Record<string, boolean>>({})
watch(tree, (t) => {
  const keys = allKeys(t)
  for (const k of keys) if (expanded.value[k] === undefined) expanded.value[k] = true
}, { immediate: true })
const selectedKey = ref<string | null>(props.ecuId)

// Flatten tree for O(1) rendering (no recursion in template)
interface FlatNode { node: TreeNode; depth: number }
const flatTree = computed<FlatNode[]>(() => {
  const result: FlatNode[] = []
  function walk(node: TreeNode, depth: number) {
    result.push({ node, depth })
    if (expanded.value[node.key] && node.children?.length) {
      for (const child of node.children) walk(child, depth + 1)
    }
  }
  walk(tree.value, 0)
  return result
})

function toggleExpand(node: TreeNode, e: Event) {
  e.stopPropagation()
  expanded.value = { ...expanded.value, [node.key]: !expanded.value[node.key] }
}

function handleNodeClick(node: TreeNode) {
  if (node.placeholder) return
  selectedKey.value = node.key
  response.value    = null
  requestBody.value = node.method === 'POST' ? '{\n\n}' : ''
}

function findNode(key: string, node: TreeNode): TreeNode | null {
  if (node.key === key) return node
  for (const child of node.children ?? []) {
    const hit = findNode(key, child)
    if (hit) return hit
  }
  return null
}

const selectedNode = computed(() =>
  selectedKey.value ? findNode(selectedKey.value, tree.value) : null,
)

// ── Request / Response ────────────────────────────────────────────────
const requestBody = ref('')
interface ApiResponse { status: number; body: unknown; ms: number; ts: string }
const response  = ref<ApiResponse | null>(null)
const requesting = ref(false)

async function sendRequest() {
  const node = selectedNode.value
  if (!node || node.infoOnly) return
  requesting.value = true
  const t0 = performance.now()
  try {
    const init: RequestInit = { method: node.method ?? 'GET' }
    if (node.method === 'POST') {
      init.headers = { 'Content-Type': 'application/json' }
      init.body    = requestBody.value
    }
    const res  = await fetch(node.url, init)
    const ms   = Math.round(performance.now() - t0)
    const body = res.status === 204 ? null : await res.json().catch(() => null)
    response.value = { status: res.status, body, ms, ts: new Date().toLocaleTimeString() }
    if (node.key === 'faults.clear') await loadFaults()
  } catch (e) {
    response.value = { status: 0, body: String(e), ms: 0, ts: new Date().toLocaleTimeString() }
  } finally {
    requesting.value = false
  }
}

// ── Style helpers ─────────────────────────────────────────────────────
const METHOD_COLORS: Record<string, string> = {
  GET:    'bg-blue-50   text-blue-700   border-blue-200',
  POST:   'bg-green-50  text-green-700  border-green-200',
  DELETE: 'bg-red-50    text-red-700    border-red-200',
}
function methodColor(m?: string) { return METHOD_COLORS[m ?? ''] ?? 'bg-gray-50 text-gray-600 border-gray-200' }

const SEVERITY_DOTS: Record<string, string> = {
  high:   'bg-red-500',
  medium: 'bg-yellow-400',
  low:    'bg-blue-400',
}
function severityDot(s?: string) { return SEVERITY_DOTS[s ?? ''] ?? 'bg-gray-400' }

const anySubscribed = computed(() => Object.values(subscriptions.value).some(Boolean))
</script>

<template>
  <div class="flex flex-col flex-1 min-h-0" style="max-width: 820px;">

    <!-- ── Title (outside card) ───────────────────────────────────── -->
    <div class="flex items-center gap-3 mb-2">
      <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">SOVD Explorer</span>
      <div v-if="anySubscribed" class="ml-auto flex items-center gap-1.5 text-[11px] text-orange-600 font-medium">
        <span class="w-1.5 h-1.5 rounded-full bg-orange-500 animate-pulse" />
        SSE polling active
      </div>
    </div>

    <!-- ── Card body ──────────────────────────────────────────────── -->
    <div class="rounded-xl border border-border bg-card shadow-sm overflow-hidden flex flex-col flex-1 min-h-0">

    <!-- ── Body: tree left + request/response right ───────────────── -->
    <div class="flex divide-x divide-border flex-1 min-h-0" style="max-width: 780px;">

      <!-- ── Object Dictionary ────────────────────────────────────── -->
      <div class="w-64 shrink-0 overflow-y-auto bg-muted/10 flex flex-col">
        <div class="px-4 py-2 border-b border-border bg-muted/20">
          <span class="text-[10px] font-bold text-muted-foreground uppercase tracking-wider">
            Object Dictionary
          </span>
        </div>

        <div class="py-1 flex-1">
          <div
            v-for="{ node, depth } in flatTree"
            :key="node.key"
            class="group relative flex items-center gap-1.5 pr-2 py-[5px] cursor-pointer select-none text-xs transition-colors"
            :class="[
              selectedKey === node.key
                ? 'bg-primary/10 text-primary'
                : node.placeholder
                  ? 'text-muted-foreground/40 cursor-default'
                  : node.infoOnly
                    ? 'text-muted-foreground hover:bg-muted/30'
                    : 'hover:bg-muted/50 text-foreground',
            ]"
            :style="{ paddingLeft: `${10 + depth * 14}px` }"
            @click="handleNodeClick(node)"
          >
            <!-- Expand / leaf indicator -->
            <button
              v-if="node.children?.length"
              class="shrink-0 w-5 h-5 flex items-center justify-center rounded border border-border bg-muted hover:bg-accent text-muted-foreground text-[11px] font-bold transition-colors"
              @click.stop="toggleExpand(node, $event)"
            >{{ expanded[node.key] ? '▾' : '▸' }}</button>
            <span v-else-if="node.severity" class="w-5 shrink-0 flex items-center justify-center">
              <span class="inline-block w-2 h-2 rounded-full" :class="severityDot(node.severity)" />
            </span>
            <span v-else class="w-5 shrink-0 text-center text-muted-foreground/30 text-[10px]">·</span>

            <!-- Label -->
            <span
              class="flex-1 font-mono truncate"
              :class="depth === 0 ? 'font-semibold text-foreground' : ''"
            >{{ node.label }}</span>

            <!-- Placeholder badge -->
            <span
              v-if="node.placeholder"
              class="shrink-0 text-[9px] px-1.5 py-0.5 rounded bg-muted text-muted-foreground/50 border border-border font-mono"
            >planned</span>

            <!-- Description (value for data items / fault desc on hover) -->
            <span
              v-if="node.description"
              class="hidden group-hover:inline text-muted-foreground truncate max-w-[90px] text-[10px] mr-1"
            >{{ node.description }}</span>

            <!-- Fault status badge -->
            <span
              v-if="node.badge"
              class="shrink-0 text-[9px] font-bold px-1 py-0.5 rounded uppercase"
              :class="node.badge === 'active' ? 'bg-red-50 text-red-600' : 'bg-yellow-50 text-yellow-600'"
            >{{ node.badge }}</span>

            <!-- Method badge (when not subscribable) -->
            <span
              v-if="node.method && !node.canSubscribe && !node.placeholder"
              class="shrink-0 text-[9px] font-bold px-1 py-0.5 rounded border font-mono"
              :class="methodColor(node.method)"
            >{{ node.method }}</span>

            <!-- Subscribe toggle (visible on hover, always if subscribed) -->
            <button
              v-if="node.canSubscribe"
              class="shrink-0 text-[9px] font-bold px-1.5 py-0.5 rounded border font-mono transition-all"
              :class="subscriptions[node.key]
                ? 'bg-orange-50 text-orange-600 border-orange-200'
                : 'opacity-0 group-hover:opacity-100 bg-muted text-muted-foreground border-border hover:bg-muted/80'"
              :title="subscriptions[node.key] ? 'Unsubscribe' : 'Subscribe (poll 3 s)'"
              @click="toggleSubscribe(node.key, $event)"
            >
              {{ subscriptions[node.key] ? '◉ sub' : '○ sub' }}
            </button>
          </div>
        </div>
      </div>

      <!-- ── Request / Response ────────────────────────────────────── -->
      <div class="flex-1 flex flex-col overflow-hidden">

        <!-- Empty state -->
        <div
          v-if="!selectedNode"
          class="flex-1 flex flex-col items-center justify-center gap-2 text-muted-foreground"
        >
          <svg class="w-8 h-8 opacity-20" fill="none" stroke="currentColor" stroke-width="1.5" viewBox="0 0 24 24">
            <path stroke-linecap="round" stroke-linejoin="round"
              d="M3.75 9.776c.112-.017.227-.026.344-.026h15.812c.117 0 .232.009.344.026m-16.5 0a2.25 2.25 0 00-1.883 2.542l.857 6a2.25 2.25 0 002.227 1.932H19.05a2.25 2.25 0 002.227-1.932l.857-6a2.25 2.25 0 00-1.883-2.542m-16.5 0V6A2.25 2.25 0 016 3.75h3.879a1.5 1.5 0 011.06.44l2.122 2.12a1.5 1.5 0 001.06.44H18A2.25 2.25 0 0120.25 9v.776" />
          </svg>
          <span class="text-sm">Select a resource in the Object Dictionary</span>
        </div>

        <div v-else class="flex-1 flex flex-col overflow-hidden">
          <!-- Top: URL bar + body + button -->
          <div class="p-5 pb-3 space-y-3 shrink-0 border-b border-border">
          <!-- URL bar -->
          <div class="flex items-center gap-2 bg-gray-50 border border-border rounded-lg px-4 py-2.5 font-mono">
            <span
              class="shrink-0 text-[11px] font-bold px-1.5 py-0.5 rounded border"
              :class="methodColor(selectedNode.method)"
            >{{ selectedNode.method ?? '—' }}</span>
            <code class="text-xs text-foreground flex-1 truncate">{{ selectedNode.url }}</code>
            <span
              v-if="subscriptions[selectedNode.key]"
              class="flex items-center gap-1 text-[10px] text-orange-600 font-sans shrink-0"
            >
              <span class="w-1.5 h-1.5 rounded-full bg-orange-500 animate-pulse shrink-0" />
              polling 3 s
            </span>
          </div>

          <!-- ── Fault detail card (infoOnly) ──────────────────────── -->
          <div
            v-if="selectedNode.infoOnly"
            class="rounded-lg border p-4"
            :class="{
              'border-red-200 bg-red-50':    selectedNode.severity === 'high',
              'border-yellow-200 bg-yellow-50': selectedNode.severity === 'medium',
              'border-blue-200 bg-blue-50':  selectedNode.severity === 'low',
            }"
          >
            <div class="flex items-center gap-2 mb-2">
              <span class="w-2.5 h-2.5 rounded-full shrink-0" :class="severityDot(selectedNode.severity)" />
              <span class="font-mono font-bold">{{ selectedNode.label }}</span>
              <span
                class="text-[10px] font-bold px-1.5 py-0.5 rounded uppercase ml-auto"
                :class="selectedNode.badge === 'active' ? 'bg-red-100 text-red-700' : 'bg-yellow-100 text-yellow-700'"
              >{{ selectedNode.badge }}</span>
            </div>
            <p class="text-sm text-gray-700 mb-1 pl-4">{{ selectedNode.description }}</p>
            <p class="text-[11px] text-muted-foreground pl-4 font-mono">severity: {{ selectedNode.severity }}</p>
            <p class="text-[11px] text-muted-foreground pl-4 font-mono mt-0.5">source: /api/v1/ecu/{{ ecuId }}/faults</p>
          </div>

          <!-- ── POST request body ──────────────────────────────────── -->
          <div v-else-if="selectedNode.method === 'POST'">
            <p class="text-[11px] font-semibold text-muted-foreground uppercase tracking-wide mb-1.5">
              Request Body
            </p>
            <textarea
              v-model="requestBody"
              rows="5"
              spellcheck="false"
              class="w-full font-mono text-xs bg-gray-50 border border-border rounded-lg px-3 py-2.5 focus:outline-none focus:ring-1 focus:ring-primary resize-none"
            />
          </div>

          <!-- ── GET / DELETE — no body ─────────────────────────────── -->
          <div v-else>
            <p class="text-[11px] font-semibold text-muted-foreground uppercase tracking-wide mb-1.5">
              Request Body
            </p>
            <div class="bg-gray-50 border border-border rounded-lg px-3 py-2 text-xs font-mono text-muted-foreground">
              (none)
            </div>
          </div>

          <!-- ── Send button ────────────────────────────────────────── -->
          <div v-if="!selectedNode.infoOnly">
            <button
              class="px-4 py-2 text-sm rounded-lg bg-primary text-primary-foreground hover:bg-primary/80 disabled:opacity-50 transition-colors font-mono"
              :disabled="requesting"
              @click="sendRequest"
            >
              {{ requesting ? 'Sending…' : `▶ Send ${selectedNode.method ?? 'GET'}` }}
            </button>
          </div>
          </div><!-- end top section -->

          <!-- Response — fills remaining height, always visible -->
          <div v-if="!selectedNode.infoOnly" class="flex-1 flex flex-col min-h-0 px-5 pt-3 pb-5">
            <div class="flex items-center gap-2 mb-1.5">
              <p class="text-[11px] font-semibold text-muted-foreground uppercase tracking-wide">Response</p>
              <template v-if="response">
                <span
                  class="text-[11px] font-mono font-bold px-1.5 py-0.5 rounded border"
                  :class="response.status >= 200 && response.status < 300
                    ? 'bg-green-50 text-green-700 border-green-200'
                    : 'bg-red-50 text-red-700 border-red-200'"
                >{{ response.status || 'ERR' }}</span>
                <span class="text-[11px] font-mono text-muted-foreground">{{ response.ms }} ms</span>
                <span class="text-[11px] text-muted-foreground ml-auto">{{ response.ts }}</span>
              </template>
            </div>
            <pre
              class="flex-1 min-h-0 overflow-y-auto text-xs bg-gray-50 border border-border rounded-lg px-4 py-3 font-mono whitespace-pre-wrap"
              :class="response ? 'text-foreground' : 'text-muted-foreground/50 italic'"
            >{{ response?.body !== undefined && response.body !== null
              ? JSON.stringify(response.body, null, 2)
              : response?.body === null
                ? '(empty response)'
                : '' }}</pre>
          </div>

        </div>
      </div>
    </div>
    </div><!-- end card -->
  </div>
</template>
