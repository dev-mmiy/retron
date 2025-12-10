/* カーネルリンカースクリプト */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(kernel_main)

/* Program Headers */
PHDRS
{
    text PT_LOAD FLAGS(5);   /* Read + Execute */
    rodata PT_LOAD FLAGS(4); /* Read only */
    data PT_LOAD FLAGS(6);   /* Read + Write */
    note PT_NOTE FLAGS(4);   /* PVH boot note */
}

SECTIONS
{
    /* カーネルは1MBから開始 */
    . = 1M;

    /* Multibootヘッダーを最初に配置（最初の8KB以内に必須） */
    .multiboot ALIGN(4K) : {
        KEEP(*(.multiboot))
    }

    /* PVH ELF Note（64-bit QEMU起動用） */
    .note.PVH : {
        KEEP(*(.note.PVH))
    } :note

    /* テキストセクション */
    .text ALIGN(4K) : {
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

    /* デバッグ情報は破棄 */
    /DISCARD/ : {
        *(.eh_frame)
        *(.note.GNU-stack)
    }
}

