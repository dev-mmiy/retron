#!/bin/bash

# Multiboot対応カーネルのビルドスクリプト

echo "=== Retron OS Multibootビルド ==="

# カーネルのビルド
echo "1. カーネルのビルド..."
cd kernel
cargo build --release

if [ $? -ne 0 ]; then
    echo "❌ カーネルビルド: 失敗"
    exit 1
fi

echo "✅ カーネルビルド: 成功"

# オブジェクトファイルの作成
echo "2. オブジェクトファイルの作成..."
cd ../boot
nasm -f elf32 multiboot.asm -o multiboot.o

if [ $? -ne 0 ]; then
    echo "❌ アセンブリ: 失敗"
    exit 1
fi

echo "✅ アセンブリ: 成功"

# リンク
echo "3. リンク..."
ld -m elf_i386 -T ../kernel/src/linker.ld \
   multiboot.o \
   ../kernel/target/x86_64-unknown-none/release/libretron_kernel.a \
   -o ../retron-kernel-multiboot

if [ $? -ne 0 ]; then
    echo "❌ リンク: 失敗"
    exit 1
fi

echo "✅ リンク: 成功"

# ファイルサイズの確認
echo "4. ファイルサイズの確認..."
SIZE=$(stat -c%s ../retron-kernel-multiboot)
echo "カーネルサイズ: $SIZE bytes"

if [ $SIZE -gt 0 ]; then
    echo "✅ ファイルサイズ: 正常"
else
    echo "❌ ファイルサイズ: 異常"
    exit 1
fi

echo "=== Multibootビルド完了 ==="
echo "retron-kernel-multiboot が生成されました！"


