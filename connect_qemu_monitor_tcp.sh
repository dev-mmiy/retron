#!/bin/bash
# QEMUのmonitorにTCPで接続するスクリプト

echo "=========================================="
echo "QEMU Monitor接続（TCP）"
echo "=========================================="
echo ""

echo "QEMUのmonitorに接続します（telnet 127.0.0.1 5555）..."
echo ""
echo "使用可能なコマンド:"
echo "  info mtree          - メモリマッピングを表示"
echo "  x/16x 0x09000000   - 0x09000000のメモリを表示"
echo "  memwrite 0x09000000 0x48 4  - UARTDRに0x48を書き込み"
echo "  quit                - QEMUを終了"
echo ""

telnet 127.0.0.1 5555
