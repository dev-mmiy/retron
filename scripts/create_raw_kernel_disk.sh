#!/bin/bash

# 生バイナリカーネルを含むディスクイメージの作成

echo "Creating raw kernel disk image..."

# 1MBの空のディスクイメージを作成
dd if=/dev/zero of=retron-raw-kernel-disk.img bs=1024 count=1024

# ブートローダーを最初のセクタに配置
dd if=retron-minimal-loader.bin of=retron-raw-kernel-disk.img bs=512 count=1 conv=notrunc

# 生バイナリカーネルを2セクタ目から配置
dd if=retron-kernel-raw.bin of=retron-raw-kernel-disk.img bs=512 seek=1 conv=notrunc

echo "Raw kernel disk image created: retron-raw-kernel-disk.img"
echo "Bootloader: 0-511 bytes"
echo "Raw kernel: 512+ bytes (4328 bytes)"
