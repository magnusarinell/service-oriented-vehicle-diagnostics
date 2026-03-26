<script setup lang="ts">
import { useRenode } from '@/composables/useRenode'
import EcmBoard from './EcmBoard.vue'

const renode = useRenode()
</script>

<template>
  <EcmBoard
    :fault-bits="renode.faultBits.value"
    :ready="renode.ready.value"
    :connected="renode.connected.value"
    :cable-cut="renode.cableCut.value"
    @cut-cable="renode.cutCable"
    @restore-cable="renode.restoreCable"
    @inject-fault="renode.injectFault"
    @clear-faults="renode.clearFaults"
  />
</template>
