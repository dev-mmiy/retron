#!/bin/bash

# 基本的なローダーのビルドスクリプト

echo "Building base loader..."

# アセンブリファイルのコンパイル
nasm -f bin boot/base_loader.asm -o retron-base-loader.bin

if [ $? -eq 0 ]; then
    echo "Base loader built successfully: retron-base-loader.bin"
else
    echo "Error building base loader"
    exit 1
fi
