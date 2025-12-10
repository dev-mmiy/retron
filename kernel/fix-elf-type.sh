#!/bin/bash
# Post-build script to fix ELF header type from DYN to EXEC

BINARY="$1"

if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found: $BINARY"
    exit 1
fi

echo "[POST-BUILD] Fixing ELF header: DYN -> EXEC"

# ELF header offset for e_type field is 0x10 (16 bytes)
# We need to change bytes at offset 0x10-0x11 from 03 00 (DYN) to 02 00 (EXEC)

# Check if it's DYN
TYPE=$(od -An -t x1 -N 2 -j 16 "$BINARY" | tr -d ' ')
echo "[POST-BUILD] Current ELF type bytes: $TYPE"
if [ "$TYPE" = "0300" ]; then
    echo "[POST-BUILD] Found DYN type, converting to EXEC..."
    # Write EXEC type (02 00 in little-endian)
    printf '\x02\x00' | dd of="$BINARY" bs=1 seek=16 count=2 conv=notrunc 2>/dev/null
    echo "[POST-BUILD] Conversion complete!"
elif [ "$TYPE" = "0200" ]; then
    echo "[POST-BUILD] Already EXEC type, no change needed"
else
    echo "[POST-BUILD] Attempting to fix unknown type: $TYPE"
    # Try to fix anyway by writing EXEC type
    printf '\x02\x00' | dd of="$BINARY" bs=1 seek=16 count=2 conv=notrunc 2>/dev/null
    echo "[POST-BUILD] Wrote EXEC type"
fi
