#!/bin/bash
echo "Building corrected final Stage2 loader..."

# 修正版最終第2段階ELFローダーのビルド
echo "Building corrected final Stage2 ELF loader..."
nasm -f bin boot/corrected_final_stage2_elf_loader.asm -o retron-corrected-final-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Corrected final Stage2 ELF loader built successfully: retron-corrected-final-stage2-elf-loader.bin"
    ls -l retron-corrected-final-stage2-elf-loader.bin
else
    echo "Error building corrected final Stage2 ELF loader"
    exit 1
fi

