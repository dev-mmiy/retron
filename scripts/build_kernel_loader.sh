#!/bin/bash

# カーネル読み込み用ブートローダーのビルドスクリプト

echo "=== Retron OS カーネルローダービルド ==="

# カーネルのビルド
echo "1. カーネルのビルド..."
cd kernel
cargo build --release

if [ $? -ne 0 ]; then
    echo "❌ カーネルビルド: 失敗"
    exit 1
fi

echo "✅ カーネルビルド: 成功"

# カーネルローダーのビルド
echo "2. カーネルローダーのビルド..."
cd ../boot
nasm -f bin kernel_loader.asm -o ../retron-kernel-loader.bin

if [ $? -ne 0 ]; then
    echo "❌ カーネルローダービルド: 失敗"
    exit 1
fi

echo "✅ カーネルローダービルド: 成功"

# ファイルサイズの確認
echo "3. ファイルサイズの確認..."
SIZE=$(stat -c%s ../retron-kernel-loader.bin)
echo "カーネルローダーサイズ: $SIZE bytes"

if [ $SIZE -eq 512 ]; then
    echo "✅ ファイルサイズ: 正常 (512 bytes)"
else
    echo "❌ ファイルサイズ: 異常 (期待値: 512 bytes, 実際: $SIZE bytes)"
    exit 1
fi

# カーネルファイルの確認
echo "4. カーネルファイルの確認..."
KERNEL_SIZE=$(stat -c%s ../kernel/target/x86_64-unknown-none/release/retron-kernel)
echo "カーネルサイズ: $KERNEL_SIZE bytes"

if [ $KERNEL_SIZE -gt 0 ]; then
    echo "✅ カーネルファイル: 存在"
else
    echo "❌ カーネルファイル: 存在しない"
    exit 1
fi

echo "=== カーネルローダービルド完了 ==="
echo "retron-kernel-loader.bin が生成されました！"
echo "QEMUで実行可能です。"


