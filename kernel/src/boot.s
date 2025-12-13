/*
 * Retron OS Boot Code - Multiboot2
 * Fixed version with working PIC addressing
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

# Storage for Multiboot2 info (in .data section for easier addressing)
.align 8
multiboot2_magic:
    .long 0
multiboot2_info:
    .long 0

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

    # Use hardcoded addresses to avoid relocation issues
    # The kernel is loaded at 1MB (0x100000) by GRUB

    # Save Multiboot info - use hardcoded address for multiboot2_magic
    # Based on linker script, .data section is around 0x103000
    # We'll use stack to preserve these values instead
    push %ebx  # Save Multiboot info pointer
    push %eax  # Save Multiboot magic

    # Set up stack (hardcoded address)
    movl $0x108000, %esp

    # Restore Multiboot info from stack
    pop %eax   # Multiboot magic
    pop %ebx   # Multiboot info pointer

    # Save to registers that won't be clobbered
    # We'll pass these to kernel_main later
    mov %eax, %edi  # EDI = Multiboot magic
    mov %ebx, %esi  # ESI = Multiboot info pointer

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
    # We need to save EDI/ESI first
    push %edi
    push %esi

    # Clear PML4 at 0x104000
    movl $0x104000, %edi
    xor %eax, %eax
    movl $1024, %ecx
    rep stosl

    # Clear PDPT at 0x105000
    movl $0x105000, %edi
    movl $1024, %ecx
    rep stosl

    # Clear PD at 0x106000
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

    # Restore EDI/ESI
    pop %esi
    pop %edi

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

    # Load GDT (hardcoded address based on readelf)
    # Get GDT pointer address using call/pop trick
    call .get_gdt_addr
.get_gdt_addr:
    pop %ebp
    # Calculate offset from here to gdt64_pointer
    # This is position-independent
    addl $(gdt64_pointer - .get_gdt_addr), %ebp
    lgdt (%ebp)

    # Far jump to 64-bit code
    # Use the same technique for long_mode_start
    call .get_lm_addr
.get_lm_addr:
    pop %ebp
    addl $(long_mode_start - .get_lm_addr), %ebp

    # Manual far jump: push CS, push EIP, retf
    pushl $0x08        # Code segment
    pushl %ebp         # Address of long_mode_start
    lret               # Far return (acts as far jump)

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

    # Set up stack
    movabs $0x108000, %rsp

    # Clear frame pointer
    xor %rbp, %rbp

    # EDI and ESI still contain Multiboot info from 32-bit mode
    # These will be passed as first two arguments to kernel_main
    # (if kernel_main signature is updated to accept them)

    # Call kernel_main
    call kernel_main

    # Halt if returned
.Lhang:
    cli
    hlt
    jmp .Lhang
