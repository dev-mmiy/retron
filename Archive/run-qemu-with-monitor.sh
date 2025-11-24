#!/bin/bash

# QEMU起動パラメータ（monitor付き）
QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M
KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys

# QEMU起動（monitorを有効化）
# -monitor unix:monitor.sock: monitorをUnixソケットに接続
# -serial stdio: シリアル出力を標準入出力に接続
${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -device loader,file=${KERNEL},addr=0x40200000 \
  -nographic \
  -serial stdio \
  -monitor unix:monitor.sock,server,nowait \
  -s -S
