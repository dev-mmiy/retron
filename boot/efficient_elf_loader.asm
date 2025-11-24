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

; カーネル読み込み（効率的ELF）
load_kernel:
    mov si, loading_msg
    call print_string
    
    ; ELFカーネルを読み込み
    mov ah, 0x02        ; 読み込み機能
    mov al, 20          ; 20セクタ読み込み
    mov ch, 0           ; シリンダー
    mov cl, 2           ; セクタ
    mov dh, 0           ; ヘッド
    mov dl, 0x80        ; ドライブ
    mov bx, 0x1000      ; 読み込み先アドレス
    int 0x13            ; ディスク読み込み
    
    jc disk_error       ; エラーチェック
    
    ; ELFヘッダーを解析
    call parse_elf_header
    
    ; プログラムセグメントを読み込み
    call load_program_segments
    
    mov si, disk_read_msg
    call print_string
    ret

; ELFヘッダー解析関数
parse_elf_header:
    mov si, 0x1000
    
    ; ELFマジックナンバーをチェック
    cmp dword [si], 0x464C457F
    jne elf_error
    
    ; エントリーポイントを取得
    mov eax, [si + 24]  ; e_entry
    mov [kernel_entry], eax
    
    ; プログラムヘッダー情報を取得
    mov eax, [si + 32]  ; e_phoff
    mov [ph_offset], eax
    mov ax, [si + 56]   ; e_phnum
    mov [ph_count], ax
    
    ret

; プログラムセグメント読み込み関数
load_program_segments:
    mov si, 0x1000
    add si, [ph_offset]
    
    mov cx, [ph_count]
    cmp cx, 0
    je load_segments_done
    
load_segment_loop:
    ; セグメントタイプをチェック（LOAD = 1）
    cmp dword [si], 1
    jne next_segment
    
    ; セグメントを読み込み
    call load_single_segment
    
next_segment:
    add si, 56                 ; 次のプログラムヘッダー
    loop load_segment_loop
    
load_segments_done:
    ret

; 単一セグメント読み込み関数
load_single_segment:
    ; セグメント情報を取得
    mov eax, [si + 8]          ; p_offset
    mov ebx, [si + 16]         ; p_vaddr
    mov ecx, [si + 32]         ; p_filesz
    
    ; セグメントサイズをチェック
    cmp ecx, 0
    je segment_skip
    
    ; セグメントを読み込み
    push si
    push cx
    push bx
    push ax
    
    ; セクタ数を計算
    mov eax, ecx
    add eax, 511
    shr eax, 9
    
    ; セクタ番号を計算
    mov ebx, [esp + 4]
    add ebx, 512
    shr ebx, 9
    
    ; 読み込み先アドレスを計算
    mov edx, [esp + 8]
    add edx, 0x100000
    
    ; ディスク読み込み
    mov ah, 0x02
    mov al, al
    mov ch, 0
    mov cl, bl
    mov dh, 0
    mov dl, 0x80
    mov bx, dx
    int 0x13
    
    pop ax
    pop bx
    pop cx
    pop si
    
    jc disk_error
    
segment_skip:
    ret

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
    add eax, 0x100000
    
    ; カーネルにジャンプ
    jmp eax

; エラー処理
disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $

elf_error:
    mov si, elf_error_msg
    call print_string
    jmp $

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
ph_offset: dd 0
ph_count: dw 0

; メッセージ定義
boot_msg db 'Retron OS ELF Loader v2.0', 13, 10, 0
loading_msg db 'Loading ELF kernel...', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
disk_error_msg db 'Disk read: ERROR', 13, 10, 0
elf_error_msg db 'ELF: ERROR', 13, 10, 0
protected_mode_msg db 'Protected mode...', 13, 10, 0
gdt_ok_msg db 'GDT: OK', 13, 10, 0
kernel_start_msg db 'Jumping to kernel...', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55
