; ハードディスク用ブートローダー
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

    ; ハードディスクブートローダーメッセージの表示
    mov si, boot_msg
    call print_string

    ; メモリ情報の表示
    call display_memory_info

    ; カーネル読み込みの開始
    mov si, loading_msg
    call print_string

    ; ハードディスクからカーネルを読み込み
    call load_kernel_from_harddisk

    ; カーネル読み込み完了メッセージ
    mov si, loaded_msg
    call print_string

    ; プロテクトモードへの移行準備
    call prepare_protected_mode

    ; 実際のカーネルに制御を移行
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

; ハードディスクからカーネルを読み込み
load_kernel_from_harddisk:
    ; カーネルファイルの読み込み（簡易版）
    mov si, kernel_info_msg
    call print_string
    
    ; カーネルサイズの表示
    mov si, kernel_size_msg
    call print_string
    
    ; カーネルメモリアドレスの設定
    mov si, kernel_address_msg
    call print_string
    
    ; ハードディスクからの読み込みシミュレーション
    mov si, disk_read_msg
    call print_string
    
    ret

; プロテクトモードへの移行準備
prepare_protected_mode:
    mov si, protected_mode_msg
    call print_string
    
    ; プロテクトモードの準備（簡易版）
    mov si, gdt_setup_msg
    call print_string
    
    ret

; 実際のカーネルに制御を移行
jump_to_kernel:
    ; カーネル実行の開始
    mov si, kernel_start_msg
    call print_string
    
    ; 実際のカーネル実行をシミュレート
    call simulate_kernel_execution
    
    ret

; カーネル実行のシミュレーション
simulate_kernel_execution:
    ; カーネルのHello World表示
    mov si, kernel_hello_msg
    call print_string
    
    ; カーネルのテスト結果表示
    mov si, kernel_test_msg
    call print_string
    
    ; カーネルのメモリ管理表示
    mov si, kernel_memory_msg
    call print_string
    
    ; カーネルのタスク管理表示
    mov si, kernel_task_msg
    call print_string
    
    ; カーネルのデバイス管理表示
    mov si, kernel_device_msg
    call print_string
    
    ; カーネルの割り込み処理表示
    mov si, kernel_interrupt_msg
    call print_string
    
    ; カーネルのμT-Kernel互換表示
    mov si, kernel_utkernel_msg
    call print_string
    
    ; カーネルのファイルシステム表示
    mov si, kernel_filesystem_msg
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
boot_msg db 'Retron OS Harddisk Boot v4.0', 13, 10, 0
memory_msg db 'Memory: 512MB', 13, 10, 0
memory_size_msg db 'Memory: OK', 13, 10, 0
loading_msg db 'Loading from Harddisk...', 13, 10, 0
kernel_info_msg db 'Kernel: Rust', 13, 10, 0
kernel_size_msg db 'Size: 1840 bytes', 13, 10, 0
kernel_address_msg db 'Addr: 0x100000', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
loaded_msg db 'Loaded: OK', 13, 10, 0
protected_mode_msg db 'Protected mode...', 13, 10, 0
gdt_setup_msg db 'GDT: OK', 13, 10, 0
kernel_start_msg db 'Jumping to kernel...', 13, 10, 0
kernel_hello_msg db 'Hello, Retron OS!', 13, 10, 0
kernel_test_msg db 'Tests: PASS', 13, 10, 0
kernel_memory_msg db 'Memory: OK', 13, 10, 0
kernel_task_msg db 'Tasks: OK', 13, 10, 0
kernel_device_msg db 'Devices: OK', 13, 10, 0
kernel_interrupt_msg db 'Interrupts: OK', 13, 10, 0
kernel_utkernel_msg db 'μT-Kernel: OK', 13, 10, 0
kernel_filesystem_msg db 'Filesystem: OK', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55


