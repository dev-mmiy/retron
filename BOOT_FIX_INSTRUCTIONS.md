# Boot Code Fix Instructions

## Summary

The original `boot.s` has critical bugs that likely cause the SeaBIOS boot loop. A fixed version `boot_fixed.s` addresses these issues.

## Bugs in Original boot.s

### Bug #1: Lost Multiboot2 Information
```assembly
# Line 71-72: Save to EDI/ESI
mov %eax, %edi
mov %ebx, %esi

# Line 87: EDI immediately overwritten!
movl $0x104000, %edi  # Bug: Multiboot magic lost
```

### Bug #2: Hardcoded Addresses
```assembly
# Line 136: Hardcoded GDT address
lgdt 0x103000

# Line 141: Hardcoded jump address
.long 0x1000ca
```

These addresses were manually verified with `readelf` but will break if:
- Code is recompiled with different settings
- Linker script changes
- Optimization level changes

### Bug #3: Unused Page Table Symbols
```assembly
# Symbols defined in .bss:
pml4: .skip 4096
pdpt: .skip 4096
pd: .skip 4096

# But code uses hardcoded addresses instead:
movl $0x104000, %edi  # Instead of: movl $pml4, %edi
```

## Fixes in boot_fixed.s

### Fix #1: Dedicated Multiboot Storage
```assembly
# In .bss section:
multiboot2_magic: .skip 4
multiboot2_info: .skip 4

# In _start:
movl %eax, (multiboot2_magic)  # Save immediately
movl %ebx, (multiboot2_info)
```

### Fix #2: Symbol-Relative Addressing
```assembly
# Use symbols instead of hardcoded addresses:
lgdt (gdt64_pointer)           # Instead of: lgdt 0x103000
ljmp $0x08, $long_mode_start   # Instead of: .long 0x1000ca
```

### Fix #3: Use Proper Page Table Symbols
```assembly
movl $pml4, %edi     # Use symbol
movl $pdpt, %eax     # Use symbol
movl $pd, %eax       # Use symbol
movl $stack_top, %esp  # Use symbol
```

## How to Apply the Fix

### Option 1: Replace boot.s (Recommended)

```bash
# Backup original
cp kernel/src/boot.s kernel/src/boot.s.original

# Use fixed version
cp kernel/src/boot_fixed.s kernel/src/boot.s

# Rebuild
make clean
make build
make iso
make run-iso
```

### Option 2: Compare and Manually Patch

```bash
# Compare the two versions
diff -u kernel/src/boot.s kernel/src/boot_fixed.s

# Apply selected fixes manually
vim kernel/src/boot.s
```

## Testing the Fix

### Step 1: Build and Verify

```bash
make build
```

Expected output:
```
Building Retron OS...
cargo build --manifest-path kernel/Cargo.toml --release
Fixing ELF header...
Build completed!
```

### Step 2: Check Symbol Addresses

```bash
readelf -s kernel/target/x86_64-unknown-none/release/retron-kernel | \
  grep -E "long_mode_start|gdt64_pointer|pml4|pdpt|pd|stack_top"
```

Expected: All symbols should have addresses in the 0x100000-0x108000 range

### Step 3: Create ISO

```bash
make iso
```

Expected output (with fixed create-iso.sh):
```
Checking prerequisites...
All prerequisites found ✓
Creating ISO directory structure...
Copying kernel...
Creating GRUB configuration...
Creating bootable ISO...
✓ ISO created successfully: retron.iso (5.1M)
  Run with: make run-iso
```

### Step 4: Test Boot

```bash
make run-iso
```

Expected output (if fix works):
```
SeaBIOS ...
Booting from CD-ROM...
GRUB loading
Loading Retron OS...
[Kernel output should appear here]
KERNEL_MAIN: Starting...
KERNEL_MAIN: Debug test 1
...
```

**If still stuck at SeaBIOS**: See troubleshooting below

### Step 5: Debug Boot (if needed)

```bash
# Terminal 1: Start QEMU with GDB
qemu-system-x86_64 -cdrom retron.iso -m 128M -nographic -s -S

# Terminal 2: Connect GDB
gdb kernel/target/x86_64-unknown-none/release/retron-kernel
(gdb) target remote :1234
(gdb) break *0x100000  # Multiboot header location
(gdb) break _start
(gdb) break long_mode_start
(gdb) break kernel_main
(gdb) continue
```

Watch for:
1. Does GRUB load the kernel? (Break at 0x100000)
2. Does _start execute? (Check with `info registers`)
3. Does long mode transition succeed?
4. Does kernel_main get called?

## Why This Should Fix SeaBIOS Loop

The SeaBIOS loop likely occurs because:

1. **Wrong jump address**: If `long_mode_start` address is incorrect (hardcoded 0x1000ca), CPU jumps to wrong location
   - → Executes invalid instruction
   - → Triple fault
   - → CPU reset
   - → SeaBIOS restart
   - → Loop

2. **Wrong GDT address**: If GDT pointer (hardcoded 0x103000) is incorrect
   - → Invalid GDT loaded
   - → Segment fault on mode switch
   - → Triple fault
   - → CPU reset
   - → Loop

**The fix uses assembler symbols**, which are guaranteed to be correct by the linker.

## Updating kernel_main (Optional)

To receive Multiboot2 info in kernel_main:

```rust
// In kernel/src/main.rs:

#[no_mangle]
pub extern "C" fn kernel_main(magic: u32, info_addr: u32) -> ! {
    // Verify magic number
    if magic != 0x36d76289 {
        simple::println("ERROR: Invalid Multiboot2 magic!");
    } else {
        simple::println("Multiboot2 boot confirmed");
        // Parse info structure at info_addr...
    }

    // Rest of kernel initialization...
}
```

Then update boot_fixed.s to pass parameters:
```assembly
# Already done in boot_fixed.s:
movl (multiboot2_magic), %edi    # First argument
movl (multiboot2_info), %esi     # Second argument
call kernel_main
```

## Verification Checklist

- [ ] boot.s replaced with boot_fixed.s
- [ ] make clean && make build succeeds
- [ ] Symbol addresses verified with readelf
- [ ] ISO created successfully (make iso)
- [ ] GRUB loads from ISO (make run-iso-vga shows menu)
- [ ] Kernel output appears (no SeaBIOS loop)
- [ ] Memory management tests pass

## Rollback

If the fix causes issues:

```bash
# Restore original
cp kernel/src/boot.s.original kernel/src/boot.s

# Rebuild
make clean
make build
```

## Next Steps After Boot Success

1. Verify Multiboot2 info parsing
2. Implement proper memory map from Multiboot2
3. Continue Phase 2 development
4. Add automated boot tests
