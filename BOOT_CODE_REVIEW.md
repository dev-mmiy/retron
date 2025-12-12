# Boot Code Review - Critical Issues Found

## Critical Issues in kernel/src/boot.s

### Issue 1: Multiboot Info Lost (Line 71-87)

**Problem**: Multiboot2 magic and info pointer are saved to EDI/ESI, then EDI is immediately overwritten.

```assembly
# Line 71-72: Save Multiboot info
mov %eax, %edi  # EDI = Multiboot2 magic
mov %ebx, %esi  # ESI = Multiboot2 info pointer

# Line 87: EDI is overwritten!
movl $0x104000, %edi  # ← BUG: EDI lost here
```

**Impact**: Kernel cannot access Multiboot2 information structure (memory map, boot modules, etc.)

**Fix**: Save Multiboot info to stack or different registers

### Issue 2: Hardcoded Addresses (Lines 136, 141)

**Problem**: Addresses are hardcoded based on readelf output:

```assembly
# Line 136: GDT pointer address
lgdt 0x103000  # Hardcoded address for gdt64_pointer

# Line 141: Long mode entry point
.long 0x1000ca  # Hardcoded address for long_mode_start
```

**Impact**:
- Fragile - breaks if code is recompiled with different optimization
- Not maintainable - requires manual verification with readelf after each build
- Error-prone - easy to forget to update

**Fix**: Use assembler symbols and linker-calculated addresses

### Issue 3: Inefficient Address Calculation (Lines 61-66)

**Problem**: Code uses call/pop trick to get EIP but doesn't use it:

```assembly
# Get position-independent address (but not used)
call .get_eip
.get_eip:
pop %ebp  # EBP = current EIP
# But then falls back to hardcoded addresses anyway
```

**Impact**: Wasted code

### Issue 4: Page Table Overlap Risk

**Problem**: Hardcoded page table addresses might overlap with other data:

```assembly
Stack: 0x108000 (1MB + 32KB)
PML4:  0x104000 (1MB + 16KB)
PDPT:  0x105000 (1MB + 20KB)
PD:    0x106000 (1MB + 24KB)
GDT:   0x103000 (1MB + 12KB)
```

These addresses are in the middle of the kernel's data segment, which could cause conflicts.

## Recommended Fixes

### Fix 1: Preserve Multiboot Info

```assembly
_start:
    cli
    cld

    # Save Multiboot info to stack FIRST
    push %ebx  # Multiboot2 info pointer
    push %eax  # Multiboot2 magic

    # Set up stack
    movl $0x108000, %esp

    # Restore and save Multiboot info to safe location
    pop %eax   # Multiboot2 magic
    pop %ebx   # Multiboot2 info pointer

    # Save to registers that won't be used
    # (or save to memory location defined in .data)
```

### Fix 2: Use Assembler Symbols

Replace hardcoded addresses with proper symbol references:

```assembly
# Instead of:
lgdt 0x103000

# Use:
lgdt (gdt64_pointer)

# Instead of:
.byte 0xEA
.long 0x1000ca
.word 0x08

# Use:
ljmp $0x08, $long_mode_start
```

### Fix 3: Use .bss Section Page Tables

The page tables are already defined in .bss (lines 26-31), but the code doesn't use them.

Current code references:
- `pml4` symbol exists but 0x104000 is used instead
- `pdpt` symbol exists but 0x105000 is used instead
- `pd` symbol exists but 0x106000 is used instead

**Fix**: Use the symbols:

```assembly
# Instead of hardcoded addresses:
movl $0x104000, %edi

# Use:
leal pml4, %edi
```

## Why This Might Cause SeaBIOS Loop

The hardcoded addresses (especially the long mode jump at 0x1000ca) might be pointing to wrong addresses if:

1. **Code layout changed** during compilation
2. **Linker script changed** memory layout
3. **Optimization level changed** code generation

When GRUB loads the kernel and jumps to `_start`, if the subsequent jump to `long_mode_start` is incorrect, the CPU could:
- Jump to unmapped memory → Page fault → CPU reset → SeaBIOS restart
- Jump to wrong code → Undefined behavior → Triple fault → CPU reset → SeaBIOS restart

## Verification Steps

To verify if addresses are correct:

```bash
# Check actual symbol addresses
readelf -s kernel/target/x86_64-unknown-none/release/retron-kernel | grep -E "long_mode_start|gdt64_pointer|pml4|_start"

# Check section layout
readelf -S kernel/target/x86_64-unknown-none/release/retron-kernel

# Disassemble to verify jump target
objdump -d kernel/target/x86_64-unknown-none/release/retron-kernel | grep -A5 "_start:"
```

## Implementation Priority

1. **High Priority**: Fix Multiboot info preservation (Critical for kernel functionality)
2. **High Priority**: Fix hardcoded jump addresses (Likely cause of SeaBIOS loop)
3. **Medium Priority**: Use proper symbol references for GDT
4. **Low Priority**: Remove unused position-independent code
