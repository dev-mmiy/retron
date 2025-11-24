#!/bin/bash
# QEMUとGDBを再起動するスクリプト

echo "=========================================="
echo "QEMUとGDBの再起動"
echo "=========================================="
echo ""

# 1. 既存のプロセスを停止
echo "1. 既存のプロセスを停止..."
pkill -9 -f "qemu-system-aarch64"
pkill -9 -f "gdb"
sleep 2

# 2. ポート確認
echo ""
echo "2. ポート1234の確認..."
if netstat -tlnp 2>/dev/null | grep -q ":1234" || ss -tlnp 2>/dev/null | grep -q ":1234"; then
    echo "   ⚠️  ポート1234がまだ使用されています"
    netstat -tlnp 2>/dev/null | grep ":1234" || ss -tlnp 2>/dev/null | grep ":1234"
else
    echo "   ✅ ポート1234は使用可能です"
fi

echo ""
echo "=========================================="
echo "次のステップ:"
echo "1. ターミナル1で: ./run-tkernel-qemu.sh"
echo "2. ターミナル2で: ./debug-kernel.sh"
echo "=========================================="
