#!/bin/bash

# QEMU起動パラメータ（T-Kernel kernel-ram.sys用）
QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M
KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys

# QEMU起動（kernel-ram.sysを直接ロード）
# 注意: -kernelオプションはLinuxカーネル用でDTBを自動ロードします
# T-KernelはDTBを必要としないため、-device loaderオプションを使用
# -s -S: GDBサーバーを有効化（デバッグ用）
${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -device loader,file=${KERNEL},addr=0x40200000 \
  -nographic \
  -serial stdio \
  -monitor none \
  -s -S

