#!/bin/bash
# クイックポート修正スクリプト

echo "既存のQEMUプロセスを停止します..."
pkill -f "qemu-system-aarch64"
sleep 2

if netstat -tlnp 2>/dev/null | grep -q ":1234" || ss -tlnp 2>/dev/null | grep -q ":1234"; then
    echo "⚠️  ポート1234がまだ使用されています"
    echo "使用しているプロセス:"
    netstat -tlnp 2>/dev/null | grep ":1234" || ss -tlnp 2>/dev/null | grep ":1234"
else
    echo "✅ ポート1234は使用可能です"
    echo "QEMUを起動できます: ./run-tkernel-qemu.sh"
fi
