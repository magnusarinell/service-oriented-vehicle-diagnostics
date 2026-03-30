/* mk64f12.ld -- Linker script for NXP MK64F12 (FRDM-K64F)
 *
 * Memory map:
 *   Flash : 0x00000000, 1 MB
 *   SRAM_L: 0x1FFF0000, 128 KB   (lower SRAM, unified alias below)
 *   SRAM_U: 0x20000000, 192 KB   (upper SRAM, mapped contiguously)
 *
 * We use a single flat SRAM region starting at 0x20000000 (256 KB) which
 * covers the upper portion.  SRAM_L is not used for simplicity.
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 1024K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
}

ENTRY(Reset_Handler)

SECTIONS
{
    /* ── Code + read-only data ───────────────────────────────────────────── */
    .text :
    {
        KEEP(*(.isr_vector))    /* vector table MUST be first */
        *(.text*)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
        KEEP(*(.ARM.exidx*))
        . = ALIGN(4);
        __etext = .;            /* symbol used by startup to copy .data */
    } > FLASH

    /* ── Initialised data (copied from flash to SRAM at startup) ─────────── */
    .data : AT(__etext)
    {
        . = ALIGN(4);
        __data_start__ = .;
        *(.data*)
        . = ALIGN(4);
        __data_end__ = .;
    } > SRAM

    /* ── Uninitialised data (zeroed at startup) ─────────────────────────── */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
        PROVIDE(end = .);   /* newlib _sbrk heap start */
        PROVIDE(_end = .);
    } > SRAM

    /* ── Stack at top of SRAM ────────────────────────────────────────────── */
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        . += 0x2000;            /* 8 KB stack */
        . = ALIGN(8);
        __StackTop = .;
    } > SRAM

    /* ── Discard C++ exception tables (not used in bare-metal) ─────────── */
    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.eh_frame*)
    }
}
