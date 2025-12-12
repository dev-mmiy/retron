/*
 * Retron OS Boot Code - Multiboot2
 * Simplified version to avoid relocation issues
 */

.section .multiboot
.align 8

multiboot_header_start:
    .long 0xe85250d6                                        # Magic
    .long 0                                                  # Architecture (i386)
    .long multiboot_header_end - multiboot_header_start     # Header length
    .long -(0xe85250d6 + (multiboot_header_end - multiboot_header_start))  # Checksum

    # End tag
    .align 8
    .short 0
    .short 0
    .long 8
multiboot_header_end:

.section .bss
.align 4096

# Page tables
pml4:
    .skip 4096
pdpt:
    .skip 4096
pd:
    .skip 4096

# Stack
.align 16
stack_bottom:
    .skip 16384
stack_top:

.section .data

# GDT for long mode
.align 16
gdt64:
    .quad 0                                      # Null descriptor
    .quad 0x00209A0000000000                     # 64-bit code segment
    .quad 0x0000920000000000                     # 64-bit data segment
gdt64_end:

gdt64_pointer:
    .word gdt64_end - gdt64 - 1
    .quad gdt64

.section .text
.code32
.global _start
_start:
    cli
    cld

    # Set up stack - use call/pop trick to get EIP
    call .get_eip
.get_eip:
    pop %ebp
    # EBP now contains EIP, calculate base address
    # We're at 1MB + some offset

    # For simplicity, use hardcoded address (kernel is at 1MB)
    movl $0x108000, %esp  # Stack at 1MB + 32KB

    # Save Multiboot info
    mov %eax, %edi
    mov %ebx, %esi

    # Check for long mode support
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    jb .no_long_mode

    mov $0x80000001, %eax
    cpuid
    test $(1 << 29), %edx
    jz .no_long_mode

    # Set up page tables (hardcoded addresses)
    # Clear PML4
    movl $0x104000, %edi  # pml4 at 1MB + 16KB
    xor %eax, %eax
    movl $1024, %ecx
    rep stosl

    # Clear PDPT
    movl $0x105000, %edi
    movl $1024, %ecx
    rep stosl

    # Clear PD
    movl $0x106000, %edi
    movl $1024, %ecx
    rep stosl

    # PML4[0] -> PDPT
    movl $0x105003, %eax  # PDPT address | Present | Writable
    movl %eax, 0x104000

    # PDPT[0] -> PD
    movl $0x106003, %eax  # PD address | Present | Writable
    movl %eax, 0x105000

    # PD[0] -> 2MB page (identity map 0-2MB)
    movl $0x000083, %eax  # Present | Writable | Huge page
    movl %eax, 0x106000

    # Load CR3 with PML4 address
    movl $0x104000, %eax
    mov %eax, %cr3

    # Enable PAE
    mov %cr4, %eax
    or $(1 << 5), %eax
    mov %eax, %cr4

    # Enable long mode in EFER
    mov $0xC0000080, %ecx
    rdmsr
    or $(1 << 8), %eax
    wrmsr

    # Enable paging
    mov %cr0, %eax
    or $(1 << 31), %eax
    mov %eax, %cr0

    # Load GDT (hardcoded address)
    # gdt64_pointer is in .data section at 0x103000
    lgdt 0x103000

    # Far jump to 64-bit code (hardcoded)
    # long_mode_start is at 0x1000ca (verified with readelf)
    .byte 0xEA
    .long 0x1000ca
    .word 0x08      # Code segment

.no_long_mode:
    cli
    hlt
    jmp .no_long_mode

.code64
long_mode_start:
    # Clear segment registers
    mov $0, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # Set up stack - use hardcoded address
    movabs $0x108000, %rsp

    # Clear frame pointer
    xor %rbp, %rbp

    # Call kernel_main
    call kernel_main

    # Halt if returned
.Lhang:
    cli
    hlt
    jmp .Lhang
