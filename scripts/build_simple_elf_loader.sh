#!/bin/bash
echo "Building simple ELF loader..."
nasm -f bin boot/simple_elf_loader.asm -o retron-simple-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Simple ELF loader built successfully: retron-simple-elf-loader.bin"
    ls -l retron-simple-elf-loader.bin
else
    echo "Error building simple ELF loader"
    exit 1
fi
