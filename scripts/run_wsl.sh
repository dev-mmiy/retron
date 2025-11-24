#!/bin/bash

echo "=== Retron OS WSL環境用実行スクリプト ==="

# WSL環境の確認
echo "WSL環境の確認..."
if grep -q Microsoft /proc/version; then
    echo "✅ WSL環境を検出"
else
    echo "⚠️  WSL環境ではない可能性があります"
fi

# 必要なパッケージの確認
echo "必要なパッケージの確認..."
if command -v qemu-system-x86_64 &> /dev/null; then
    echo "✅ QEMU: インストール済み"
else
    echo "❌ QEMU: 未インストール"
    echo "sudo apt update && sudo apt install -y qemu-system-x86 を実行してください"
    exit 1
fi

if command -v nasm &> /dev/null; then
    echo "✅ NASM: インストール済み"
else
    echo "❌ NASM: 未インストール"
    echo "sudo apt install -y nasm を実行してください"
    exit 1
fi

if command -v cargo &> /dev/null; then
    echo "✅ Rust: インストール済み"
else
    echo "❌ Rust: 未インストール"
    echo "curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh を実行してください"
    exit 1
fi

# カーネルのビルド
echo "カーネルのビルド..."
cargo build --manifest-path kernel/Cargo.toml --release
if [ $? -eq 0 ]; then
    echo "✅ カーネルビルド: 成功"
else
    echo "❌ カーネルビルド: 失敗"
    exit 1
fi

# ハードディスク用ブートローダーのビルド
echo "ハードディスク用ブートローダーのビルド..."
nasm -f bin boot/harddisk_boot.asm -o retron-harddisk-boot.bin
if [ $? -eq 0 ]; then
    echo "✅ ハードディスク用ブートローダービルド: 成功"
else
    echo "❌ ハードディスク用ブートローダービルド: 失敗"
    exit 1
fi

# WSL環境用のQEMU実行
echo "WSL環境用のQEMU実行..."
echo "注意: Ctrl+C で終了できます"
echo ""

# 実行中のプロセスを確認
if pgrep -f qemu-system-x86_64 > /dev/null; then
    echo "既存のQEMUプロセスを終了しています..."
    pkill -f qemu-system-x86_64
    sleep 2
fi

# WSL環境用のQEMU実行オプション
qemu-system-x86_64 \
    -drive file=retron-harddisk-boot.bin,format=raw,if=floppy \
    -nographic \
    -no-reboot \
    -serial stdio \
    -monitor none \
    -m 512M \
    -cpu qemu64 \
    -machine q35 \
    -accel tcg \
    -d guest_errors \
    -D qemu_debug.log

echo ""
echo "=== 実行完了 ==="
echo "デバッグログ: qemu_debug.log"


