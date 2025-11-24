#!/bin/bash

# 生バイナリ形式でカーネルをビルドするスクリプト

echo "Building raw binary kernel..."

# カーネルディレクトリに移動
cd kernel

# カーネルをビルド
cargo build --release --target x86_64-unknown-none

if [ $? -ne 0 ]; then
    echo "Error building kernel"
    exit 1
fi

# ELFファイルから生バイナリを抽出
objcopy -O binary target/x86_64-unknown-none/release/retron-kernel ../retron-kernel-raw.bin

if [ $? -eq 0 ]; then
    echo "Raw binary kernel created: retron-kernel-raw.bin"
    echo "Size: $(wc -c < ../retron-kernel-raw.bin) bytes"
else
    echo "Error creating raw binary"
    exit 1
fi

cd ..
