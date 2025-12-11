#!/bin/bash
echo "Building verified loaders..."

# 第2段階ELFローダーのビルド
echo "Building verified Stage2 ELF loader..."
nasm -f bin boot/verified_stage2_elf_loader.asm -o retron-verified-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Verified Stage2 ELF loader built successfully: retron-verified-stage2-elf-loader.bin"
    ls -l retron-verified-stage2-elf-loader.bin
else
    echo "Error building verified Stage2 ELF loader"
    exit 1
fi
