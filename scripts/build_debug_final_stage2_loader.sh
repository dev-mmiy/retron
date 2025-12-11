#!/bin/bash
echo "Building debug final Stage2 loader..."

# デバッグ版最終第2段階ELFローダーのビルド
echo "Building debug final Stage2 ELF loader..."
nasm -f bin boot/debug_final_stage2_elf_loader.asm -o retron-debug-final-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Debug final Stage2 ELF loader built successfully: retron-debug-final-stage2-elf-loader.bin"
    ls -l retron-debug-final-stage2-elf-loader.bin
else
    echo "Error building debug final Stage2 ELF loader"
    exit 1
fi

