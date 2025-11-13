#!/bin/bash
# カーネル検証スクリプト

KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys

echo "=========================================="
echo "カーネル検証"
echo "=========================================="
echo ""

echo "1. ELFファイル形式の確認"
file "$KERNEL"
echo ""

echo "2. セクション情報"
aarch64-linux-gnu-readelf -S "$KERNEL" 2>/dev/null | grep -E "(\.text|\.data|\.bss|\.rodata)" | head -5
echo ""

echo "3. プログラムヘッダー（ロードセグメント）"
aarch64-linux-gnu-readelf -l "$KERNEL" 2>/dev/null | grep -A 3 "LOAD" | head -8
echo ""

echo "4. シンボルテーブル（主要関数）"
echo "   エントリーポイント:"
aarch64-linux-gnu-nm "$KERNEL" 2>/dev/null | grep -E " (T|t) start$" | head -1
echo "   初期化関数:"
aarch64-linux-gnu-nm "$KERNEL" 2>/dev/null | grep -E " (T|t) (main|init_device|uart_pl011_init)$" | head -3
echo ""

echo "5. 依存関係（動的ライブラリ）"
aarch64-linux-gnu-readelf -d "$KERNEL" 2>/dev/null | grep -E "(NEEDED|DYNAMIC)" || echo "   静的リンク（依存ライブラリなし）"
echo ""

echo "=========================================="
echo "検証完了"
echo "=========================================="
