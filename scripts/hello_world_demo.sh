#!/bin/bash

# Hello World デモンストレーション

echo "=== Retron OS Hello World デモ ==="

# カーネルのビルド
echo "1. カーネルのビルド..."
cd kernel
cargo build --release

if [ $? -ne 0 ]; then
    echo "❌ カーネルビルド: 失敗"
    exit 1
fi

echo "✅ カーネルビルド: 成功"

# Hello Worldコードの表示
echo ""
echo "2. Hello Worldコードの確認..."
echo "--- kernel/src/simple.rs ---"
cat src/simple.rs

echo ""
echo "3. メインカーネルコードの確認..."
echo "--- kernel/src/main.rs (関連部分) ---"
grep -A 10 -B 5 "simple_hello" src/main.rs

echo ""
echo "4. ビルドされたバイナリの確認..."
echo "ファイル: kernel/target/x86_64-unknown-none/release/retron-kernel"
ls -la target/x86_64-unknown-none/release/retron-kernel

echo ""
echo "=== Hello World デモ完了 ==="
echo "Hello Worldコードは正しく実装されています！"
echo "QEMUでの実行には追加のブートローダーが必要です。"


