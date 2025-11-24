#!/bin/bash
# QEMUのmonitorでUARTメモリマッピングを確認するスクリプト（シンプル版）
# monitorをTCPポートで接続

echo "=========================================="
echo "QEMU UARTメモリマッピング確認（シンプル版）"
echo "=========================================="
echo ""

# QEMUを起動（monitorをTCPポートで接続）
QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M
KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
MONITOR_PORT=5555

echo "QEMUを起動中（monitor: TCP localhost:${MONITOR_PORT}）..."
echo ""
echo "別のターミナルで以下を実行してmonitorに接続してください："
echo ""
echo "  telnet localhost ${MONITOR_PORT}"
echo ""
echo "または、ncを使用："
echo ""
echo "  nc localhost ${MONITOR_PORT}"
echo ""
echo "monitorで以下のコマンドを実行："
echo ""
echo "  (qemu) info mtree"
echo "  (qemu) x/16x 0x09000000"
echo "  (qemu) info qtree"
echo "  (qemu) quit"
echo ""
echo "QEMUを停止するには、monitorで 'quit' を実行するか、Ctrl+Cを押してください。"
echo ""

${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -device loader,file=${KERNEL},addr=0x40200000 \
  -nographic \
  -serial stdio \
  -monitor tcp:localhost:${MONITOR_PORT},server,nowait \
  -s -S

