<script setup lang="ts">
import { useGatewayEvents } from '@/composables/useGatewayEvents'

const gw = useGatewayEvents()
</script>

<template>
  <div class="border-t border-border bg-card overflow-hidden">
    <div class="px-4 py-3 flex items-center gap-3">
      <!-- Pulse indicator -->
      <div class="relative shrink-0">
        <div
          class="w-3 h-3 rounded-full transition-colors duration-100"
          :class="gw.pulsing.value ? 'bg-green-400' : 'bg-gray-300'"
        />
        <div
          v-if="gw.pulsing.value"
          class="absolute inset-0 rounded-full bg-green-400 animate-ping"
        />
      </div>

      <!-- Labels -->
      <div class="flex-1 min-w-0">
        <p class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">SomeIp Gateway</p>
        <p class="text-[10px] font-mono text-muted-foreground mt-0.5">D-Bus ↔ SOME/IP bridge</p>
      </div>

      <!-- Counters -->
      <div class="flex items-center gap-3 shrink-0">
        <div class="text-center">
          <p class="text-[15px] font-bold font-mono tabular-nums leading-none">{{ gw.callCount.value }}</p>
          <p class="text-[9px] text-muted-foreground uppercase tracking-wide">calls</p>
        </div>
        <div class="text-center">
          <p class="text-[15px] font-bold font-mono tabular-nums leading-none">{{ gw.replyCount.value }}</p>
          <p class="text-[9px] text-muted-foreground uppercase tracking-wide">replies</p>
        </div>
      </div>
    </div>

    <!-- Recent events (last 6) -->
    <div
      v-if="gw.events.value.length > 0"
      class="border-t border-border px-4 py-2 space-y-0.5"
    >
      <div
        v-for="(ev, i) in gw.events.value.slice(0, 6)"
        :key="i"
        class="flex items-center gap-2 text-[10px] font-mono py-0.5"
        :class="i === 0 ? 'text-foreground' : 'text-muted-foreground'"
      >
        <span
          class="shrink-0 font-bold"
          :class="ev.direction === '→' ? 'text-blue-500' : 'text-green-500'"
        >{{ ev.direction }}</span>
        <span class="font-semibold">{{ ev.method }}</span>
        <span class="opacity-60 truncate">{{ ev.detail }}</span>
        <span class="ml-auto shrink-0 opacity-40">{{ ev.ts }}</span>
      </div>
    </div>
    <div v-else class="border-t border-border px-4 py-2 text-[10px] text-muted-foreground italic">
      Waiting for activity…
    </div>
  </div>
</template>
