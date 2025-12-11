[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, boot_msg
    call print_string

    ; 第2段階ローダーを読み込み
    call load_stage2

    ; 第2段階ローダーにジャンプ
    call execute_stage2

    jmp $

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

; 第2段階ローダー読み込み
load_stage2:
    mov si, loading_msg
    call print_string
    
    ; 第2段階ローダーを読み込み（複数セクタ）
    mov ah, 0x02        ; 読み込み機能
    mov al, 3           ; 3セクタ読み込み（1.5KB）
    mov ch, 0           ; シリンダー
    mov cl, 2           ; セクタ（第2段階ローダーは2セクタ目から）
    mov dh, 0           ; ヘッド
    mov dl, 0x80        ; ドライブ（ハードディスク）
    mov bx, 0x2000      ; 読み込み先アドレス（8KB）
    int 0x13            ; ディスク読み込み
    
    jc disk_error       ; エラーチェック
    
    mov si, disk_read_msg
    call print_string
    ret

; 第2段階ローダー実行
execute_stage2:
    mov si, stage2_msg
    call print_string
    
    ; 第2段階ローダーにジャンプ
    jmp 0x2000

; エラー処理
disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $

; メッセージ定義
boot_msg db 'Retron OS Fixed Stage1 Loader v1.0', 13, 10, 0
loading_msg db 'Loading Stage2 loader...', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
disk_error_msg db 'Disk read: ERROR', 13, 10, 0
stage2_msg db 'Jumping to Stage2...', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55
