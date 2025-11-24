#!/bin/bash
echo "Building debug hexdump loaders..."

# 16進数ダンプ版第2段階ELFローダーのビルド
echo "Building debug hexdump Stage2 ELF loader..."
nasm -f bin boot/debug_hexdump_stage2_elf_loader.asm -o retron-debug-hexdump-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Debug hexdump Stage2 ELF loader built successfully: retron-debug-hexdump-stage2-elf-loader.bin"
    ls -l retron-debug-hexdump-stage2-elf-loader.bin
else
    echo "Error building debug hexdump Stage2 ELF loader"
    exit 1
fi

