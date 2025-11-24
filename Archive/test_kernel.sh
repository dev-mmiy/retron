#!/bin/bash
# カーネル動作確認テスト

KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
QEMU=qemu-system-aarch64

echo "=========================================="
echo "T-Kernel 動作確認テスト"
echo "=========================================="
echo ""
echo "1. カーネルファイルの確認"
if [ -f "$KERNEL" ]; then
    echo "   ✅ カーネルファイル存在: $KERNEL"
    ls -lh "$KERNEL" | awk '{print "   サイズ: " $5}'
else
    echo "   ❌ カーネルファイルが見つかりません"
    exit 1
fi

echo ""
echo "2. エントリーポイントの確認"
ENTRY=$(aarch64-linux-gnu-readelf -h "$KERNEL" 2>/dev/null | grep "Entry point" | awk '{print $4}')
if [ -n "$ENTRY" ]; then
    echo "   ✅ エントリーポイント: $ENTRY"
    if [ "$ENTRY" = "0x40200000" ]; then
        echo "   ✅ エントリーポイントが正しい"
    else
        echo "   ⚠️  エントリーポイントが期待値と異なります"
    fi
else
    echo "   ❌ エントリーポイントを取得できませんでした"
fi

echo ""
echo "3. シンボルの確認"
SYMBOLS=$(aarch64-linux-gnu-nm "$KERNEL" 2>/dev/null | grep -E " (T|t) (start|main|uart_pl011_init|uart_pl011_putchar)$" | wc -l)
if [ "$SYMBOLS" -ge 4 ]; then
    echo "   ✅ 主要シンボルが存在します ($SYMBOLS個)"
    aarch64-linux-gnu-nm "$KERNEL" 2>/dev/null | grep -E " (T|t) (start|main|uart_pl011_init|uart_pl011_putchar)$" | head -4
else
    echo "   ⚠️  一部のシンボルが見つかりません"
fi

echo ""
echo "4. QEMUの確認"
if command -v "$QEMU" >/dev/null 2>&1; then
    echo "   ✅ QEMUがインストールされています"
    "$QEMU" --version | head -1
else
    echo "   ❌ QEMUがインストールされていません"
fi

echo ""
echo "=========================================="
echo "テスト完了"
echo "=========================================="
