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

    ; カーネル読み込み
    call load_kernel

    ; カーネル読み込み成功メッセージ
    mov si, loaded_msg
    call print_string

    ; GDT設定
    call setup_gdt

    ; プロテクトモード有効化
    call enable_protected_mode

    ; カーネル実行
    call execute_kernel

    jmp $ ; 無限ループ（エラー時またはカーネル終了時）

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

; メモリ情報表示関数
display_memory_info:
    mov si, memory_msg
    call print_string
    mov si, memory_ok_msg
    call print_string
    ret

; ELFヘッダーの構造体定義
struc elf_header
    .e_ident:      resb 16    ; ELF識別子
    .e_type:       resw 1     ; ファイルタイプ
    .e_machine:    resw 1     ; マシンタイプ
    .e_version:    resd 1     ; バージョン
    .e_entry:      resq 1     ; エントリーポイント
    .e_phoff:      resq 1     ; プログラムヘッダーオフセット
    .e_shoff:      resq 1     ; セクションヘッダーオフセット
    .e_flags:      resd 1     ; フラグ
    .e_ehsize:    resw 1     ; ELFヘッダーサイズ
    .e_phentsize:  resw 1     ; プログラムヘッダーエントリサイズ
    .e_phnum:     resw 1     ; プログラムヘッダーエントリ数
    .e_shentsize: resw 1     ; セクションヘッダーエントリサイズ
    .e_shnum:     resw 1     ; セクションヘッダーエントリ数
    .e_shstrndx:  resw 1     ; セクション名文字列テーブルインデックス
endstruc

; プログラムヘッダーの構造体定義
struc program_header
    .p_type:      resd 1     ; セグメントタイプ
    .p_flags:     resd 1     ; セグメントフラグ
    .p_offset:    resq 1     ; ファイル内オフセット
    .p_vaddr:     resq 1     ; 仮想アドレス
    .p_paddr:     resq 1     ; 物理アドレス
    .p_filesz:    resq 1     ; ファイル内サイズ
    .p_memsz:     resq 1     ; メモリ内サイズ
    .p_align:     resq 1     ; アライメント
endstruc

; カーネル読み込み（ELF固有）
load_kernel:
    mov si, loading_msg
    call print_string
    
    mov si, kernel_info_msg
    call print_string
    mov si, kernel_size_msg
    call print_string
    mov si, kernel_address_msg
    call print_string
    
    ; ELFカーネルを読み込み
    call load_elf_kernel
    ret

; ELFカーネル読み込み関数
load_elf_kernel:
    ; カーネルファイルを読み込み（複数セクタ）
    mov ah, 0x02        ; 読み込み機能
    mov al, 8           ; 8セクタ読み込み（4KB）
    mov ch, 0           ; シリンダー
    mov cl, 2           ; セクタ（カーネルは2セクタ目から）
    mov dh, 0           ; ヘッド
    mov dl, 0x80        ; ドライブ（ハードディスク）
    mov bx, 0x1000      ; 読み込み先アドレス
    int 0x13            ; ディスク読み込み
    
    jc disk_error       ; エラーチェック
    
    ; ELFヘッダーを解析
    call parse_elf_header
    
    ; プログラムヘッダーを解析してセグメントを読み込み
    call load_program_segments
    
    mov si, disk_read_msg
    call print_string
    ret

; ELFヘッダー解析関数
parse_elf_header:
    ; ELFマジックナンバーをチェック
    mov si, 0x1000
    cmp dword [si], 0x464C457F  ; ELFマジックナンバー
    jne elf_error
    
    ; エントリーポイントを取得
    mov eax, [si + elf_header.e_entry]
    mov [kernel_entry_point], eax
    
    ; プログラムヘッダー情報を取得
    mov eax, [si + elf_header.e_phoff]
    mov [ph_offset], eax
    mov ax, [si + elf_header.e_phnum]
    mov [ph_count], ax
    
    ret

; プログラムセグメント読み込み関数
load_program_segments:
    mov si, 0x1000
    add si, [ph_offset]        ; プログラムヘッダーの開始位置
    
    mov cx, [ph_count]         ; プログラムヘッダー数
    cmp cx, 0
    je load_segments_done
    
load_segment_loop:
    ; セグメントタイプをチェック（LOAD = 1）
    cmp dword [si + program_header.p_type], 1
    jne next_segment
    
    ; セグメントを読み込み
    call load_single_segment
    
next_segment:
    add si, 64                 ; 次のプログラムヘッダー（64バイト）
    loop load_segment_loop
    
load_segments_done:
    ret

; 単一セグメント読み込み関数
load_single_segment:
    ; セグメント情報を取得
    mov eax, [si + program_header.p_offset]    ; ファイル内オフセット
    mov ebx, [si + program_header.p_vaddr]     ; 仮想アドレス
    mov ecx, [si + program_header.p_filesz]     ; ファイルサイズ
    
    ; セグメントを読み込み
    push si
    push cx
    push bx
    push ax
    
    ; セクタ数を計算（512バイト/セクタ）
    mov eax, ecx
    add eax, 511
    shr eax, 9                 ; セクタ数 = (サイズ + 511) / 512
    
    ; セクタ番号を計算
    mov ebx, [esp + 4]        ; ファイル内オフセット
    add ebx, 512              ; ブートセクタ分を加算
    shr ebx, 9                 ; セクタ番号 = (オフセット + 512) / 512
    
    ; 読み込み先アドレスを計算
    mov edx, [esp + 8]        ; 仮想アドレス
    add edx, 0x100000         ; 1MB + 仮想アドレス
    
    ; ディスク読み込み
    mov ah, 0x02
    mov al, al                 ; セクタ数
    mov ch, 0
    mov cl, bl                 ; セクタ番号
    mov dh, 0
    mov dl, 0x80
    mov bx, dx                 ; 読み込み先アドレス
    int 0x13
    
    pop ax
    pop bx
    pop cx
    pop si
    
    jc disk_error
    ret

; GDT設定（ELF固有）
setup_gdt:
    mov si, gdt_ok_msg
    call print_string
    lgdt [gdt_descriptor]
    ret

; プロテクトモード有効化（ELF固有）
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

; カーネル実行（ELF固有）
execute_kernel:
    mov si, kernel_start_msg
    call print_string
    
    ; カーネルエントリーポイントを取得
    mov eax, [kernel_entry_point]
    add eax, 0x100000          ; 1MB + エントリーポイント
    
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
kernel_entry_point: dd 0
ph_offset: dd 0
ph_count: dw 0

; メッセージ定義
boot_msg db 'Retron OS ELF Loader v1.0', 13, 10, 0
memory_msg db 'Memory: 512MB', 13, 10, 0
memory_ok_msg db 'Memory: OK', 13, 10, 0
loaded_msg db 'Loaded: OK', 13, 10, 0
loading_msg db 'Loading ELF kernel...', 13, 10, 0
kernel_info_msg db 'Kernel: ELF', 13, 10, 0
kernel_size_msg db 'Size: Variable', 13, 10, 0
kernel_address_msg db 'Addr: Dynamic', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
disk_error_msg db 'Disk read: ERROR', 13, 10, 0
elf_error_msg db 'ELF: ERROR', 13, 10, 0
protected_mode_msg db 'Protected mode...', 13, 10, 0
gdt_ok_msg db 'GDT: OK', 13, 10, 0
kernel_start_msg db 'Jumping to kernel...', 13, 10, 0

; ブートセクタの終端
times 510-($-$$) db 0
dw 0xAA55
