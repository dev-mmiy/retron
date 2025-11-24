; Multibootヘッダー
; カーネルをQEMUで実行するためのブートローダー

[BITS 32]

; Multibootヘッダー
section .multiboot
align 4
    dd 0x1BADB002              ; magic
    dd 0x00000000              ; flags
    dd -(0x1BADB002 + 0x00000000) ; checksum

; スタックの設定
section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

; カーネルエントリーポイント
section .text
global _start
_start:
    ; スタックポインタの設定
    mov esp, stack_top
    
    ; カーネルのメイン関数を呼び出し
    extern kernel_main
    call kernel_main
    
    ; 無限ループ
    cli
    hlt
    jmp $


