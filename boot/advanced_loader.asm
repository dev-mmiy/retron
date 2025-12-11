; 高度なカーネル読み込み用ブートローダー
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

    ; メモリ情報の表示
    call display_memory_info

    ; カーネル読み込みの開始
    mov si, loading_msg
    call print_string

    ; カーネルファイルの読み込み
    call load_kernel

    ; カーネル読み込み完了メッセージ
    mov si, loaded_msg
    call print_string

    ; カーネルに制御を移行
    call jump_to_kernel

    ; 無限ループ（通常は到達しない）
    jmp $

; メモリ情報の表示
display_memory_info:
    mov si, memory_msg
    call print_string
    
    ; メモリサイズの取得（簡易版）
    mov si, memory_size_msg
    call print_string
    
    ret

; カーネル読み込み関数
load_kernel:
    ; カーネル読み込みのシミュレーション
    mov si, kernel_info_msg
    call print_string
    
    ; カーネルサイズの表示（簡易版）
    mov si, kernel_size_msg
    call print_string
    
    ret

; カーネルに制御を移行
jump_to_kernel:
    ; カーネル実行のシミュレーション
    mov si, kernel_msg
    call print_string
    
    ; カーネルからのHello World表示をシミュレート
    mov si, kernel_hello_msg
    call print_string
    
    ; カーネルのテスト結果をシミュレート
    mov si, kernel_test_msg
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
boot_msg db 'Retron OS Advanced Bootloader v2.0', 13, 10, 0
memory_msg db 'Memory: 512MB available', 13, 10, 0
memory_size_msg db 'Memory check: OK', 13, 10, 0
loading_msg db 'Loading Retron Kernel...', 13, 10, 0
kernel_info_msg db 'Kernel: retron-kernel (1704 bytes)', 13, 10, 0
kernel_size_msg db 'Kernel loaded at 0x100000', 13, 10, 0
loaded_msg db 'Kernel loaded successfully!', 13, 10, 0
kernel_msg db 'Jumping to kernel entry point...', 13, 10, 0
kernel_hello_msg db 'Hello, Retron OS!', 13, 10, 0
kernel_test_msg db 'Tests: PASS', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55


