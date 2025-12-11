#!/bin/bash
echo "Building efficient ELF loader..."
nasm -f bin boot/efficient_elf_loader.asm -o retron-efficient-elf-loader.bin
if [ $? -eq 0 ]; then
    echo "Efficient ELF loader built successfully: retron-efficient-elf-loader.bin"
    ls -l retron-efficient-elf-loader.bin
else
    echo "Error building efficient ELF loader"
    exit 1
fi
