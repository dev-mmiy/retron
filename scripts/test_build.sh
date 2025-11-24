#!/bin/bash

# Retron OS ビルドテストスクリプト

echo "=== Retron OS ビルドテスト ==="

# カーネルのビルドテスト
echo "1. カーネルのビルドテスト..."
cd kernel
if cargo build --release; then
    echo "✅ カーネルビルド: 成功"
else
    echo "❌ カーネルビルド: 失敗"
    exit 1
fi

# バイナリファイルの存在確認
echo "2. バイナリファイルの確認..."
if [ -f "target/x86_64-unknown-none/release/retron-kernel" ]; then
    echo "✅ カーネルバイナリ: 存在"
    ls -la target/x86_64-unknown-none/release/retron-kernel
else
    echo "❌ カーネルバイナリ: 存在しない"
    exit 1
fi

# ファイルサイズの確認
echo "3. ファイルサイズの確認..."
SIZE=$(stat -c%s target/x86_64-unknown-none/release/retron-kernel)
echo "カーネルサイズ: $SIZE bytes"

if [ $SIZE -gt 0 ]; then
    echo "✅ ファイルサイズ: 正常"
else
    echo "❌ ファイルサイズ: 異常"
    exit 1
fi

echo "=== ビルドテスト完了 ==="
echo "Retron OSカーネルが正常にビルドされました！"


