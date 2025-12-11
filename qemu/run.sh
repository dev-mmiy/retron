#!/bin/bash

# Retron OS QEMU実行スクリプト

# 設定
KERNEL_PATH="../target/x86_64-unknown-none/release/retron-kernel"
QEMU_OPTS="-machine q35 -cpu qemu64 -m 512M -serial stdio -nographic"

# カーネルファイルの存在チェック
if [ ! -f "$KERNEL_PATH" ]; then
    echo "Error: Kernel file not found: $KERNEL_PATH"
    echo "Please build the kernel first with: cargo build --release"
    exit 1
fi

# QEMUの存在チェック
if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "Error: QEMU not found. Please install QEMU."
    exit 1
fi

echo "Starting Retron OS in QEMU..."
echo "Kernel: $KERNEL_PATH"
echo "Options: $QEMU_OPTS"
echo "Press Ctrl+A, X to exit QEMU"
echo ""

# QEMUを実行
qemu-system-x86_64 -kernel "$KERNEL_PATH" $QEMU_OPTS


