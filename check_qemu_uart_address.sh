#!/bin/bash
# QEMU virt machineのUARTアドレスを確認するスクリプト

echo "=========================================="
echo "QEMU virt machine UARTアドレス確認"
echo "=========================================="
echo ""

echo "QEMU virt machineのデフォルト設定:"
echo "  - PL011 UART base address: 0x09000000"
echo ""

echo "確認方法:"
echo "  1. QEMUのドキュメントを確認"
echo "  2. QEMUのソースコードを確認"
echo "  3. デバイスツリーを確認"
echo ""

echo "代替案:"
echo "  - QEMUの-monitorオプションでメモリマッピングを確認"
echo "  - QEMUのデバッグオプションでUARTアクセスを確認"
