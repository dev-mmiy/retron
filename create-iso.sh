#!/bin/bash
set -e

KERNEL_PATH="kernel/target/x86_64-unknown-none/release/retron-kernel"
ISO_DIR="build/iso"
BOOT_DIR="$ISO_DIR/boot"
GRUB_DIR="$BOOT_DIR/grub"

# Check prerequisites
echo "Checking prerequisites..."

if ! command -v grub-mkrescue &> /dev/null; then
    echo "ERROR: grub-mkrescue not found"
    echo "Install with: sudo apt-get install grub-pc-bin grub-common xorriso mtools"
    exit 1
fi

if ! command -v xorriso &> /dev/null; then
    echo "ERROR: xorriso not found"
    echo "Install with: sudo apt-get install xorriso"
    exit 1
fi

if [ ! -f "$KERNEL_PATH" ]; then
    echo "ERROR: Kernel not found at $KERNEL_PATH"
    echo "Run 'make build' first"
    exit 1
fi

echo "All prerequisites found ✓"

# Create directory structure
echo "Creating ISO directory structure..."
mkdir -p "$GRUB_DIR"

# Copy kernel
echo "Copying kernel..."
cp "$KERNEL_PATH" "$BOOT_DIR/retron-kernel"

# Create grub.cfg with proper configuration
echo "Creating GRUB configuration..."
cat > "$GRUB_DIR/grub.cfg" << 'GRUBCFG'
# Serial console configuration for -nographic mode
serial --unit=0 --speed=115200 --word=8 --parity=no --stop=1
terminal_input serial console
terminal_output serial console

set timeout=1
set default=0

menuentry "Retron OS" {
    multiboot2 /boot/retron-kernel
    boot
}

menuentry "Retron OS (Debug)" {
    set debug=all
    multiboot2 /boot/retron-kernel
    boot
}
GRUBCFG

# Create ISO
echo "Creating bootable ISO..."
grub-mkrescue --verbose -o retron.iso "$ISO_DIR" 2>&1 | tee grub-mkrescue.log

if [ -f retron.iso ]; then
    ISO_SIZE=$(stat -f%z retron.iso 2>/dev/null || stat -c%s retron.iso 2>/dev/null)
    echo "✓ ISO created successfully: retron.iso ($(numfmt --to=iec $ISO_SIZE 2>/dev/null || echo "$ISO_SIZE bytes"))"
    echo "  Run with: make run-iso"
else
    echo "ERROR: ISO creation failed. Check grub-mkrescue.log for details"
    exit 1
fi
