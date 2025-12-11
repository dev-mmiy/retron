#!/bin/bash

# Retron OS QEMUデバッグ実行スクリプト

# 設定
KERNEL_PATH="../target/x86_64-unknown-none/debug/retron-kernel"
QEMU_OPTS="-machine q35 -cpu qemu64 -m 512M -serial stdio -nographic -s -S"

# カーネルファイルの存在チェック
if [ ! -f "$KERNEL_PATH" ]; then
    echo "Error: Kernel file not found: $KERNEL_PATH"
    echo "Please build the kernel first with: cargo build"
    exit 1
fi

# QEMUの存在チェック
if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "Error: QEMU not found. Please install QEMU."
    exit 1
fi

echo "Starting Retron OS in QEMU (Debug Mode)..."
echo "Kernel: $KERNEL_PATH"
echo "Options: $QEMU_OPTS"
echo "GDB Server: localhost:1234"
echo "To connect with GDB: gdb -ex 'target remote localhost:1234'"
echo ""

# QEMUを実行
qemu-system-x86_64 -kernel "$KERNEL_PATH" $QEMU_OPTS


