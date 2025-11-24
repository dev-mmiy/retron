#!/bin/bash

# Retron OS テストスクリプト

set -e

echo "Running Retron OS tests..."

# カーネルテスト
echo "Running kernel tests..."
cd kernel
cargo test
cd ..

# Coreレイヤーテスト
echo "Running core layer tests..."
cd core
cargo test
cd ..

# UIレイヤーテスト
echo "Running UI layer tests..."
cd ui
cargo test
cd ..

# Robotレイヤーテスト
echo "Running robot layer tests..."
cd robot
cargo test
cd ..

echo "All tests completed successfully!"


