#!/bin/bash

# QEMU起動パラメータ（GDBサーバー有効）
QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=1G
DISK=retron.img
NETDEV=user,id=net0
NETDEVICE=virtio-net-device,netdev=net0

# QEMU起動（GDBサーバー有効）
${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -drive file=${DISK},format=raw \
  -netdev ${NETDEV} \
  -device ${NETDEVICE} \
  -nographic \
  -serial stdio \
  -s -S
