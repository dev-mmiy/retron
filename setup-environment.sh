#!/bin/bash
# ReTron OS 開発環境セットアップスクリプト
# Windows 11 + WSL2 (Ubuntu 20.04) 用

set -e

echo "=== ReTron OS 開発環境セットアップ ==="
echo ""

# パッケージリストの更新
echo "1. パッケージリストを更新中..."
sudo apt-get update

# QEMUのインストール
echo ""
echo "2. QEMUをインストール中..."
sudo apt-get install -y qemu-system-aarch64 qemu-utils

# クロスコンパイルツールチェーンのインストール
echo ""
echo "3. クロスコンパイルツールチェーンをインストール中..."
sudo apt-get install -y gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu

# GDBのインストール
echo ""
echo "4. GDB (multiarch)をインストール中..."
sudo apt-get install -y gdb-multiarch

# Rust環境の確認
echo ""
echo "5. Rust環境を確認中..."
if [ -f "$HOME/.cargo/env" ]; then
    source "$HOME/.cargo/env"
    echo "   Rust: $(rustc --version)"
    echo "   ターゲット: $(rustup target list --installed | grep aarch64-unknown-none || echo '未インストール')"
else
    echo "   Rustがインストールされていません。"
    echo "   以下のコマンドでインストールしてください:"
    echo "   curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh"
fi

echo ""
echo "=== セットアップ完了 ==="
echo ""
echo "環境確認スクリプトを実行しますか？ (y/n)"
read -r response
if [ "$response" = "y" ] || [ "$response" = "Y" ]; then
    ./check-environment.sh
fi




