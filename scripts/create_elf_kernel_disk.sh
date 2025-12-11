#!/bin/bash
echo "Creating ELF kernel disk image..."
# 1MBの空のディスクイメージを作成
dd if=/dev/zero of=retron-elf-kernel-disk.img bs=1M count=1
# ブートローダーを最初のセクタに書き込む
dd if=retron-simple-elf-loader.bin of=retron-elf-kernel-disk.img bs=512 count=1 conv=notrunc
# ELFカーネルを2セクタ目（オフセット512バイト）から書き込む
dd if=kernel/target/x86_64-unknown-none/release/retron-kernel of=retron-elf-kernel-disk.img bs=512 seek=1 conv=notrunc
echo "ELF kernel disk image created: retron-elf-kernel-disk.img"
echo "Bootloader: 0-511 bytes"
echo "ELF kernel: 512+ bytes ($(stat -c %s kernel/target/x86_64-unknown-none/release/retron-kernel) bytes)"
