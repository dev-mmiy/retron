#!/bin/bash
# QEMUのmonitorでUART情報を取得するスクリプト
# monitorがTCPポート5555で起動していることを前提とする

MONITOR_PORT=5555

echo "=========================================="
echo "QEMU UART情報取得"
echo "=========================================="
echo ""

if ! nc -z localhost ${MONITOR_PORT} 2>/dev/null; then
    echo "エラー: QEMUのmonitorが起動していません。"
    echo "まず、./check_uart_simple.sh を実行してQEMUを起動してください。"
    exit 1
fi

echo "QEMU monitorに接続中..."
echo ""

echo "=== メモリマッピング情報 ==="
echo "info mtree" | nc localhost ${MONITOR_PORT} | head -50
echo ""

echo "=== UARTアドレス（0x09000000）のメモリ内容 ==="
echo "x/16x 0x09000000" | nc localhost ${MONITOR_PORT}
echo ""

echo "=== デバイスツリー情報（UART関連） ==="
echo "info qtree" | nc localhost ${MONITOR_PORT} | grep -i -A 5 -B 5 uart
echo ""

echo "=== デバイスツリー情報（PL011関連） ==="
echo "info qtree" | nc localhost ${MONITOR_PORT} | grep -i -A 5 -B 5 pl011
echo ""

echo "完了しました。"

