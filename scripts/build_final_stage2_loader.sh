#!/bin/bash
echo "Building final Stage2 loader..."

# 最終版第2段階ELFローダーのビルド
echo "Building final Stage2 ELF loader..."
nasm -f bin boot/final_stage2_elf_loader.asm -o retron-final-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Final Stage2 ELF loader built successfully: retron-final-stage2-elf-loader.bin"
    ls -l retron-final-stage2-elf-loader.bin
else
    echo "Error building final Stage2 ELF loader"
    exit 1
fi

