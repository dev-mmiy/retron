#!/bin/bash
echo "Building ELF loader..."
nasm -f bin boot/elf_loader.asm -o retron-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "ELF loader built successfully: retron-elf-loader.bin"
    ls -l retron-elf-loader.bin
else
    echo "Error building ELF loader"
    exit 1
fi
