#!/bin/bash
echo "Creating two-stage bootloader disk image..."

# 1MBの空のディスクイメージを作成
dd if=/dev/zero of=retron-two-stage-disk.img bs=1M count=1

# 第1段階ローダーを最初のセクタに書き込む
dd if=retron-stage1-loader.bin of=retron-two-stage-disk.img bs=512 count=1 conv=notrunc

# 第2段階ELFローダーを3セクタ目（オフセット1024バイト）から書き込む
dd if=retron-stage2-elf-loader.bin of=retron-two-stage-disk.img bs=512 seek=2 conv=notrunc

# ELFカーネルを11セクタ目（オフセット5120バイト）から書き込む
dd if=kernel/target/x86_64-unknown-none/release/retron-kernel of=retron-two-stage-disk.img bs=512 seek=10 conv=notrunc

echo "Two-stage bootloader disk image created: retron-two-stage-disk.img"
echo "Stage1 loader: 0-511 bytes"
echo "Stage2 loader: 1024+ bytes ($(stat -c %s retron-stage2-elf-loader.bin) bytes)"
echo "ELF kernel: 5120+ bytes ($(stat -c %s kernel/target/x86_64-unknown-none/release/retron-kernel) bytes)"
