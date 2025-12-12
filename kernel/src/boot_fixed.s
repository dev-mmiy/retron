/*
 * Retron OS Boot Code - Multiboot2
 * Fixed version with proper symbol addressing
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

# Storage for Multiboot2 info
.align 8
multiboot2_magic:
    .skip 4
multiboot2_info:
    .skip 4

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

    # Save Multiboot info immediately before doing anything
    # Store in dedicated memory location
    movl %eax, (multiboot2_magic)
    movl %ebx, (multiboot2_info)

    # Set up stack
    movl $stack_top, %esp

    # Check for long mode support
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    jb .no_long_mode

    mov $0x80000001, %eax
    cpuid
    test $(1 << 29), %edx
    jz .no_long_mode

    # Set up page tables using proper symbols
    # Clear PML4
    movl $pml4, %edi
    xor %eax, %eax
    movl $1024, %ecx
    rep stosl

    # Clear PDPT
    movl $pdpt, %edi
    movl $1024, %ecx
    rep stosl

    # Clear PD
    movl $pd, %edi
    movl $1024, %ecx
    rep stosl

    # PML4[0] -> PDPT
    movl $pdpt, %eax
    or $0x03, %eax           # Present | Writable
    movl $pml4, %edi
    movl %eax, (%edi)

    # PDPT[0] -> PD
    movl $pd, %eax
    or $0x03, %eax           # Present | Writable
    movl $pdpt, %edi
    movl %eax, (%edi)

    # PD[0] -> 2MB page (identity map 0-2MB)
    movl $0x000083, %eax     # Present | Writable | Huge page
    movl $pd, %edi
    movl %eax, (%edi)

    # Load CR3 with PML4 address
    movl $pml4, %eax
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

    # Load GDT using proper symbol
    lgdt (gdt64_pointer)

    # Far jump to 64-bit code
    # We use ljmp with proper symbol reference
    # The assembler/linker will resolve the address
    ljmp $0x08, $long_mode_start

.no_long_mode:
    cli
    hlt
    jmp .no_long_mode

.code64
long_mode_start:
    # Clear segment registers
    xor %ax, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # Set up stack using proper symbol
    movabs $stack_top, %rsp

    # Clear frame pointer
    xor %rbp, %rbp

    # Load Multiboot2 info into registers before calling kernel_main
    # kernel_main(magic: u32, info: *const MultibootInfo)
    movl (multiboot2_magic), %edi    # First argument (magic)
    movl (multiboot2_info), %esi     # Second argument (info pointer)

    # Call kernel_main
    call kernel_main

    # Halt if returned
.Lhang:
    cli
    hlt
    jmp .Lhang
