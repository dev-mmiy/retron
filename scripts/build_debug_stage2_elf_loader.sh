#!/bin/bash
echo "Building debug Stage2 ELF loader..."
nasm -f bin boot/debug_stage2_elf_loader.asm -o retron-debug-stage2-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Debug Stage2 ELF loader built successfully: retron-debug-stage2-elf-loader.bin"
    ls -l retron-debug-stage2-elf-loader.bin
else
    echo "Error building debug Stage2 ELF loader"
    exit 1
fi
