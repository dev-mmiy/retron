#!/bin/bash

# シンプルなブートローダーのビルドスクリプト

echo "=== Retron OS シンプルビルド ==="

# アセンブリのビルド
echo "1. ブートローダーのビルド..."
cd boot
nasm -f bin simple_boot.asm -o ../retron-boot.bin

if [ $? -ne 0 ]; then
    echo "❌ ブートローダービルド: 失敗"
    exit 1
fi

echo "✅ ブートローダービルド: 成功"

# ファイルサイズの確認
echo "2. ファイルサイズの確認..."
SIZE=$(stat -c%s ../retron-boot.bin)
echo "ブートローダーサイズ: $SIZE bytes"

if [ $SIZE -eq 512 ]; then
    echo "✅ ファイルサイズ: 正常 (512 bytes)"
else
    echo "❌ ファイルサイズ: 異常 (期待値: 512 bytes, 実際: $SIZE bytes)"
    exit 1
fi

echo "=== シンプルビルド完了 ==="
echo "retron-boot.bin が生成されました！"
echo "QEMUで実行可能です。"


