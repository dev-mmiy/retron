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

; カーネル読み込み（詳細デバッグ付き）
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
    mov al, 5            ; セクタ番号
    call print_hex
    mov si, newline_msg
    call print_string
    
    ; デバッグ: セクタ数の表示
    mov si, debug_sector_count_msg
    call print_string
    mov al, 21           ; セクタ数
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
    
    ; ELFカーネルを読み込み（詳細デバッグ付き）
    mov ah, 0x02        ; 読み込み機能
    mov al, 21          ; 21セクタ読み込み（10.5KB）
    mov ch, 0           ; シリンダー
    mov cl, 5           ; セクタ（カーネルは5セクタ目から）
    mov dh, 0           ; ヘッド
    mov dl, 0x80        ; ドライブ（ハードディスク）
    mov bx, 0x1000      ; 読み込み先アドレス
    int 0x13            ; ディスク読み込み
    
    ; デバッグ: 読み込み結果の表示
    jc disk_error_detailed   ; エラーチェック
    
    ; デバッグ: 読み込み成功の表示
    mov si, debug_load_success_msg
    call print_string
    
    ; デバッグ: 読み込み後の状態表示
    mov si, debug_after_load_msg
    call print_string
    
    ; ELFヘッダーを解析
    call parse_elf_header
    
    ; プログラムセグメントを読み込み
    call load_program_segments
    
    mov si, disk_read_msg
    call print_string
    ret

; 詳細なディスクエラー処理
disk_error_detailed:
    mov si, disk_error_msg
    call print_string
    
    ; エラーコードの表示
    mov si, debug_error_code_msg
    call print_string
    mov al, ah
    call print_hex
    mov si, newline_msg
    call print_string
    
    ; エラー詳細の表示
    mov si, debug_error_details_msg
    call print_string
    
    jmp $

; ELFヘッダー解析関数（詳細デバッグ付き）
parse_elf_header:
    mov si, debug_parse_header_msg
    call print_string
    
    mov si, 0x1000
    
    ; デバッグ: ヘッダーアドレスの表示
    mov si, debug_header_addr_msg
    call print_string
    mov eax, 0x1000
    call print_hex32
    mov si, newline_msg
    call print_string
    
    ; ELFマジックナンバーをチェック
    mov si, debug_magic_check_msg
    call print_string
    
    ; デバッグ: マジックナンバーの表示
    mov si, debug_magic_value_msg
    call print_string
    mov eax, [si]
    call print_hex32
    mov si, newline_msg
    call print_string
    
    cmp dword [si], 0x464C457F  ; ELFマジックナンバー
    jne elf_error_detailed
    
    mov si, debug_magic_ok_msg
    call print_string
    
    ; エントリーポイントを取得
    mov eax, [si + 24]  ; e_entry オフセット
    mov [kernel_entry], eax
    
    ; プログラムヘッダー情報を取得
    mov eax, [si + 32]  ; e_phoff オフセット
    mov [ph_offset], eax
    mov ax, [si + 56]   ; e_phnum オフセット
    mov [ph_count], ax
    
    ; デバッグ情報表示
    mov si, debug_header_parsed_msg
    call print_string
    
    ; エントリーポイントを表示
    mov si, entry_msg
    call print_string
    mov eax, [kernel_entry]
    call print_hex32
    mov si, newline_msg
    call print_string
    
    ; プログラムヘッダーオフセットを表示
    mov si, debug_ph_offset_msg
    call print_string
    mov eax, [ph_offset]
    call print_hex32
    mov si, newline_msg
    call print_string
    
    ; プログラムヘッダー数を表示
    mov si, debug_ph_count_msg
    call print_string
    mov ax, [ph_count]
    call print_hex
    mov si, newline_msg
    call print_string
    
    ret

; 詳細なELFエラー処理
elf_error_detailed:
    mov si, elf_error_msg
    call print_string
    
    ; エラー詳細の表示
    mov si, debug_elf_error_details_msg
    call print_string
    
    jmp $

; プログラムセグメント読み込み関数（詳細デバッグ付き）
load_program_segments:
    mov si, debug_segments_msg
    call print_string
    
    mov si, 0x1000
    add si, [ph_offset]        ; プログラムヘッダーの開始位置
    
    ; デバッグ: プログラムヘッダーアドレスの表示
    mov si, debug_ph_addr_msg
    call print_string
    mov eax, 0x1000
    add eax, [ph_offset]
    call print_hex32
    mov si, newline_msg
    call print_string
    
    mov cx, [ph_count]         ; プログラムヘッダー数
    cmp cx, 0
    je load_segments_done
    
    mov si, segments_msg
    call print_string
    
load_segment_loop:
    ; デバッグ: セグメントループの表示
    mov si, debug_segment_loop_msg
    call print_string
    mov ax, cx
    call print_hex
    mov si, newline_msg
    call print_string
    
    ; セグメントタイプをチェック（LOAD = 1）
    cmp dword [si], 1
    jne next_segment
    
    ; デバッグ: セグメント情報を表示
    mov si, debug_segment_info_msg
    call print_string
    
    ; セグメントを読み込み
    call load_single_segment
    
next_segment:
    add si, 56                 ; 次のプログラムヘッダー（56バイト）
    loop load_segment_loop
    
load_segments_done:
    mov si, debug_segments_done_msg
    call print_string
    ret

; 単一セグメント読み込み関数（詳細デバッグ付き）
load_single_segment:
    ; セグメント情報を取得
    mov eax, [si + 8]          ; p_offset
    mov ebx, [si + 16]         ; p_vaddr
    mov ecx, [si + 32]         ; p_filesz
    
    ; デバッグ: セグメント情報を表示
    push si
    mov si, debug_offset_msg
    call print_string
    mov eax, eax
    call print_hex32
    mov si, newline_msg
    call print_string
    
    mov si, debug_vaddr_msg
    call print_string
    mov eax, ebx
    call print_hex32
    mov si, newline_msg
    call print_string
    
    mov si, debug_filesz_msg
    call print_string
    mov eax, ecx
    call print_hex32
    mov si, newline_msg
    call print_string
    pop si
    
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
    shr eax, 9                 ; セクタ数 = (サイズ + 511) / 512
    
    ; デバッグ: セクタ数情報を表示
    push eax
    mov si, debug_sectors_msg
    call print_string
    mov eax, eax
    call print_hex
    mov si, newline_msg
    call print_string
    pop eax
    
    ; セクタ番号を計算（詳細デバッグ付き）
    mov ebx, [esp + 4]        ; ファイル内オフセット
    add ebx, 2048             ; カーネル開始オフセット（5セクタ * 512バイト）
    shr ebx, 9                 ; セクタ番号 = (オフセット + 2048) / 512
    
    ; デバッグ: セクタ番号情報を表示
    push eax
    mov si, debug_sector_num_msg
    call print_string
    mov eax, ebx
    call print_hex
    mov si, newline_msg
    call print_string
    pop eax
    
    ; 読み込み先アドレスを計算
    mov edx, [esp + 8]        ; 仮想アドレス
    add edx, 0x100000         ; 1MB + 仮想アドレス
    
    ; デバッグ: 読み込み先アドレス情報を表示
    push eax
    mov si, debug_load_addr_msg
    call print_string
    mov eax, edx
    call print_hex32
    mov si, newline_msg
    call print_string
    pop eax
    
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
    
    jc disk_error_detailed
    
    ; デバッグ: セグメント読み込み成功
    mov si, debug_segment_loaded_msg
    call print_string
    
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
kernel_entry: dd 0
ph_offset: dd 0
ph_count: dw 0

; メッセージ定義
boot_msg db 'Retron OS Debug Detailed Stage2 ELF Loader v1.0', 13, 10, 0
loading_msg db 'Loading ELF kernel...', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
disk_error_msg db 'Disk read: ERROR', 13, 10, 0
elf_error_msg db 'ELF: ERROR', 13, 10, 0
protected_mode_msg db 'Protected mode...', 13, 10, 0
gdt_ok_msg db 'GDT: OK', 13, 10, 0
kernel_start_msg db 'Jumping to kernel...', 13, 10, 0

; 詳細デバッグメッセージ
debug_before_load_msg db 'DEBUG: Before kernel load', 13, 10, 0
debug_after_load_msg db 'DEBUG: After kernel load', 13, 10, 0
debug_load_params_msg db 'DEBUG: Load parameters:', 13, 10, 0
debug_sector_msg db 'DEBUG: Sector: 0x', 0
debug_sector_count_msg db 'DEBUG: Sector count: 0x', 0
debug_load_addr_msg db 'DEBUG: Load address: 0x', 0
debug_load_success_msg db 'DEBUG: Load success', 13, 10, 0
debug_error_code_msg db 'DEBUG: Error code: 0x', 0
debug_error_details_msg db 'DEBUG: Error details', 13, 10, 0
debug_parse_header_msg db 'DEBUG: Parsing ELF header', 13, 10, 0
debug_header_addr_msg db 'DEBUG: Header address: 0x', 0
debug_magic_check_msg db 'DEBUG: Checking ELF magic', 13, 10, 0
debug_magic_value_msg db 'DEBUG: Magic value: 0x', 0
debug_magic_ok_msg db 'DEBUG: ELF magic OK', 13, 10, 0
debug_header_parsed_msg db 'DEBUG: Header parsed', 13, 10, 0
debug_ph_offset_msg db 'DEBUG: PH offset: 0x', 0
debug_ph_count_msg db 'DEBUG: PH count: 0x', 0
debug_segments_msg db 'DEBUG: Loading segments', 13, 10, 0
debug_ph_addr_msg db 'DEBUG: PH address: 0x', 0
debug_segment_loop_msg db 'DEBUG: Segment loop: 0x', 0
debug_segments_done_msg db 'DEBUG: Segments loaded', 13, 10, 0
debug_segment_info_msg db 'DEBUG: Segment info:', 13, 10, 0
debug_offset_msg db 'DEBUG: Offset: 0x', 0
debug_vaddr_msg db 'DEBUG: VAddr: 0x', 0
debug_filesz_msg db 'DEBUG: FileSz: 0x', 0
debug_sectors_msg db 'DEBUG: Sectors: 0x', 0
debug_sector_num_msg db 'DEBUG: Sector: 0x', 0
debug_segment_loaded_msg db 'DEBUG: Segment loaded', 13, 10, 0
debug_elf_error_details_msg db 'DEBUG: ELF error details', 13, 10, 0

entry_msg db 'Entry point: 0x', 0
newline_msg db 13, 10, 0
segments_msg db 'Loading segments...', 13, 10, 0

