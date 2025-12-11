#!/bin/bash
set -euo pipefail

STAGE1_BIN=${1:-retron-corrected-stage1-loader.bin}
STAGE2_BIN=${2:-retron-corrected-final-stage2-elf-loader.bin}
KERNEL_BIN=${3:-kernel/target/x86_64-unknown-none/release/retron-kernel}
OUT_IMG=${4:-retron-canonical-disk.img}

echo "Creating canonical disk layout..."

# 1MB empty image
dd if=/dev/zero of="$OUT_IMG" bs=1M count=1 status=none

# Write stage1 to LBA 0
dd if="$STAGE1_BIN" of="$OUT_IMG" bs=512 count=1 conv=notrunc status=none

# Write stage2 to LBA 1
dd if="$STAGE2_BIN" of="$OUT_IMG" bs=512 seek=1 conv=notrunc status=none

# Compute kernel start LBA = 1 + ceil(stage2_size/512)
STAGE2_SIZE=$(stat -c %s "$STAGE2_BIN")
KERNEL_LBA=$((1 + ( (STAGE2_SIZE + 511) / 512 )))

# Write kernel from computed LBA
dd if="$KERNEL_BIN" of="$OUT_IMG" bs=512 seek="$KERNEL_LBA" conv=notrunc status=none

echo "Canonical disk layout created: $OUT_IMG"
echo "Stage1: LBA 0 (512 bytes) -> $STAGE1_BIN ($(stat -c %s "$STAGE1_BIN") bytes)"
echo "Stage2: LBA 1..$((KERNEL_LBA-1)) -> $STAGE2_BIN ($STAGE2_SIZE bytes)"
echo "Kernel: LBA $KERNEL_LBA.. -> $KERNEL_BIN ($(stat -c %s "$KERNEL_BIN") bytes)"


