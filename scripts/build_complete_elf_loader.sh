#!/bin/bash
echo "Building complete ELF loader..."
nasm -f bin boot/complete_elf_loader.asm -o retron-complete-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Complete ELF loader built successfully: retron-complete-elf-loader.bin"
    ls -l retron-complete-elf-loader.bin
else
    echo "Error building complete ELF loader"
    exit 1
fi
