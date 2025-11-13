#!/bin/bash
# QEMUプロセスを即座に停止するスクリプト

echo "QEMUプロセスを停止します..."
pkill -9 -f "qemu-system-aarch64"
sleep 1

if ps aux | grep -E "qemu-system-aarch64" | grep -v grep > /dev/null; then
    echo "⚠️  一部のQEMUプロセスが残っています"
    ps aux | grep -E "qemu-system-aarch64" | grep -v grep
else
    echo "✅ すべてのQEMUプロセスを停止しました"
fi

# ポート1234の確認
if netstat -tlnp 2>/dev/null | grep -q ":1234" || ss -tlnp 2>/dev/null | grep -q ":1234"; then
    echo "⚠️  ポート1234がまだ使用されています"
    netstat -tlnp 2>/dev/null | grep ":1234" || ss -tlnp 2>/dev/null | grep ":1234"
else
    echo "✅ ポート1234は使用可能です"
fi
