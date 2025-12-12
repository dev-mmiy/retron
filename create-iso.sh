#!/bin/bash

KERNEL_PATH="kernel/target/x86_64-unknown-none/release/retron-kernel"
ISO_DIR="build/iso"
BOOT_DIR="$ISO_DIR/boot"
GRUB_DIR="$BOOT_DIR/grub"

# Create directory structure
mkdir -p "$GRUB_DIR"

# Copy kernel
cp "$KERNEL_PATH" "$BOOT_DIR/retron-kernel"

# Create grub.cfg
cat > "$GRUB_DIR/grub.cfg" << 'GRUBCFG'
set timeout=0
set default=0

menuentry "Retron OS" {
    echo "Loading Retron OS..."
    multiboot2 /boot/retron-kernel
    echo "Booting..."
    boot
}
GRUBCFG

# Create ISO
grub-mkrescue -o retron.iso "$ISO_DIR"

echo "ISO created: retron.iso"
