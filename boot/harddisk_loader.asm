; ハードディスク環境用ローダー
; 基本的なローダーを継承してハードディスク固有の機能を追加

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

; カーネル読み込み（ハードディスク固有）
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
    
    ; ハードディスクからカーネル読み込み
    call harddisk_read
    
    mov si, loaded_msg
    call print_string
    ret

; プロテクトモード移行
enter_protected_mode:
    mov si, protected_mode_msg
    call print_string
    
    ; GDT設定
    call setup_gdt
    
    mov si, gdt_ok_msg
    call print_string
    
    ; プロテクトモード有効化
    call enable_protected_mode
    ret

; カーネル実行
execute_kernel:
    mov si, kernel_start_msg
    call print_string
    
    ; カーネル呼び出し
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

; 16進数表示関数
print_hex:
    push ax
    push bx
    push cx
    
    mov cx, 2           ; 2桁表示
    mov bl, al           ; 値を保存
    
hex_loop:
    rol bl, 4            ; 上位4ビットを下位に移動
    mov al, bl
    and al, 0x0F         ; 下位4ビットのみ取得
    
    cmp al, 9
    jle hex_digit
    add al, 7            ; A-Fの文字に変換
    
hex_digit:
    add al, '0'          ; 数字の文字に変換
    mov ah, 0x0E
    int 0x10
    loop hex_loop
    
    pop cx
    pop bx
    pop ax
    ret

; ハードディスク読み込み（環境依存）
harddisk_read:
    ; ディスクリセット
    mov ah, 0x00
    mov dl, 0x80
    int 0x13
    jc disk_error
    
    ; まず1セクタだけ読み込んでテスト
    mov ah, 0x02        ; 読み込み機能
    mov al, 1           ; 1セクタのみ
    mov ch, 0           ; シリンダー
    mov cl, 2           ; セクタ（カーネルは2セクタ目から）
    mov dh, 0           ; ヘッド
    mov dl, 0x80        ; ドライブ（ハードディスク）
    mov bx, 0x1000      ; 読み込み先アドレス
    int 0x13            ; ディスク読み込み
    
    jc disk_error       ; エラーチェック
    
    ; 読み込み成功
    mov si, disk_read_msg
    call print_string
    ret

disk_error:
    ; エラー詳細の表示
    mov si, disk_error_msg
    call print_string
    
    ; エラーコードの表示
    mov si, error_code_msg
    call print_string
    
    ; エラーコードを16進数で表示
    mov al, ah
    call print_hex
    
    jmp $

; GDT設定（環境依存）
setup_gdt:
    ; GDTテーブルの設定
    lgdt [gdt_descriptor]
    ret

; プロテクトモード有効化（環境依存）
enable_protected_mode:
    ; A20ライン有効化
    call enable_a20
    
    ; プロテクトモードフラグ設定
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; ジャンプしてプロテクトモードに移行
    jmp 0x08:protected_mode_start

; A20ライン有効化
enable_a20:
    ; A20ライン有効化処理
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

; プロテクトモード開始
[BITS 32]
protected_mode_start:
    ; セグメントレジスタの設定
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

; カーネル呼び出し（環境依存）
call_kernel:
    ; カーネルエントリーポイント呼び出し
    call 0x100000       ; カーネルのアドレス
    ret

; GDTテーブル（簡略版）
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
boot_msg db 'Retron OS Harddisk Loader v1.0', 13, 10, 0
memory_msg db 'Memory: 512MB', 13, 10, 0
memory_ok_msg db 'Memory: OK', 13, 10, 0
loading_msg db 'Loading from Harddisk...', 13, 10, 0
kernel_info_msg db 'Kernel: Rust', 13, 10, 0
kernel_size_msg db 'Size: 1840 bytes', 13, 10, 0
kernel_address_msg db 'Addr: 0x100000', 13, 10, 0
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
