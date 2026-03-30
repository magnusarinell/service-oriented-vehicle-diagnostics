/* startup_mk64f12.S -- Minimal Cortex-M4 startup for NXP MK64F12
 * Provides the interrupt vector table and Reset_Handler that initialises
 * .data and .bss then calls main(). */

    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

/* ── Vector table ────────────────────────────────────────────────────────── */
    .section .isr_vector, "a", %progbits
    .type    __isr_vector, %object
    .globl   __isr_vector
__isr_vector:
    .long   __StackTop          /* 0x00: initial SP                   */
    .long   Reset_Handler       /* 0x04: reset                        */
    .long   NMI_Handler         /* 0x08: NMI                          */
    .long   HardFault_Handler   /* 0x0C: hard fault                   */
    .long   MemManage_Handler   /* 0x10: MemManage fault              */
    .long   BusFault_Handler    /* 0x14: bus fault                    */
    .long   UsageFault_Handler  /* 0x18: usage fault                  */
    .long   0                   /* 0x1C: reserved                     */
    .long   0                   /* 0x20: reserved                     */
    .long   0                   /* 0x24: reserved                     */
    .long   0                   /* 0x28: reserved                     */
    .long   SVC_Handler         /* 0x2C: SVCall                       */
    .long   DebugMon_Handler    /* 0x30: Debug monitor                */
    .long   0                   /* 0x34: reserved                     */
    .long   PendSV_Handler      /* 0x38: PendSV                       */
    .long   SysTick_Handler     /* 0x3C: SysTick                      */
    /* External interrupts (IRQ0..IRQ239) -- all default for now      */
    .fill   240, 4, 0
    .size   __isr_vector, . - __isr_vector

/* ── Reset handler ───────────────────────────────────────────────────────── */
    .thumb_func
    .globl  Reset_Handler
    .type   Reset_Handler, %function
Reset_Handler:
    /* Copy initialised data from flash (LMA) to SRAM (VMA) */
    ldr     r0, =__data_start__
    ldr     r1, =__data_end__
    ldr     r2, =__etext
copy_data:
    cmp     r0, r1
    bge     zero_bss
    ldr     r3, [r2], #4
    str     r3, [r0], #4
    b       copy_data

    /* Zero .bss */
zero_bss:
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    mov     r2, #0
zero_loop:
    cmp     r0, r1
    bge     call_main
    str     r2, [r0], #4
    b       zero_loop

call_main:
    bl      main
    /* main() should never return, but just in case */
spin:
    b       spin

    .size   Reset_Handler, . - Reset_Handler

/* ── Default / weak handlers ─────────────────────────────────────────────── */
    .thumb_func
    .globl  Default_Handler
    .type   Default_Handler, %function
Default_Handler:
    b       Default_Handler
    .size   Default_Handler, . - Default_Handler

    .macro  WEAK_ALIAS name
    .weak   \name
    .thumb_set \name, Default_Handler
    .endm

    WEAK_ALIAS NMI_Handler
    WEAK_ALIAS HardFault_Handler
    WEAK_ALIAS MemManage_Handler
    WEAK_ALIAS BusFault_Handler
    WEAK_ALIAS UsageFault_Handler
    WEAK_ALIAS SVC_Handler
    WEAK_ALIAS DebugMon_Handler
    WEAK_ALIAS PendSV_Handler
    WEAK_ALIAS SysTick_Handler
