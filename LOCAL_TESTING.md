# Retron OS - Local Testing Guide

## Overview

This guide explains how to build and test Retron OS on your local machine using QEMU.

## Prerequisites

### Required Tools

1. **Rust Toolchain** (already configured via rust-toolchain.toml):
   ```bash
   rustup target add x86_64-unknown-none
   ```

2. **Build Tools**:
   ```bash
   sudo apt-get install build-essential gcc binutils
   ```

3. **QEMU x86_64**:
   ```bash
   sudo apt-get install qemu-system-x86
   ```

4. **GRUB Tools** (for ISO boot):
   ```bash
   sudo apt-get install grub-pc-bin grub-common xorriso mtools
   ```

### Verify Installation

```bash
# Check Rust
rustc --version
cargo --version

# Check QEMU
qemu-system-x86_64 --version

# Check GRUB tools
grub-mkrescue --version
xorriso --version

# Check assembler and linker
as --version
ld --version
```

## Build Instructions

### 1. Build the Kernel

```bash
make build
```

This will:
- Compile Rust code to x86_64-unknown-none target
- Assemble boot.s (32-bit → 64-bit transition code)
- Link everything together
- Fix ELF header type (DYN → EXEC if needed)

**Output**: `kernel/target/x86_64-unknown-none/release/retron-kernel` (~19KB)

### 2. Create Bootable ISO

```bash
make iso
```

This will:
- Run the build if needed
- Create ISO directory structure
- Copy kernel to ISO
- Generate GRUB configuration
- Create bootable ISO with grub-mkrescue

**Output**: `retron.iso` (~5MB)

## Testing Methods

### Method 1: Direct Kernel Loading (Simple, Fast)

**Note**: This method does NOT work with Multiboot2 ELF files. QEMU's `-kernel` option expects PVH boot protocol for ELF64 binaries.

```bash
make run-qemu
```

**Expected**: Will fail with "Cannot load x86-64 image, give a 32bit one"

**Why it fails**: QEMU's direct kernel loading requires either:
- PVH boot protocol (which we tried but didn't work)
- Raw binary format (not ELF)
- 32-bit ELF

### Method 2: ISO Boot via GRUB (Recommended)

```bash
make run-iso
```

This boots from the ISO in `-nographic` mode (serial console).

**Expected output**:
```
SeaBIOS (bios-256k.bin)
...
GRUB loading
...
Retron OS kernel output
```

#### VGA Mode (for debugging GRUB):

```bash
make run-iso-vga
```

This opens a graphical window showing GRUB menu.

## Troubleshooting

### Issue 1: "grub-mkrescue: command not found"

**Solution**:
```bash
sudo apt-get install grub-pc-bin grub-common xorriso mtools
```

### Issue 2: SeaBIOS Loops, GRUB Never Loads

**Symptoms**: QEMU shows repeating SeaBIOS messages, never gets to GRUB

**Possible causes**:

1. **ISO not properly bootable**

   Check ISO boot catalog:
   ```bash
   # Mount ISO and verify structure
   mkdir -p /tmp/iso_mount
   sudo mount -o loop retron.iso /tmp/iso_mount
   ls -la /tmp/iso_mount/boot/
   ls -la /tmp/iso_mount/boot/grub/
   cat /tmp/iso_mount/boot/grub/grub.cfg
   sudo umount /tmp/iso_mount
   ```

   Should contain:
   - `/boot/retron-kernel` (~19KB)
   - `/boot/grub/grub.cfg` (GRUB menu config)
   - `/boot/grub/i386-pc/` (GRUB modules)

2. **GRUB not installed in ISO boot sector**

   Check grub-mkrescue output:
   ```bash
   cat grub-mkrescue.log
   ```

   Look for errors like:
   - "xorriso : FAILURE"
   - "grub-mkrescue: error: cannot find GRUB files"

3. **QEMU not configured for CD-ROM boot**

   Try explicit boot order:
   ```bash
   qemu-system-x86_64 \
       -cdrom retron.iso \
       -m 128M \
       -nographic \
       -boot d  # Force CD-ROM boot
   ```

### Issue 3: No Kernel Output After GRUB

**Symptoms**: GRUB loads and boots kernel, but no output

**Debug steps**:

1. **Verify kernel was loaded**:
   ```bash
   qemu-system-x86_64 -cdrom retron.iso -m 128M -nographic -d guest_errors,cpu_reset
   ```

2. **Check if kernel is hanging in boot.s**:

   Use GDB:
   ```bash
   # Terminal 1: Start QEMU with GDB server
   qemu-system-x86_64 -cdrom retron.iso -m 128M -nographic -s -S

   # Terminal 2: Connect GDB
   gdb kernel/target/x86_64-unknown-none/release/retron-kernel
   (gdb) target remote :1234
   (gdb) break *0x100018  # _start entry point
   (gdb) break long_mode_start
   (gdb) break kernel_main
   (gdb) continue
   ```

3. **Check Multiboot2 info passed to kernel**:

   The kernel receives:
   - EAX = Multiboot2 magic (0x36d76289)
   - EBX = Address of Multiboot2 information structure

   In boot.s, these are saved to EDI and ESI before long mode transition.

### Issue 4: "Invalid read" or Page Fault

**Symptoms**: QEMU shows CPU exception or invalid memory access

**Possible causes**:

1. **Page tables not set up correctly**

   Check boot.s identity mapping:
   ```assembly
   # PML4[0] -> PDPT at 0x105000
   # PDPT[0] -> PD at 0x106000
   # PD[0] -> 2MB page at 0x000000
   ```

2. **Stack not properly initialized**

   Check boot.s stack setup:
   ```assembly
   movabs $0x108000, %rsp  # Stack at 1MB + 32KB
   ```

3. **GDT addresses incorrect**

   Verify with:
   ```bash
   readelf -s kernel/target/x86_64-unknown-none/release/retron-kernel | grep gdt64
   ```

### Issue 5: Build Warnings

**Unused imports and dead code**:

These warnings are safe to ignore for now:
```
warning: unused import: `core::mem::MaybeUninit`
warning: constant `MULTIBOOT_MAGIC` is never used
```

The PVH-related code is not currently used (we're using Multiboot2).

## Advanced Debugging

### Dump Assembly

```bash
objdump -d kernel/target/x86_64-unknown-none/release/retron-kernel > kernel.asm
```

### Check ELF Structure

```bash
readelf -a kernel/target/x86_64-unknown-none/release/retron-kernel
```

### Check Multiboot2 Header

```bash
readelf -x .multiboot kernel/target/x86_64-unknown-none/release/retron-kernel
```

Should show:
```
Hex dump of section '.multiboot':
  0x00100000 d650e825 00000000 18000000 e8af17da ........
  0x00100010 00000000 00000800                  ........
```

### QEMU Monitor

Access QEMU monitor during runtime:
```bash
# Start with monitor on stdio
qemu-system-x86_64 -cdrom retron.iso -m 128M -monitor stdio

# Commands:
(qemu) info registers
(qemu) info mem
(qemu) info tlb
(qemu) x/10i $pc  # Disassemble at current PC
```

## Boot Process Flow

1. **QEMU** loads GRUB from ISO boot sector
2. **GRUB** reads `/boot/grub/grub.cfg`
3. **GRUB** loads `/boot/retron-kernel` into memory at 1MB
4. **GRUB** sets up Multiboot2 info structure
5. **GRUB** jumps to kernel entry point (`_start` at 0x100018)
6. **boot.s:_start** (32-bit mode):
   - Sets up stack
   - Saves Multiboot2 info (EAX, EBX)
   - Checks CPU supports long mode
   - Sets up page tables (identity map 0-2MB)
   - Enables PAE, long mode, paging
   - Loads GDT
   - Far jump to long mode
7. **boot.s:long_mode_start** (64-bit mode):
   - Clears segment registers
   - Sets up stack
   - Calls kernel_main
8. **kernel_main** (Rust):
   - Initializes memory manager
   - Sets up interrupts
   - Starts kernel services

## Next Steps

Once the kernel boots successfully:

1. **Verify boot logs**: Check that kernel initialization messages appear
2. **Test memory management**: Verify heap allocation works
3. **Continue Phase 2**: Implement remaining memory management features
4. **Add testing**: Create automated test suite

## References

- [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)
- [OSDev Wiki - Multiboot](https://wiki.osdev.org/Multiboot)
- [Intel 64 and IA-32 Architectures Manual](https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-manual-325462.html)
- [QEMU Documentation](https://www.qemu.org/docs/master/)
