#!/bin/bash
echo "Creating corrected disk layout..."

# 1MBの空のディスクイメージを作成
dd if=/dev/zero of=retron-corrected-disk.img bs=1M count=1

# 第1段階ローダーを最初のセクタに書き込む
dd if=retron-corrected-stage1-loader.bin of=retron-corrected-disk.img bs=512 count=1 conv=notrunc

# 16進数ダンプ版第2段階ELFローダーを2セクタ目（オフセット512バイト）から書き込む
dd if=retron-debug-hexdump-stage2-elf-loader.bin of=retron-corrected-disk.img bs=512 seek=1 conv=notrunc

# ELFカーネルを7セクタ目（オフセット3072バイト）から書き込む
dd if=kernel/target/x86_64-unknown-none/release/retron-kernel of=retron-corrected-disk.img bs=512 seek=6 conv=notrunc

echo "Corrected disk layout created: retron-corrected-disk.img"
echo "Stage1 loader: 0-511 bytes (sector 0)"
echo "Stage2 loader: 512+ bytes (sectors 1-5, $(stat -c %s retron-debug-hexdump-stage2-elf-loader.bin) bytes)"
echo "ELF kernel: 3072+ bytes (sectors 6-26, $(stat -c %s kernel/target/x86_64-unknown-none/release/retron-kernel) bytes)"