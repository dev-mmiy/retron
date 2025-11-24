#!/bin/bash

# カーネルファイルを配置したディスクイメージの作成

echo "Creating kernel disk image..."

# 1MBの空のディスクイメージを作成
dd if=/dev/zero of=retron-kernel-disk.img bs=1024 count=1024

# ブートローダーを最初のセクタに配置
dd if=retron-minimal-loader.bin of=retron-kernel-disk.img bs=512 count=1 conv=notrunc

# カーネルファイルを2セクタ目から配置
dd if=kernel/target/x86_64-unknown-none/release/retron-kernel of=retron-kernel-disk.img bs=512 seek=1 conv=notrunc

echo "Kernel disk image created: retron-kernel-disk.img"
echo "Bootloader: 0-511 bytes"
echo "Kernel: 512+ bytes"
