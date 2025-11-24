#!/bin/bash
# QEMUのmonitorでUARTメモリマッピングを確認するスクリプト

echo "=========================================="
echo "QEMU UARTメモリマッピング確認"
echo "=========================================="
echo ""

# QEMUを起動（monitorをunixソケットに接続）
QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M
KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys
MONITOR_SOCK=monitor.sock

# 既存のソケットファイルを削除
rm -f ${MONITOR_SOCK}

echo "QEMUを起動中（monitor: unix:${MONITOR_SOCK}）..."
echo ""
echo "別のターミナルで以下を実行してmonitorに接続してください："
echo ""
echo "  socat unix-connect:${MONITOR_SOCK} -"
echo ""
echo "または、QEMU monitorコマンドを直接実行："
echo ""
echo "  echo 'info mtree' | socat unix-connect:${MONITOR_SOCK} -"
echo "  echo 'x/16x 0x09000000' | socat unix-connect:${MONITOR_SOCK} -"
echo "  echo 'info qtree' | socat unix-connect:${MONITOR_SOCK} - | grep -i uart"
echo "  echo 'info qtree' | socat unix-connect:${MONITOR_SOCK} - | grep -i pl011"
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
  -monitor unix:${MONITOR_SOCK},server,nowait \
  -s -S

