#!/bin/bash
# 自動化された動作確認テスト

KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
QEMU=qemu-system-aarch64
GDB_CMD=aarch64-linux-gnu-gdb

if ! command -v "$GDB_CMD" >/dev/null 2>&1; then
    GDB_CMD=gdb-multiarch
fi

echo "=========================================="
echo "自動化された動作確認テスト"
echo "=========================================="
echo ""

# 1. カーネルファイルの確認
echo "1. カーネルファイルの確認"
if [ -f "$KERNEL" ]; then
    echo "   ✅ カーネルファイル存在"
    SIZE=$(ls -lh "$KERNEL" | awk '{print $5}')
    echo "   サイズ: $SIZE"
else
    echo "   ❌ カーネルファイルが見つかりません"
    exit 1
fi

# 2. QEMUの確認
echo ""
echo "2. QEMUの確認"
if command -v "$QEMU" >/dev/null 2>&1; then
    echo "   ✅ QEMUがインストールされています"
    VERSION=$("$QEMU" --version | head -1)
    echo "   $VERSION"
else
    echo "   ❌ QEMUがインストールされていません"
    exit 1
fi

# 3. GDBの確認
echo ""
echo "3. GDBの確認"
if command -v "$GDB_CMD" >/dev/null 2>&1; then
    echo "   ✅ GDBがインストールされています"
    echo "   使用するGDB: $GDB_CMD"
else
    echo "   ⚠️  GDBがインストールされていません"
    echo "   インストール方法: sudo apt-get install gdb-multiarch"
fi

# 4. エントリーポイントの確認
echo ""
echo "4. エントリーポイントの確認"
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

# 5. シンボルの確認
echo ""
echo "5. シンボルの確認"
SYMBOLS=$(aarch64-linux-gnu-nm "$KERNEL" 2>/dev/null | grep -E " (T|t) (start|main|uart_pl011_init|uart_pl011_putchar)$" | wc -l)
if [ "$SYMBOLS" -ge 4 ]; then
    echo "   ✅ 主要シンボルが存在します ($SYMBOLS個)"
else
    echo "   ⚠️  一部のシンボルが見つかりません"
fi

echo ""
echo "=========================================="
echo "テスト完了"
echo "=========================================="
echo ""
echo "次のステップ:"
echo "1. QEMUを起動: ./run-tkernel-qemu.sh"
echo "2. GDBで接続: ./debug-kernel.sh"
