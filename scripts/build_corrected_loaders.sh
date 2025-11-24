#!/bin/bash
echo "Building corrected loaders..."

# 第1段階ローダーのビルド
echo "Building corrected Stage1 loader..."
nasm -f bin boot/corrected_stage1_loader.asm -o retron-corrected-stage1-loader.bin
if [ $? -eq 0 ]; then
    echo "Corrected Stage1 loader built successfully: retron-corrected-stage1-loader.bin"
    ls -l retron-corrected-stage1-loader.bin
else
    echo "Error building corrected Stage1 loader"
    exit 1
fi

# 第2段階ELFローダーのビルド
echo "Building corrected Stage2 ELF loader..."
nasm -f bin boot/corrected_stage2_elf_loader.asm -o retron-corrected-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Corrected Stage2 ELF loader built successfully: retron-corrected-stage2-elf-loader.bin"
    ls -l retron-corrected-stage2-elf-loader.bin
else
    echo "Error building corrected Stage2 ELF loader"
    exit 1
fi
