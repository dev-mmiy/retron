#!/bin/bash

# QEMU起動パラメータ（T-Kernel kernel-ram.sys用、GDBなし）
QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=256M
KERNEL=third_party/tkernel_2/kernel/sysmain/build/retron_aarch64/kernel-ram.sys

# QEMU起動（kernel-ram.sysを直接ロード）
# 注意: -kernelオプションはLinuxカーネル用でDTBを自動ロードします
# T-KernelはDTBを必要としないため、-device loaderオプションを使用
# force-raw=on: ELFファイルを生のバイナリとして扱う
${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -device loader,file=${KERNEL},addr=0x40200000,force-raw=on \
  -nographic \
  -serial stdio \
  -monitor none

