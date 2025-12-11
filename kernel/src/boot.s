# PVH Boot Entry Point
#
# This is the actual entry point for PVH boot protocol.
# QEMU jumps here directly in 64-bit long mode with:
# - RSI: pointer to start_info structure
# - Paging enabled
# - GDT/IDT set up by firmware
#
# We need to:
# 1. Set up a stack
# 2. Save boot parameters
# 3. Call kernel_main

.section .text.pvh_entry, "ax", @progbits
.global pvh_entry
.type pvh_entry, @function

pvh_entry:
    # Clear direction flag (required by System V ABI)
    cld

    # Set up stack pointer to our boot stack
    # The stack grows downward, so we point to the end
    leaq boot_stack_end(%rip), %rax
    movq %rax, %rsp

    # Clear frame pointer
    xorq %rbp, %rbp

    # RSI contains pointer to PVH start_info structure
    # Save it for potential future use (first argument to kernel_main in System V ABI)
    # But our current kernel_main takes no arguments, so we just clear it
    xorq %rdi, %rdi
    xorq %rsi, %rsi
    xorq %rdx, %rdx
    xorq %rcx, %rcx
    xorq %r8, %r8
    xorq %r9, %r9

    # Call the Rust kernel main function
    call kernel_main

    # If kernel_main returns (it shouldn't), halt
.Lhalt:
    cli
    hlt
    jmp .Lhalt

.size pvh_entry, . - pvh_entry

# Boot stack (16KB)
.section .bss
.align 16
boot_stack:
    .skip 16384  # 16KB stack
boot_stack_end:
