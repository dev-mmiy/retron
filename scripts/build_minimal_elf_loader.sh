#!/bin/bash
echo "Building minimal ELF loader..."
nasm -f bin boot/minimal_elf_loader.asm -o retron-minimal-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Minimal ELF loader built successfully: retron-minimal-elf-loader.bin"
    ls -l retron-minimal-elf-loader.bin
else
    echo "Error building minimal ELF loader"
    exit 1
fi
