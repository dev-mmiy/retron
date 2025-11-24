#!/bin/bash

# QEMU用ローダーのビルドスクリプト

echo "Building QEMU loader..."

# アセンブリファイルのコンパイル
nasm -f bin boot/qemu_loader.asm -o retron-qemu-loader.bin

if [ $? -eq 0 ]; then
    echo "QEMU loader built successfully: retron-qemu-loader.bin"
else
    echo "Error building QEMU loader"
    exit 1
fi
