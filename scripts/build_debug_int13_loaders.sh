#!/bin/bash
echo "Building debug INT13 loaders..."

# INT13デバッグ版第2段階ELFローダーのビルド
echo "Building debug INT13 Stage2 ELF loader..."
nasm -f bin boot/debug_int13_stage2_elf_loader.asm -o retron-debug-int13-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Debug INT13 Stage2 ELF loader built successfully: retron-debug-int13-stage2-elf-loader.bin"
    ls -l retron-debug-int13-stage2-elf-loader.bin
else
    echo "Error building debug INT13 Stage2 ELF loader"
    exit 1
fi

