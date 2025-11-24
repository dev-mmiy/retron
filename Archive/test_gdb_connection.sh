#!/bin/bash
# GDB接続テスト

KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
GDB_CMD=aarch64-linux-gnu-gdb

if ! command -v "$GDB_CMD" >/dev/null 2>&1; then
    GDB_CMD=gdb-multiarch
fi

if ! command -v "$GDB_CMD" >/dev/null 2>&1; then
    echo "❌ GDBがインストールされていません"
    echo "   インストール方法: sudo apt-get install gdb-multiarch"
    exit 1
fi

echo "=========================================="
echo "GDB接続テスト"
echo "=========================================="
echo ""
echo "使用するGDB: $GDB_CMD"
echo "カーネル: $KERNEL"
echo ""

# GDBコマンドファイルを作成
cat > /tmp/gdb_test_commands.txt << 'GDBEOF'
# GDB test commands
set confirm off
file third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
target remote :1234
info registers pc
x/10i $pc
quit
GDBEOF

echo "GDBコマンドファイルを作成しました: /tmp/gdb_test_commands.txt"
echo ""
echo "注意: QEMUがポート1234でGDBサーバーを起動している必要があります"
echo "      QEMUを起動するには: ./run-tkernel-qemu.sh"
echo ""
echo "GDBを起動するには:"
echo "  $GDB_CMD -x /tmp/gdb_test_commands.txt $KERNEL"
echo ""
