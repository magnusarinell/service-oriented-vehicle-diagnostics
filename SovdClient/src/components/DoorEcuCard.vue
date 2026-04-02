<template>
  <div class="flex gap-4 items-start">
    <!-- Door ECU black box -->
    <div class="flex-1">
      <p class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest mb-2">Door ECU</p>
      <div class="rounded-lg border border-gray-700 bg-gray-900 p-4">
        <div class="flex items-center justify-between mb-2">
          <p class="text-green-400 font-mono text-[10px] tracking-widest uppercase">NXP FRDM-K64F</p>
          <div class="flex items-center gap-1.5">
            <span class="w-2 h-2 rounded-full bg-gray-600" />
            <span class="text-[10px] font-mono text-gray-600">NOT CONNECTED</span>
          </div>
        </div>
        <p class="text-[10px] text-gray-500 font-mono">CAN / LIN · SOME/IP</p>
      </div>
    </div>

    <!-- Fault Injection (disabled) -->
    <div class="w-44 shrink-0">
      <p class="text-[11px] font-bold text-muted-foreground uppercase tracking-widest mb-2">Fault Injection</p>
      <div class="space-y-1.5">
        <div
          v-for="code in ['B1001', 'B1002', 'B1003']"
          :key="code"
          class="w-full flex items-center gap-2 px-2.5 py-1.5 rounded border text-[11px] bg-white border-gray-200 text-gray-400"
        >
          <span class="w-1.5 h-1.5 rounded-full shrink-0 bg-gray-300" />
          <span class="font-mono font-bold">{{ code }}</span>
        </div>
      </div>
      <div class="mt-2 w-full px-2 py-1 text-[10px] rounded border border-gray-200 text-gray-400 text-center">
        Clear faults
      </div>
    </div>
  </div>
</template>
