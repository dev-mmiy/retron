; 基本的なカーネルローダー
; 環境に依存しない基本機能のみを提供

[BITS 16]
[ORG 0x7C00]

start:
    ; セグメントレジスタの初期化
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; ブートメッセージの表示
    mov si, boot_msg
    call print_string

    ; メモリ情報の表示
    call display_memory_info

    ; カーネル読み込み
    call load_kernel

    ; プロテクトモードへの移行
    call enter_protected_mode

    ; カーネル実行
    call execute_kernel

    ; 無限ループ（通常は到達しない）
    jmp $

; メモリ情報表示
display_memory_info:
    mov si, memory_msg
    call print_string
    mov si, memory_ok_msg
    call print_string
    ret

; カーネル読み込み
load_kernel:
    mov si, loading_msg
    call print_string
    
    ; カーネル情報表示
    mov si, kernel_info_msg
    call print_string
    
    mov si, kernel_size_msg
    call print_string
    
    mov si, kernel_address_msg
    call print_string
    
    ; ディスク読み込み（環境依存）
    call disk_read
    
    mov si, loaded_msg
    call print_string
    ret

; プロテクトモード移行
enter_protected_mode:
    mov si, protected_mode_msg
    call print_string
    
    ; GDT設定（環境依存）
    call setup_gdt
    
    mov si, gdt_ok_msg
    call print_string
    
    ; プロテクトモード有効化（環境依存）
    call enable_protected_mode
    ret

; カーネル実行
execute_kernel:
    mov si, kernel_start_msg
    call print_string
    
    ; カーネル呼び出し（環境依存）
    call call_kernel
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

; 環境依存関数（後で実装）
disk_read:
    ; 環境依存の実装
    mov si, disk_read_msg
    call print_string
    ret

setup_gdt:
    ; 環境依存の実装
    ret

enable_protected_mode:
    ; 環境依存の実装
    ret

call_kernel:
    ; 環境依存の実装
    ; 実際のカーネル呼び出しはここで行う
    ret

; メッセージ定義
boot_msg db 'Retron OS Base Loader v1.0', 13, 10, 0
memory_msg db 'Memory: 512MB', 13, 10, 0
memory_ok_msg db 'Memory: OK', 13, 10, 0
loading_msg db 'Loading kernel...', 13, 10, 0
kernel_info_msg db 'Kernel: Rust', 13, 10, 0
kernel_size_msg db 'Size: 1840 bytes', 13, 10, 0
kernel_address_msg db 'Addr: 0x100000', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
loaded_msg db 'Loaded: OK', 13, 10, 0
protected_mode_msg db 'Protected mode...', 13, 10, 0
gdt_ok_msg db 'GDT: OK', 13, 10, 0
kernel_start_msg db 'Jumping to kernel...', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55
