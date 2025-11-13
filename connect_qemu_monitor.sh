#!/bin/bash
# QEMUのmonitorに接続するスクリプト

echo "=========================================="
echo "QEMU Monitor接続"
echo "=========================================="
echo ""

if [ ! -S monitor.sock ]; then
    echo "❌ monitor.sockが見つかりません"
    echo "   ./run-qemu-with-monitor.shを先に起動してください"
    exit 1
fi

echo "QEMUのmonitorに接続します..."
echo ""
echo "使用可能なコマンド:"
echo "  info mtree          - メモリマッピングを表示"
echo "  x/16x 0x09000000   - 0x09000000のメモリを表示"
echo "  memwrite 0x09000000 0x48 4  - UARTDRに0x48を書き込み"
echo "  quit                - QEMUを終了"
echo ""

socat - unix:monitor.sock
