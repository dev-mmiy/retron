#!/bin/bash

# QEMU起動パラメータ（monitorを別ターミナルで使用）
QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M
KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys

# QEMU起動（monitorをTCPポートで有効化）
# -monitor tcp:127.0.0.1:5555,server,nowait: monitorをTCPポート5555で有効化
# -serial stdio: シリアル出力を標準入出力に接続
${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -device loader,file=${KERNEL},addr=0x40200000 \
  -nographic \
  -serial stdio \
  -monitor tcp:127.0.0.1:5555,server,nowait \
  -s -S
