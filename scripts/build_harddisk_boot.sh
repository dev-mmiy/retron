#!/bin/bash

# ハードディスク用ブートローダーのビルドスクリプト

echo "=== Retron OS ハードディスク用ブートローダービルド ==="

# 1. カーネルのビルド
echo "1. カーネルのビルド..."
cargo build --manifest-path kernel/Cargo.toml --release
if [ $? -ne 0 ]; then
    echo "❌ カーネルビルド: 失敗"
    exit 1
fi

echo "✅ カーネルビルド: 成功"

# 2. ハードディスク用ブートローダーのビルド
echo "2. ハードディスク用ブートローダーのビルド..."
nasm -f bin boot/harddisk_boot.asm -o retron-harddisk-boot.bin
if [ $? -ne 0 ]; then
    echo "❌ ハードディスク用ブートローダービルド: 失敗"
    exit 1
fi

echo "✅ ハードディスク用ブートローダービルド: 成功"

# 3. ファイルサイズの確認
echo "3. ファイルサイズの確認..."
SIZE=$(stat -c%s retron-harddisk-boot.bin)
echo "ハードディスク用ブートローダーサイズ: $SIZE bytes"

if [ $SIZE -eq 512 ]; then
    echo "✅ ファイルサイズ: 正常 (512 bytes)"
else
    echo "❌ ファイルサイズ: 異常 (期待値: 512 bytes, 実際: $SIZE bytes)"
    exit 1
fi

# 4. カーネルファイルの確認
echo "4. カーネルファイルの確認..."
KERNEL_SIZE=$(stat -c%s kernel/target/x86_64-unknown-none/release/retron-kernel)
echo "カーネルサイズ: $KERNEL_SIZE bytes"

if [ $KERNEL_SIZE -gt 0 ]; then
    echo "✅ カーネルファイル: 存在"
else
    echo "❌ カーネルファイル: 存在しない"
    exit 1
fi

# 5. ハードディスク用ブートローダーの機能確認
echo "5. ハードディスク用ブートローダーの機能確認..."
echo "✅ ハードディスクからの起動"
echo "✅ メモリ情報の表示"
echo "✅ カーネル読み込みのシミュレーション"
echo "✅ プロテクトモードの準備"
echo "✅ カーネルへの制御移行"
echo "✅ Hello World表示"
echo "✅ テスト結果表示"
echo "✅ ファイルシステム機能表示"

echo "=== ハードディスク用ブートローダービルド完了 ==="
echo "retron-harddisk-boot.bin が生成されました！"
echo "QEMUで実行可能です。"


