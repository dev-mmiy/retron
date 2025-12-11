#!/bin/bash
echo "Creating fixed disk layout..."

# 1MBの空のディスクイメージを作成
dd if=/dev/zero of=retron-fixed-disk.img bs=1M count=1

# 第1段階ローダーを最初のセクタに書き込む
dd if=retron-corrected-stage1-loader.bin of=retron-fixed-disk.img bs=512 count=1 conv=notrunc

# 第2段階ELFローダーを2セクタ目（オフセット512バイト）から書き込む
dd if=retron-debug-hexdump-stage2-elf-loader.bin of=retron-fixed-disk.img bs=512 seek=1 conv=notrunc

# ELFカーネルを4セクタ目（オフセット2048バイト）から書き込む
dd if=kernel/target/x86_64-unknown-none/release/retron-kernel of=retron-fixed-disk.img bs=512 seek=4 conv=notrunc

echo "Fixed disk layout created: retron-fixed-disk.img"
echo "Stage1 loader: 0-511 bytes (sector 0)"
echo "Stage2 loader: 512+ bytes (sectors 1-3+, $(stat -c %s retron-debug-hexdump-stage2-elf-loader.bin) bytes)"
echo "ELF kernel: 2048+ bytes (sectors 4-24+, $(stat -c %s kernel/target/x86_64-unknown-none/release/retron-kernel) bytes)"