#!/bin/bash

# シンプルなハードディスク用ローダーのビルドスクリプト

echo "Building simple harddisk loader..."

# アセンブリファイルのコンパイル
nasm -f bin boot/simple_harddisk_loader.asm -o retron-simple-harddisk-loader.bin

if [ $? -eq 0 ]; then
    echo "Simple harddisk loader built successfully: retron-simple-harddisk-loader.bin"
else
    echo "Error building simple harddisk loader"
    exit 1
fi
