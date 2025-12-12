/* カーネルリンカースクリプト - Multiboot2対応 */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

/* Program Headers */
PHDRS
{
    boot PT_LOAD FLAGS(5);   /* Boot code (Read + Execute) */
    text PT_LOAD FLAGS(5);   /* Read + Execute */
    rodata PT_LOAD FLAGS(4); /* Read only */
    data PT_LOAD FLAGS(6);   /* Read + Write */
}

SECTIONS
{
    /* カーネルは1MBから開始（Multiboot2標準） */
    . = 1M;

    /* Boot segment: Multiboot header + code */
    .boot : {
        KEEP(*(.multiboot))
        *(.text)
        *(.text.*)
    } :boot

    /* Align for next segment */
    . = ALIGN(4K);

    /* 読み取り専用データ */
    .rodata ALIGN(4K) : {
        *(.rodata .rodata.*)
    } :rodata

    /* データセクション */
    .data ALIGN(4K) : {
        *(.data .data.*)
    } :data

    /* BSSセクション（NOBITSだが、同じdata segmentに含める） */
    .bss (NOLOAD) : {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    } :data

    /* デバッグ情報とダイナミックセクションは破棄 */
    /DISCARD/ : {
        *(.eh_frame)
        *(.note.GNU-stack)
        *(.note.PVH)  /* PVH noteは不要 */
        *(.dynsym)
        *(.dynstr)
        *(.dynamic)
        *(.gnu.hash)
        *(.hash)
        *(.gnu.version*)
        *(.rela.*)
        *(.rel.*)
    }
}
