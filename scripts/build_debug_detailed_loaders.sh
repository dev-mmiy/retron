#!/bin/bash
echo "Building debug detailed loaders..."

# 詳細デバッグ版第2段階ELFローダーのビルド
echo "Building debug detailed Stage2 ELF loader..."
nasm -f bin boot/debug_detailed_stage2_elf_loader.asm -o retron-debug-detailed-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Debug detailed Stage2 ELF loader built successfully: retron-debug-detailed-stage2-elf-loader.bin"
    ls -l retron-debug-detailed-stage2-elf-loader.bin
else
    echo "Error building debug detailed Stage2 ELF loader"
    exit 1
fi

