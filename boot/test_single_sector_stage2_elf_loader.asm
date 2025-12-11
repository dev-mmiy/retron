[BITS 16]
[ORG 0x2000]

start:
    ; セグメントレジスタの初期化
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x2000

    ; 第2段階ローダーメッセージの表示
    mov si, boot_msg
    call print_string

    ; カーネル読み込み
    call load_kernel

    ; GDT設定
    call setup_gdt

    ; プロテクトモード有効化
    call enable_protected_mode

    ; カーネル実行
    call execute_kernel

    jmp $ ; 無限ループ

; 文字列表示関数
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
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

; 32ビット16進数表示関数
print_hex32:
    push eax
    push bx
    push cx
    
    mov cx, 8           ; 8桁表示
    mov ebx, eax        ; 値を保存
    
hex32_loop:
    rol ebx, 4          ; 上位4ビットを下位に移動
    mov eax, ebx
    and eax, 0x0F       ; 下位4ビットのみ取得
    
    cmp eax, 9
    jle hex32_digit
    add eax, 7          ; A-Fの文字に変換
    
hex32_digit:
    add eax, '0'        ; 数字の文字に変換
    mov ah, 0x0E
    int 0x10
    loop hex32_loop
    
    pop cx
    pop bx
    pop eax
    ret

; カーネル読み込み（セクタ数1でテスト）
load_kernel:
    mov si, loading_msg
    call print_string
    
    ; デバッグ: 読み込み前の状態表示
    mov si, debug_before_load_msg
    call print_string
    
    ; デバッグ: 読み込みパラメータの表示
    mov si, debug_load_params_msg
    call print_string
    
    ; デバッグ: セクタ番号の表示
    mov si, debug_sector_msg
    call print_string
    mov al, 4            ; セクタ番号（4）
    call print_hex
    mov si, newline_msg
    call print_string
    
    ; デバッグ: セクタ数の表示
    mov si, debug_sector_count_msg
    call print_string
    mov al, 1            ; セクタ数（1に減らす）
    call print_hex
    mov si, newline_msg
    call print_string
    
    ; デバッグ: 読み込み先アドレスの表示
    mov si, debug_load_addr_msg
    call print_string
    mov eax, 0x1000      ; 読み込み先アドレス
    call print_hex32
    mov si, newline_msg
    call print_string
    
    ; デバッグ: int 0x13の前の状態
    mov si, debug_before_int13_msg
    call print_string
    
    ; ELFカーネルを読み込み（セクタ数1でテスト）
    mov ah, 0x02        ; 読み込み機能
    mov al, 1           ; 1セクタ読み込み（テスト用）
    mov ch, 0           ; シリンダー
    mov cl, 4           ; セクタ（カーネルは4セクタ目から）
    mov dh, 0           ; ヘッド
    mov dl, 0x80        ; ドライブ（ハードディスク）
    mov bx, 0x1000      ; 読み込み先アドレス
    int 0x13            ; ディスク読み込み
    
    ; デバッグ: int 0x13の後の状態
    mov si, debug_after_int13_msg
    call print_string
    
    ; エラーチェック
    jc disk_error       ; エラーチェック
    
    ; デバッグ: 読み込み成功の表示
    mov si, debug_load_success_msg
    call print_string
    
    ; デバッグ: 読み込み後の状態表示
    mov si, debug_after_load_msg
    call print_string
    
    ; 読み込まれたデータの16進数ダンプ
    call dump_loaded_data
    
    mov si, disk_read_msg
    call print_string
    ret

; 読み込まれたデータの16進数ダンプ
dump_loaded_data:
    mov si, debug_dump_msg
    call print_string
    
    mov si, 0x1000      ; 読み込み先アドレス
    mov cx, 16          ; 16バイトダンプ
    
dump_loop:
    lodsb               ; バイトを読み込み
    call print_hex      ; 16進数で表示
    mov al, ' '         ; スペース
    mov ah, 0x0E
    int 0x10
    loop dump_loop
    
    mov si, newline_msg
    call print_string
    ret

; ディスクエラー処理
disk_error:
    mov si, disk_error_msg
    call print_string
    
    ; エラーコードの表示
    mov si, debug_error_code_msg
    call print_string
    mov al, ah
    call print_hex
    mov si, newline_msg
    call print_string
    
    jmp $

; GDT設定
setup_gdt:
    mov si, gdt_ok_msg
    call print_string
    lgdt [gdt_descriptor]
    ret

; プロテクトモード有効化
enable_protected_mode:
    mov si, protected_mode_msg
    call print_string
    
    cli
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:protected_mode_start

[BITS 32]
protected_mode_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    ret

; カーネル実行
execute_kernel:
    mov si, kernel_start_msg
    call print_string
    
    ; カーネルエントリーポイントを取得
    mov eax, [kernel_entry]
    add eax, 0x100000          ; 1MB + エントリーポイント
    
    ; カーネルにジャンプ
    jmp eax

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

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; 変数定義
kernel_entry: dd 0

; メッセージ定義
boot_msg db 'Retron OS Test Single Sector Stage2 ELF Loader v1.0', 13, 10, 0
loading_msg db 'Loading ELF kernel...', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
disk_error_msg db 'Disk read: ERROR', 13, 10, 0
protected_mode_msg db 'Protected mode...', 13, 10, 0
gdt_ok_msg db 'GDT: OK', 13, 10, 0
kernel_start_msg db 'Jumping to kernel...', 13, 10, 0

; デバッグメッセージ
debug_before_load_msg db 'DEBUG: Before kernel load', 13, 10, 0
debug_after_load_msg db 'DEBUG: After kernel load', 13, 10, 0
debug_load_params_msg db 'DEBUG: Load parameters:', 13, 10, 0
debug_sector_msg db 'DEBUG: Sector: 0x', 0
debug_sector_count_msg db 'DEBUG: Sector count: 0x', 0
debug_load_addr_msg db 'DEBUG: Load address: 0x', 0
debug_load_success_msg db 'DEBUG: Load success', 13, 10, 0
debug_error_code_msg db 'DEBUG: Error code: 0x', 0
debug_before_int13_msg db 'DEBUG: Before INT 0x13', 13, 10, 0
debug_after_int13_msg db 'DEBUG: After INT 0x13', 13, 10, 0
debug_dump_msg db 'DEBUG: Loaded data dump:', 13, 10, 0

newline_msg db 13, 10, 0

