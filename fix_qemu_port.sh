#!/bin/bash
# QEMUポート問題の修正スクリプト

echo "=========================================="
echo "QEMUポート問題の修正"
echo "=========================================="
echo ""

# 1. 既存のQEMUプロセスを確認
echo "1. 既存のQEMUプロセスの確認"
QEMU_PIDS=$(ps aux | grep -E "qemu-system-aarch64" | grep -v grep | awk '{print $2}')
if [ -n "$QEMU_PIDS" ]; then
    echo "   見つかったQEMUプロセス: $QEMU_PIDS"
    echo "   停止します..."
    pkill -f "qemu-system-aarch64"
    sleep 2
    echo "   ✅ QEMUプロセスを停止しました"
else
    echo "   ✅ 実行中のQEMUプロセスはありません"
fi

# 2. ポート1234の使用状況を確認
echo ""
echo "2. ポート1234の使用状況を確認"
PORT_INFO=$(netstat -tlnp 2>/dev/null | grep ":1234" || ss -tlnp 2>/dev/null | grep ":1234" || lsof -i :1234 2>/dev/null)
if [ -n "$PORT_INFO" ]; then
    echo "   ⚠️  ポート1234が使用されています:"
    echo "$PORT_INFO"
    echo ""
    echo "   ポート1234を使用しているプロセスを停止しますか？ (y/n)"
    read -r answer
    if [ "$answer" = "y" ]; then
        PID=$(echo "$PORT_INFO" | grep -oP '\d+(?=/)' | head -1)
        if [ -n "$PID" ]; then
            kill "$PID" 2>/dev/null
            sleep 1
            echo "   ✅ プロセス $PID を停止しました"
        fi
    fi
else
    echo "   ✅ ポート1234は使用可能です"
fi

# 3. 確認
echo ""
echo "3. 最終確認"
sleep 1
FINAL_CHECK=$(netstat -tlnp 2>/dev/null | grep ":1234" || ss -tlnp 2>/dev/null | grep ":1234")
if [ -z "$FINAL_CHECK" ]; then
    echo "   ✅ ポート1234は使用可能です"
    echo ""
    echo "   QEMUを起動できます: ./run-tkernel-qemu.sh"
else
    echo "   ⚠️  ポート1234がまだ使用されています"
    echo "   手動でプロセスを停止してください"
fi

echo ""
echo "=========================================="
