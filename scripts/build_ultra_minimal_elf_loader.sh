#!/bin/bash
echo "Building ultra minimal ELF loader..."
nasm -f bin boot/ultra_minimal_elf_loader.asm -o retron-ultra-minimal-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Ultra minimal ELF loader built successfully: retron-ultra-minimal-elf-loader.bin"
    ls -l retron-ultra-minimal-elf-loader.bin
else
    echo "Error building ultra minimal ELF loader"
    exit 1
fi
