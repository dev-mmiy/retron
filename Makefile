# Retron OS Makefile

.PHONY: all clean build run-qemu test

# デフォルトターゲット
all: build

# カーネルビルド
build:
	@echo "Building Retron OS..."
	cargo build --manifest-path kernel/Cargo.toml --release
	@echo "Build completed!"

# クリーンアップ
clean:
	@echo "Cleaning build artifacts..."
	cargo clean --manifest-path kernel/Cargo.toml
	rm -rf target/
	@echo "Clean completed!"

# QEMUでの実行
run-qemu: build
	@echo "Starting Retron OS in QEMU..."
	qemu-system-x86_64 \
		-machine q35 \
		-cpu qemu64 \
		-m 512M \
		-kernel kernel/target/x86_64-unknown-none/release/retron-kernel \
		-nographic \
		-no-reboot \
		-accel tcg

# シンプルなQEMU実行（デバッグ用）
run-simple: build
	@echo "Starting Retron OS in QEMU (Simple mode)..."
	qemu-system-x86_64 \
		-machine pc \
		-cpu qemu64 \
		-m 128M \
		-kernel kernel/target/x86_64-unknown-none/release/retron-kernel \
		-nographic \
		-no-reboot

# ブートローダーでの実行
run-bootloader:
	@echo "Starting Retron OS with Bootloader..."
	qemu-system-x86_64 \
		-drive file=retron-boot.bin,format=raw,if=floppy \
		-nographic \
		-no-reboot

# カーネルローダーでの実行
run-kernel-loader:
	@echo "Starting Retron OS with Kernel Loader..."
	qemu-system-x86_64 \
		-drive file=retron-kernel-loader.bin,format=raw,if=floppy \
		-nographic \
		-no-reboot

# 高度なカーネルローダーでの実行
run-advanced-loader:
	@echo "Starting Retron OS with Advanced Kernel Loader..."
	qemu-system-x86_64 \
		-drive file=retron-advanced-loader.bin,format=raw,if=floppy \
		-nographic \
		-no-reboot

# 実際のカーネル実行
run-real-kernel:
	@echo "Starting Retron OS with Real Kernel Execution..."
	qemu-system-x86_64 \
		-drive file=retron-real-kernel-loader.bin,format=raw,if=floppy \
		-nographic \
		-no-reboot

# ハードディスク用ブートローダーでの実行
run-harddisk:
	@echo "Starting Retron OS with Harddisk Bootloader..."
	qemu-system-x86_64 \
		-drive file=retron-harddisk-boot.bin,format=raw,if=floppy \
		-nographic \
		-no-reboot \
		-serial stdio \
		-monitor none \
		-m 512M \
		-cpu qemu64 \
		-machine q35 \
		-accel tcg

# テスト実行
test:
	@echo "Running tests..."
	cargo test --manifest-path kernel/Cargo.toml

# 開発用デバッグビルド
debug:
	cargo build --manifest-path kernel/Cargo.toml

# ヘルプ
help:
	@echo "Available targets:"
	@echo "  build     - Build the kernel (release)"
	@echo "  debug     - Build the kernel (debug)"
	@echo "  run-qemu  - Run in QEMU"
	@echo "  test      - Run tests"
	@echo "  clean     - Clean build artifacts"
	@echo "  help      - Show this help"
