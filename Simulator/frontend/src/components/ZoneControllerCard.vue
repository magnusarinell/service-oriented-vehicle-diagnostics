<template>
  <div class="rounded-xl border border-border bg-card shadow-sm overflow-hidden">
    <div class="px-4 py-3 border-b border-border bg-muted/30 flex items-center gap-2">
      <span class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest">Zone Controller</span>
      <span class="ml-auto text-[10px] px-1.5 py-0.5 rounded bg-muted text-muted-foreground border border-border font-mono">
        docker only
      </span>
    </div>

    <div class="p-4 flex items-end gap-4">
      <!-- PCB-box -->
      <div class="rounded-lg border-2 border-slate-400 bg-slate-700 p-3 flex-1">
        <div class="relative rounded border border-slate-500 bg-slate-800 px-3 py-2 text-center mb-3">
          <div class="absolute -top-px left-1/2 -translate-x-1/2 w-4 h-1.5 bg-slate-800 border border-slate-500 rounded-b-sm" />
          <p class="text-blue-300 font-mono text-[9px] tracking-widest uppercase">vsomeip</p>
          <p class="text-white font-bold text-xs mt-0.5">Zone Controller</p>
        </div>
        <div class="flex items-center justify-between px-1">
          <div class="flex flex-col gap-0.5">
            <div v-for="i in 3" :key="i" class="w-5 h-1.5 bg-slate-600 border border-slate-500 rounded-sm" />
          </div>
          <div class="flex gap-0.5">
            <div v-for="i in 3" :key="i" class="w-1.5 h-3 bg-slate-500 rounded-sm border border-slate-400" />
          </div>
          <div class="w-4 h-4 rounded-full bg-slate-600 border border-slate-500" />
        </div>
      </div>

      <!-- Connector pins on right side -->
      <div class="flex flex-col gap-0.5">
        <div v-for="n in 10" :key="n" class="h-2 w-4 bg-yellow-400 border border-yellow-600 rounded-r-sm" />
      </div>
    </div>

    <div class="px-4 pb-3">
      <p class="text-[10px] text-muted-foreground font-mono">SOME/IP · CAN · LIN</p>
      <p class="text-[10px] text-muted-foreground/60">172.20.0.10 (docker network)</p>
    </div>
  </div>
</template>
