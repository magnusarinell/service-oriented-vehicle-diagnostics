<script setup lang="ts">
</script>

<template>
  <section class="rounded-xl border border-border bg-card shadow-sm overflow-hidden">
    <!-- Header -->
    <div class="px-6 py-3 border-b border-border bg-muted/30">
      <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">Vehicle Network Topology</span>
    </div>

    <!-- Topology canvas -->
    <div class="p-6 overflow-x-auto">
      <div class="flex items-center gap-8 min-w-max">

        <!-- ── HPC box ───────────────────────────────────────────────── -->
        <div class="shrink-0 rounded-lg border-2 border-blue-700 bg-blue-950 p-4 w-44 text-center">
          <p class="text-blue-400 font-mono text-[10px] tracking-widest uppercase mb-1">HPC</p>
          <p class="text-white font-bold text-sm">SOVD Server</p>
          <p class="text-blue-300 font-mono text-[10px] mt-1">REST · D-Bus</p>
        </div>

        <!-- ── SOME/IP bus line ─────────────────────────────────────── -->
        <div class="flex flex-col items-center gap-1 shrink-0">
          <span class="text-[10px] text-gray-400 font-mono uppercase tracking-wider">SOME/IP</span>
          <div class="flex items-center gap-1">
            <div class="w-12 h-0.5 bg-gray-400" />
            <div class="w-2 h-2 rounded-full bg-gray-400 shrink-0" />
            <div class="w-12 h-0.5 bg-gray-400" />
          </div>
        </div>

        <!-- ── Zone Controller box ───────────────────────────────────── -->
        <div class="shrink-0 rounded-lg border-2 border-slate-500 bg-slate-800 p-4 w-44 text-center">
          <p class="text-blue-400 font-mono text-[10px] tracking-widest uppercase mb-1">vsomeip3</p>
          <p class="text-white font-bold text-sm">Zone Controller</p>
          <p class="text-slate-400 font-mono text-[10px] mt-1">0x1111 · 0x0001</p>
        </div>

      </div>
    </div>
  </section>
</template>
