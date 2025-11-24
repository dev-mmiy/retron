#!/bin/bash
echo "Building Stage2 ELF loader..."
nasm -f bin boot/stage2_elf_loader.asm -o retron-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Stage2 ELF loader built successfully: retron-stage2-elf-loader.bin"
    ls -l retron-stage2-elf-loader.bin
else
    echo "Error building Stage2 ELF loader"
    exit 1
fi
