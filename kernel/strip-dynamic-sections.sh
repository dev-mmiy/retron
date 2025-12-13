#!/bin/bash
# Strip dynamic sections that confuse QEMU PVH loader

BINARY="$1"

if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found: $BINARY"
    exit 1
fi

echo "[STRIP-DYNAMIC] Removing dynamic sections from kernel binary..."

# Remove dynamic sections using objcopy
objcopy \
    --remove-section=.dynsym \
    --remove-section=.gnu.hash \
    --remove-section=.hash \
    --remove-section=.dynstr \
    --remove-section=.dynamic \
    "$BINARY" "$BINARY.tmp"

# Replace original with stripped version
mv "$BINARY.tmp" "$BINARY"

echo "[STRIP-DYNAMIC] Dynamic sections removed successfully"
