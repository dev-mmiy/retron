/* カーネルリンカースクリプト */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(pvh_entry)

/* Program Headers - PT_NOTE must come first for QEMU PVH loader */
PHDRS
{
    note PT_NOTE FLAGS(4);   /* PVH boot note - MUST BE FIRST */
    text PT_LOAD FLAGS(5);   /* Read + Execute */
    rodata PT_LOAD FLAGS(4); /* Read only */
    data PT_LOAD FLAGS(6);   /* Read + Write */
}

SECTIONS
{
    /* カーネルは1MBから開始 */
    . = 1M;

    /* PVH ELF Note（64-bit QEMU起動用）- 最初に配置が重要 */
    .note.PVH : {
        KEEP(*(.note.PVH))
    } :note

    /* テキストセクション - エントリーポイント */
    .text ALIGN(4K) : {
        KEEP(*(.text.pvh_entry))
        KEEP(*(.text.kernel_main))
        *(.text .text.*)
    } :text

    /* 読み取り専用データ */
    .rodata ALIGN(4K) : {
        *(.rodata .rodata.*)
    } :rodata

    /* データセクション */
    .data ALIGN(4K) : {
        *(.data .data.*)
    } :data

    /* BSSセクション */
    .bss ALIGN(4K) : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /* Boot stack - 16KB aligned, placed at fixed address for assembly reference */
    . = 0x108000;
    .bss.boot_stack . : {
        boot_stack_start = .;
        *(.bss.boot_stack)
        boot_stack_end = .;
    } :data

    /* デバッグ情報とダイナミックセクションは破棄 */
    /DISCARD/ : {
        *(.eh_frame)
        *(.note.GNU-stack)
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

