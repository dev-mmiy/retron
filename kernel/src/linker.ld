/* カーネルリンカースクリプト */
ENTRY(kernel_main)

SECTIONS
{
    /* カーネルは1MBから開始 */
    . = 1M;

    /* Multibootヘッダーを最初に配置（最初の8KB以内に必須） */
    .multiboot ALIGN(4K) : {
        KEEP(*(.multiboot))
    }

    /* テキストセクション */
    .text ALIGN(4K) : {
        KEEP(*(.text.kernel_main))
        *(.text .text.*)
    }
    
    /* 読み取り専用データ */
    .rodata ALIGN(4K) : {
        *(.rodata .rodata.*)
    }

    /* データセクション */
    .data ALIGN(4K) : {
        *(.data .data.*)
    }

    /* BSSセクション */
    .bss ALIGN(4K) : {
        *(.bss .bss.*)
        *(COMMON)
    }

    /* デバッグ情報は破棄 */
    /DISCARD/ : {
        *(.eh_frame)
        *(.note.GNU-stack)
    }
}

