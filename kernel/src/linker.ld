/* カーネルリンカースクリプト */
ENTRY(kernel_main)

SECTIONS
{
    /* カーネルは1MBから開始 */
    . = 1M;
    
    /* Multibootヘッダーを最初に配置 */
    .multiboot : {
        *(.multiboot)
    }
    
    /* テキストセクション */
    .text : {
        *(.text)
    }
    
    /* 読み取り専用データ */
    .rodata : {
        *(.rodata)
    }
    
    /* データセクション */
    .data : {
        *(.data)
    }
    
    /* BSSセクション */
    .bss : {
        *(.bss)
    }
}

