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
    call load_kernel
    call setup_gdt
    call enable_protected_mode
    call execute_kernel
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

load_kernel:
    mov si, loading_msg
    call print_string
    mov ah, 0x02
    mov al, 20
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x1000
    int 0x13
    jc disk_error
    call parse_elf_header
    call load_program_segments
    mov si, disk_read_msg
    call print_string
    ret

parse_elf_header:
    mov si, 0x1000
    cmp dword [si], 0x464C457F
    jne elf_error
    mov eax, [si + 24]
    mov [kernel_entry], eax
    mov eax, [si + 32]
    mov [ph_offset], eax
    mov ax, [si + 56]
    mov [ph_count], ax
    ret

load_program_segments:
    mov si, 0x1000
    add si, [ph_offset]
    mov cx, [ph_count]
    cmp cx, 0
    je load_segments_done
load_segment_loop:
    cmp dword [si], 1
    jne next_segment
    call load_single_segment
next_segment:
    add si, 56
    loop load_segment_loop
load_segments_done:
    ret

load_single_segment:
    mov eax, [si + 8]
    mov ebx, [si + 16]
    mov ecx, [si + 32]
    cmp ecx, 0
    je segment_skip
    push si
    push cx
    push bx
    push ax
    mov eax, ecx
    add eax, 511
    shr eax, 9
    mov ebx, [esp + 4]
    add ebx, 512
    shr ebx, 9
    mov edx, [esp + 8]
    add edx, 0x100000
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

setup_gdt:
    mov si, gdt_ok_msg
    call print_string
    lgdt [gdt_descriptor]
    ret

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

execute_kernel:
    mov si, kernel_start_msg
    call print_string
    mov eax, [kernel_entry]
    add eax, 0x100000
    jmp eax

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $

elf_error:
    mov si, elf_error_msg
    call print_string
    jmp $

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

kernel_entry: dd 0
ph_offset: dd 0
ph_count: dw 0

boot_msg db 'Retron OS ELF Loader v4.0', 13, 10, 0
loading_msg db 'Loading ELF kernel...', 13, 10, 0
disk_read_msg db 'Disk read: OK', 13, 10, 0
disk_error_msg db 'Disk read: ERROR', 13, 10, 0
elf_error_msg db 'ELF: ERROR', 13, 10, 0
protected_mode_msg db 'Protected mode...', 13, 10, 0
gdt_ok_msg db 'GDT: OK', 13, 10, 0
kernel_start_msg db 'Jumping to kernel...', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55
