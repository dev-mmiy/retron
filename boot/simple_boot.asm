; シンプルなブートローダー
; 16-bit リアルモードで動作

[BITS 16]
[ORG 0x7C00]

start:
    ; セグメントレジスタの初期化
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Hello Worldメッセージの表示
    mov si, msg
    call print_string

    ; カーネル情報の表示
    mov si, kernel_msg
    call print_string

    ; 無限ループ
    jmp $

print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_string
done:
    ret

msg db 'Hello, Retron OS!', 13, 10, 0
kernel_msg db 'Kernel loaded successfully!', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55


