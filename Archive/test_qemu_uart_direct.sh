#!/bin/bash
# QEMUのUARTを直接テストするスクリプト

QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M

echo "=========================================="
echo "QEMU UART直接テスト"
echo "=========================================="
echo ""

echo "QEMUを起動して、UARTのメモリマッピングを確認します"
echo ""

# 最小限のカーネルイメージを作成（テスト用）
# 実際には、既存のkernel-ram.sysを使用

echo "QEMU起動コマンド:"
echo "${QEMU} \\"
echo "  -machine ${MACHINE} \\"
echo "  -cpu ${CPU} \\"
echo "  -m ${MEMORY} \\"
echo "  -nographic \\"
echo "  -serial stdio \\"
echo "  -monitor stdio"
echo ""

echo "QEMUのmonitorで以下を実行:"
echo "  (qemu) info mtree"
echo "  (qemu) x/16x 0x09000000"
echo ""
