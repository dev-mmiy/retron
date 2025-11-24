#!/bin/bash
echo "Building fixed loaders..."

# 第1段階ローダーのビルド
echo "Building fixed Stage1 loader..."
nasm -f bin boot/fixed_stage1_loader.asm -o retron-fixed-stage1-loader.bin
if [ $? -eq 0 ]; then
    echo "Fixed Stage1 loader built successfully: retron-fixed-stage1-loader.bin"
    ls -l retron-fixed-stage1-loader.bin
else
    echo "Error building fixed Stage1 loader"
    exit 1
fi

# 第2段階ELFローダーのビルド
echo "Building fixed Stage2 ELF loader..."
nasm -f bin boot/fixed_stage2_elf_loader.asm -o retron-fixed-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Fixed Stage2 ELF loader built successfully: retron-fixed-stage2-elf-loader.bin"
    ls -l retron-fixed-stage2-elf-loader.bin
else
    echo "Error building fixed Stage2 ELF loader"
    exit 1
fi
