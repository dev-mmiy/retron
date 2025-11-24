#!/bin/bash
# クイックテスト: QEMUでカーネルを起動して短時間実行

QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M
KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys

echo "=========================================="
echo "T-Kernel Quick Test"
echo "=========================================="
echo ""
echo "Kernel: $KERNEL"
echo "Starting QEMU (5 seconds timeout)..."
echo ""

timeout 5 ${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -device loader,file=${KERNEL},addr=0x40200000 \
  -nographic \
  -serial stdio \
  -monitor none 2>&1 | head -30 || echo ""
echo ""
echo "Test completed."
