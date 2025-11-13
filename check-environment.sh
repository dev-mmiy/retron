#!/bin/bash

echo "=== Development Environment Check ==="

echo -n "QEMU: "
qemu-system-aarch64 --version | head -1 || echo "NOT FOUND"

echo -n "Rust: "
source $HOME/.cargo/env 2>/dev/null && rustc --version || echo "NOT FOUND"

echo -n "Rust target (aarch64-unknown-none): "
source $HOME/.cargo/env 2>/dev/null && rustup target list --installed | grep aarch64-unknown-none || echo "NOT INSTALLED"

if command -v aarch64-linux-gnu-gcc &> /dev/null; then
    echo -n "GCC (aarch64-linux-gnu): "
    aarch64-linux-gnu-gcc --version | head -1 || echo "NOT FOUND"
elif command -v aarch64-elf-gcc &> /dev/null; then
    echo -n "GCC (aarch64-elf): "
    aarch64-elf-gcc --version | head -1 || echo "NOT FOUND"
else
    echo "GCC: NOT FOUND"
fi

if command -v gdb-multiarch &> /dev/null; then
    echo -n "GDB: "
    gdb-multiarch --version | head -1 || echo "NOT FOUND"
elif command -v gdb &> /dev/null; then
    echo -n "GDB: "
    gdb --version | head -1 || echo "NOT FOUND"
else
    echo "GDB: NOT FOUND"
fi

echo "=== Check Complete ==="
