#!/bin/bash
# QEMUシリアル出力テストスクリプト

QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M
KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys

echo "=========================================="
echo "QEMUシリアル出力テスト"
echo "=========================================="
echo ""
echo "このスクリプトは、QEMUのシリアル出力をテストします"
echo ""
echo "QEMUを起動します（5秒後に自動停止）..."
echo ""

timeout 5 ${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -device loader,file=${KERNEL},addr=0x40200000 \
  -nographic \
  -serial stdio \
  -monitor none \
  -s -S 2>&1 | head -20 || echo ""

echo ""
echo "=========================================="
echo "テスト完了"
echo "=========================================="
echo ""
echo "注意: カーネルが実行されていないため、出力は表示されません"
echo "      GDBで接続して実行を開始する必要があります"
