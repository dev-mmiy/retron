#!/bin/bash

# Retron OS ビルドスクリプト

set -e

echo "Building Retron OS..."

# カーネルビルド
echo "Building kernel..."
cd kernel
cargo build --release
cd ..

# Coreレイヤービルド
echo "Building core layer..."
cd core
cargo build --release
cd ..

# UIレイヤービルド
echo "Building UI layer..."
cd ui
cargo build --release
cd ..

# Robotレイヤービルド
echo "Building robot layer..."
cd robot
cargo build --release
cd ..

echo "Build completed successfully!"
echo "Kernel binary: target/x86_64-unknown-none/release/retron-kernel"
echo "To run: make run-qemu"


