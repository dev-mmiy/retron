#!/bin/bash

# ハードディスク用ローダーのビルドスクリプト

echo "Building harddisk loader..."

# アセンブリファイルのコンパイル
nasm -f bin boot/harddisk_loader.asm -o retron-harddisk-loader.bin

if [ $? -eq 0 ]; then
    echo "Harddisk loader built successfully: retron-harddisk-loader.bin"
else
    echo "Error building harddisk loader"
    exit 1
fi
