#!/bin/bash
echo "Building Stage1 loader..."
nasm -f bin boot/stage1_loader.asm -o retron-stage1-loader.bin
if [ $? -eq 0 ]; then
    echo "Stage1 loader built successfully: retron-stage1-loader.bin"
    ls -l retron-stage1-loader.bin
else
    echo "Error building Stage1 loader"
    exit 1
fi
