; 最小限のハードディスク用ローダー
; ディスク読み込みエラーの詳細表示機能付き

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

    ; カーネル読み込み
    mov si, loading_msg
    call print_string
    
    ; ハードディスクからカーネル読み込み
    call harddisk_read
    
    mov si, loaded_msg
    call print_string

    ; プロテクトモード移行
    mov si, protected_mode_msg
    call print_string
    
    ; GDT設定
    call setup_gdt
    
    mov si, gdt_ok_msg
    call print_string
    
    ; プロテクトモード有効化
    call enable_protected_mode

    ; カーネル実行
    mov si, kernel_start_msg
    call print_string
    
    ; カーネル呼び出し
    call call_kernel

    ; 無限ループ（通常は到達しない）
    jmp $

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

; 16進数表示関数
print_hex:
    push ax
    push bx
    push cx
    
    mov cx, 2
    mov bl, al
    
hex_loop:
    rol bl, 4
    mov al, bl
    and al, 0x0F
    
    cmp al, 9
    jle hex_digit
    add al, 7
    
hex_digit:
    add al, '0'
    mov ah, 0x0E
    int 0x10
    loop hex_loop
    
    pop cx
    pop bx
    pop ax
    ret

; ハードディスク読み込み
harddisk_read:
    ; ディスクリセット
    mov ah, 0x00
    mov dl, 0x80
    int 0x13
    jc disk_error
    
    ; 1セクタ読み込み
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x1000
    int 0x13
    
    jc disk_error
    
    mov si, disk_read_msg
    call print_string
    ret

disk_error:
    mov si, disk_error_msg
    call print_string
    mov si, error_code_msg
    call print_string
    mov al, ah
    call print_hex
    jmp $

; GDT設定
setup_gdt:
    lgdt [gdt_descriptor]
    ret

; プロテクトモード有効化
enable_protected_mode:
    in al, 0x92
    or al, 2
    out 0x92, al
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode_start

; プロテクトモード開始
[BITS 32]
protected_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

; カーネル呼び出し
call_kernel:
    jmp 0x1001d1

; GDTテーブル
gdt_start:
    dd 0, 0
gdt_code:
    dw 0xFFFF, 0, 0x9A00, 0x00CF
gdt_data:
    dw 0xFFFF, 0, 0x9200, 0x00CF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; メッセージ定義
boot_msg db 'Retron OS Minimal Loader v1.0', 13, 10, 0
loading_msg db 'Loading from Harddisk...', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
disk_error_msg db 'Disk read: ERROR', 13, 10, 0
error_code_msg db 'Error code: 0x', 0
loaded_msg db 'Loaded: OK', 13, 10, 0
protected_mode_msg db 'Protected mode...', 13, 10, 0
gdt_ok_msg db 'GDT: OK', 13, 10, 0
kernel_start_msg db 'Jumping to kernel...', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55
