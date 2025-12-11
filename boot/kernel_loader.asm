; カーネル読み込み用ブートローダー
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

    ; ブートローダーメッセージの表示
    mov si, boot_msg
    call print_string

    ; カーネル読み込みの開始
    mov si, loading_msg
    call print_string

    ; カーネルファイルの読み込み（簡易版）
    ; 実際の実装では、ファイルシステムから読み込む
    call load_kernel

    ; カーネル読み込み完了メッセージ
    mov si, loaded_msg
    call print_string

    ; カーネルに制御を移行
    call jump_to_kernel

    ; 無限ループ（通常は到達しない）
    jmp $

; カーネル読み込み関数（簡易版）
load_kernel:
    ; ここでは実際のファイル読み込みは行わない
    ; 代わりに、カーネルが既にメモリに配置されていると仮定
    ret

; カーネルに制御を移行
jump_to_kernel:
    ; カーネルのエントリーポイント（仮想アドレス）
    ; 実際の実装では、カーネルの実際のアドレスを使用
    mov si, kernel_msg
    call print_string
    
    ; カーネル実行のシミュレーション
    mov si, running_msg
    call print_string
    
    ret

; 文字列表示関数
print_string:
    lodsb
    or al, al
    jz done
    mov ah, 0x0E
    int 0x10
    jmp print_string
done:
    ret

; メッセージ定義
boot_msg db 'Retron OS Bootloader v1.0', 13, 10, 0
loading_msg db 'Loading kernel...', 13, 10, 0
loaded_msg db 'Kernel loaded successfully!', 13, 10, 0
kernel_msg db 'Jumping to kernel...', 13, 10, 0
running_msg db 'Kernel is running!', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55


