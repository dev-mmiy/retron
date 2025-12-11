#!/bin/bash

# 最小限のローダーのビルドスクリプト

echo "Building minimal loader..."

# アセンブリファイルのコンパイル
nasm -f bin boot/minimal_loader.asm -o retron-minimal-loader.bin

if [ $? -eq 0 ]; then
    echo "Minimal loader built successfully: retron-minimal-loader.bin"
else
    echo "Error building minimal loader"
    exit 1
fi
