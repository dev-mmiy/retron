#!/bin/bash
set -e

# ターゲットアーキテクチャ
TARGET="aarch64-unknown-none"

# ビルドディレクトリ
BUILD_DIR="build"

echo "Building ReTron OS for ${TARGET}..."

# Rustプロジェクトのビルド
cargo build --target ${TARGET} --release

# T-Kernelのビルド（必要に応じて）
# cd third_party/t-kernel
# make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu-

echo "Build completed!"
