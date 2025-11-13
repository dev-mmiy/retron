# ReTron OS - 開発環境構築ガイド

> **ドキュメント種別**: 開発環境構築ガイド  
> **対象読者**: 開発者  
> **関連ドキュメント**: [REQUIREMENTS_SPEC.md](REQUIREMENTS_SPEC.md), [ARCHITECTURE.md](ARCHITECTURE.md), [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md)

## ドキュメント情報

| 項目 | 内容 |
|------|------|
| **ドキュメント名** | ReTron OS 開発環境構築ガイド |
| **バージョン** | 1.0 |
| **作成日** | 2024年 |
| **最終更新日** | 2024年 |
| **作成者** | - |
| **承認者** | - |
| **ステータス** | 草案 |

---

## 概要

このドキュメントでは、ReTron OSの開発環境を構築する手順を説明します。

**ターゲットプラットフォーム**: ARM Generic AArch64（QEMU `virt` machine）

---

## 1. 必要なソフトウェア

### 1.1 必須ソフトウェア

以下のソフトウェアが必要です：

| ソフトウェア | バージョン | 用途 |
|------------|-----------|------|
| **QEMU** | 6.0以上 | 仮想マシンエミュレータ |
| **Rust** | 1.70以上 | Rustコンパイラ |
| **GCC** | 10.0以上 | C/C++コンパイラ（クロスコンパイル用） |
| **binutils** | 2.35以上 | バイナリユーティリティ（クロスコンパイル用） |
| **GDB** | 10.0以上 | デバッガ |
| **make** | 4.0以上 | ビルドシステム |
| **git** | 2.30以上 | バージョン管理 |

### 1.2 オプションソフトウェア

| ソフトウェア | バージョン | 用途 |
|------------|-----------|------|
| **ninja** | 1.10以上 | 高速ビルドシステム（オプション） |
| **cmake** | 3.20以上 | CMakeビルドシステム（オプション） |

---

## 2. 開発環境のセットアップ

### 2.1 前提条件

**対応OS**:
- **Windows 11 + WSL2**（推奨: Ubuntu 20.04以上、Debian 11以上）
- **macOS**（推奨: macOS 12以上）
- **Linux**（推奨: Ubuntu 20.04以上、Debian 11以上）

**システム要件**:
- **メモリ**: 8GB以上（推奨: 16GB以上）
- **ストレージ**: 20GB以上の空き容量
- **ネットワーク**: インターネット接続（パッケージダウンロード用）

---

## 2A. Windows 11 + WSL2環境のセットアップ

### 2A.1 WSL2のインストール

#### WSL2の有効化

**PowerShell（管理者権限）で実行**:

```powershell
# WSL2の有効化
wsl --install

# または、個別に有効化する場合
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart

# 再起動後、WSL2をデフォルトバージョンに設定
wsl --set-default-version 2
```

#### Ubuntu/Debianのインストール

```powershell
# Ubuntuのインストール
wsl --install -d Ubuntu-22.04

# または、Microsoft Storeから「Ubuntu」をインストール
```

#### WSL2の確認

```bash
# WSL内で実行
wsl --version
cat /proc/version
```

### 2A.2 QEMUのインストール（WSL2内）

#### Ubuntu/Debian（WSL2内）

```bash
sudo apt-get update
sudo apt-get install -y qemu-system-aarch64 qemu-utils
```

#### 動作確認

```bash
qemu-system-aarch64 --version
```

**期待される出力例**:
```
QEMU emulator version 6.2.0
```

### 2A.3 Rustのインストール（WSL2内）

#### rustupのインストール

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source $HOME/.cargo/env
```

#### クロスコンパイルターゲットの追加

```bash
# ARM 64-bit (AArch64) ターゲット
rustup target add aarch64-unknown-none

# 必要に応じて、その他のターゲットも追加
# rustup target add riscv64gc-unknown-none-elf  # RISC-V（将来対応）
```

#### 動作確認

```bash
rustc --version
rustup target list --installed
```

### 2A.4 クロスコンパイルツールチェーンのインストール（WSL2内）

#### ARM GCC ツールチェーンのインストール

**Ubuntu/Debian（WSL2内）**:

```bash
sudo apt-get install -y gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
```

**または、Linaro GCCツールチェーンを使用する場合**:

```bash
# Linaro GCCツールチェーンのダウンロード（例）
wget https://releases.linaro.org/components/toolchain/binaries/latest/aarch64-linux-gnu/gcc-linaro-12.3.0-2023.05-x86_64_aarch64-linux-gnu.tar.xz
tar -xf gcc-linaro-12.3.0-2023.05-x86_64_aarch64-linux-gnu.tar.xz
export PATH=$PATH:$(pwd)/gcc-linaro-12.3.0-2023.05-x86_64_aarch64-linux-gnu/bin
```

#### 動作確認

```bash
aarch64-linux-gnu-gcc --version
aarch64-linux-gnu-objdump --version
```

### 2A.5 GDBのインストール（WSL2内）

#### Ubuntu/Debian（WSL2内）

```bash
sudo apt-get install -y gdb-multiarch
```

#### 動作確認

```bash
gdb-multiarch --version
```

### 2A.6 Windows + WSL2特有の注意点

#### パフォーマンス

- WSL2のファイルシステムはLinuxファイルシステム（`/home/`など）で高速
- Windowsファイルシステム（`/mnt/c/`など）へのアクセスは遅いため、プロジェクトはWSL2のLinuxファイルシステムに配置することを推奨

#### プロジェクトの配置

```bash
# 推奨: WSL2のLinuxファイルシステムに配置
cd ~/source/retron

# 非推奨: Windowsファイルシステム（/mnt/c/）は避ける
# cd /mnt/c/Users/username/source/retron  # 遅い
```

#### X11転送（GUIアプリケーションが必要な場合）

```bash
# Windows側でXサーバーをインストール（例: VcXsrv、X410）
# WSL2内で環境変数を設定
export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0.0
```

---

## 2B. macOS環境のセットアップ

### 2B.1 Homebrewのインストール

```bash
# Homebrewがインストールされていない場合
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 2B.2 QEMUのインストール

#### Homebrewを使用

```bash
brew install qemu
```

#### 動作確認

```bash
qemu-system-aarch64 --version
```

**期待される出力例**:
```
QEMU emulator version 7.2.0
```

### 2B.3 Rustのインストール

#### rustupのインストール

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source $HOME/.cargo/env
```

#### クロスコンパイルターゲットの追加

```bash
# ARM 64-bit (AArch64) ターゲット
rustup target add aarch64-unknown-none

# 必要に応じて、その他のターゲットも追加
# rustup target add riscv64gc-unknown-none-elf  # RISC-V（将来対応）
```

#### 動作確認

```bash
rustc --version
rustup target list --installed
```

### 2B.4 クロスコンパイルツールチェーンのインストール

#### ARM GCC ツールチェーンのインストール

**Homebrewを使用**:

```bash
brew install aarch64-elf-gcc aarch64-elf-binutils
```

**または、Linaro GCCツールチェーンを使用する場合**:

```bash
# Linaro GCCツールチェーンのダウンロード（例）
curl -L -O https://releases.linaro.org/components/toolchain/binaries/latest/aarch64-linux-gnu/gcc-linaro-12.3.0-2023.05-darwin_x86_64_aarch64-linux-gnu.tar.xz
tar -xf gcc-linaro-12.3.0-2023.05-darwin_x86_64_aarch64-linux-gnu.tar.xz
export PATH=$PATH:$(pwd)/gcc-linaro-12.3.0-2023.05-darwin_x86_64_aarch64-linux-gnu/bin
```

**注意**: macOS（Apple Silicon）の場合、`darwin_arm64`版を使用してください。

#### 動作確認

```bash
# Homebrew版の場合
aarch64-elf-gcc --version

# Linaro版の場合
aarch64-linux-gnu-gcc --version
```

### 2B.5 GDBのインストール

#### Homebrewを使用

```bash
brew install gdb
```

#### 動作確認

```bash
gdb --version
```

### 2B.6 macOS特有の注意点

#### Apple Silicon（M1/M2/M3）の場合

- ネイティブでARM64アーキテクチャのため、一部のツールは高速に動作
- Rosetta 2を使用してx86_64アプリケーションも実行可能

#### コード署名（GDBの場合）

macOSでGDBを使用する場合、コード署名が必要な場合があります：

```bash
# GDBのコード署名（初回のみ）
codesign --entitlements - --force --sign - $(which gdb)
```

#### パス設定

```bash
# ~/.zshrc または ~/.bash_profile に追加
export PATH="/opt/homebrew/bin:$PATH"  # Apple Siliconの場合
# または
export PATH="/usr/local/bin:$PATH"     # Intel Macの場合
```

---

## 2C. Linux環境のセットアップ

### 2C.1 QEMUのインストール

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y qemu-system-aarch64 qemu-utils
```

#### 動作確認

```bash
qemu-system-aarch64 --version
```

### 2C.2 Rustのインストール

#### rustupのインストール

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source $HOME/.cargo/env
```

#### クロスコンパイルターゲットの追加

```bash
# ARM 64-bit (AArch64) ターゲット
rustup target add aarch64-unknown-none
```

### 2C.3 クロスコンパイルツールチェーンのインストール

#### ARM GCC ツールチェーンのインストール

**Ubuntu/Debian**:

```bash
sudo apt-get install -y gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
```

### 2C.4 GDBのインストール

#### Ubuntu/Debian

```bash
sudo apt-get install -y gdb-multiarch
```

---

## 3. T-Kernelの取得と準備

### 3.1 T-Kernelの取得

**詳細**: T-Kernelの取得方法については、[TKERNEL_ACQUISITION.md](TKERNEL_ACQUISITION.md) を参照してください。

#### T-Kernelのソースコード取得（概要）

```bash
# third_partyディレクトリに移動
cd third_party

# 方法1: GitHubリポジトリから取得（推奨）
git clone https://github.com/tron-forum/tkernel_2.git

# 方法2: TRONフォーラム公式サイトから取得
# https://www.tron.org/ からダウンロード

# 方法3: T-Kernelトレーサビリティサービスから取得
# https://trace.tron.org/ から検索・取得
```

### 3.2 T-Kernelのビルド設定

#### ビルド設定ファイルの確認

T-Kernelのビルド設定を確認し、ARM Generic AArch64向けの設定を作成します。

```bash
cd third_party/t-kernel
# ビルド設定ファイルを確認
# 通常、Makefile または configure スクリプトがあります
```

#### ターゲット設定

ARM Generic AArch64向けの設定例：

```makefile
# 例: Makefile または config ファイル
ARCH = aarch64
CROSS_COMPILE = aarch64-linux-gnu-
```

---

## 4. プロジェクト構造の作成

### 4.1 ディレクトリ構造

```bash
cd /home/miyasaka/source/retron

# プロジェクトディレクトリ構造の作成
mkdir -p kernel
mkdir -p rust-tkernel-interface
mkdir -p hal
mkdir -p system-services
mkdir -p drivers
mkdir -p ui
mkdir -p apps
mkdir -p tools
mkdir -p build
mkdir -p third_party
```

### 4.2 Rustプロジェクトの初期化

```bash
# ルートのCargo.tomlを作成
cat > Cargo.toml << 'EOF'
[workspace]
members = [
    "rust-tkernel-interface",
    "system-services",
    "ui",
    "apps",
]

[profile.release]
opt-level = "s"
lto = true
EOF

# 各Rustプロジェクトの初期化
cd rust-tkernel-interface
cargo init --name retron-tkernel-interface --lib

cd ../system-services
cargo init --name retron-system-services --lib

cd ../ui
cargo init --name retron-ui --lib

cd ../apps
cargo init --name retron-apps --bin
```

### 4.3 ビルドスクリプトの作成

```bash
# build.sh を作成
cat > build.sh << 'EOF'
#!/bin/bash
set -e

# ターゲットアーキテクチャ
TARGET="aarch64-unknown-none"

# ビルドディレクトリ
BUILD_DIR="build"

# ビルド
echo "Building ReTron OS for ${TARGET}..."

# Rustプロジェクトのビルド
cargo build --target ${TARGET} --release

# T-Kernelのビルド（必要に応じて）
# cd third_party/t-kernel
# make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu-

echo "Build completed!"
EOF

chmod +x build.sh
```

---

## 5. QEMU環境のセットアップ

### 5.1 仮想ディスクの作成

```bash
# 16GBの仮想ディスクを作成
qemu-img create -f raw retron.img 16G
```

### 5.2 QEMU起動スクリプトの作成

```bash
# run-qemu.sh を作成
cat > run-qemu.sh << 'EOF'
#!/bin/bash

# QEMU起動パラメータ
QEMU=qemu-system-aarch64
MACHINE=virt
CPU=cortex-a53
MEMORY=1G
DISK=retron.img
NETDEV=user,id=net0
NETDEVICE=virtio-net-device,netdev=net0

# QEMU起動
${QEMU} \
  -machine ${MACHINE} \
  -cpu ${CPU} \
  -m ${MEMORY} \
  -drive file=${DISK},format=raw \
  -netdev ${NETDEV} \
  -device ${NETDEVICE} \
  -nographic \
  -serial stdio
EOF

chmod +x run-qemu.sh
```

### 5.3 QEMU起動の確認

```bash
# QEMUが起動できることを確認（実際のOSイメージがない場合はエラーになりますが、QEMU自体は動作します）
./run-qemu.sh
# Ctrl+A, X で終了
```

---

## 6. デバッグ環境のセットアップ

### 6.1 GDB設定ファイルの作成

```bash
# .gdbinit を作成
cat > .gdbinit << 'EOF'
# GDB設定
set architecture aarch64
target remote :1234
EOF
```

### 6.2 QEMU + GDBデバッグ用スクリプト

```bash
# run-qemu-gdb.sh を作成
cat > run-qemu-gdb.sh << 'EOF'
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
EOF

chmod +x run-qemu-gdb.sh
```

### 6.3 GDBデバッグの実行

```bash
# ターミナル1: QEMUをGDBサーバーモードで起動
./run-qemu-gdb.sh

# ターミナル2: GDBで接続
gdb-multiarch -arch aarch64
# GDB内で:
# (gdb) target remote :1234
# (gdb) file build/kernel/retron.elf
# (gdb) break main
# (gdb) continue
```

---

## 7. 動作確認

### 7.1 環境確認チェックリスト

以下のコマンドで環境が正しく構築されているか確認します：

```bash
# チェックスクリプト
cat > check-environment.sh << 'EOF'
#!/bin/bash

echo "=== Development Environment Check ==="

echo -n "QEMU: "
qemu-system-aarch64 --version | head -1 || echo "NOT FOUND"

echo -n "Rust: "
rustc --version || echo "NOT FOUND"

echo -n "Rust target (aarch64-unknown-none): "
rustup target list --installed | grep aarch64-unknown-none || echo "NOT INSTALLED"

echo -n "GCC (aarch64-linux-gnu): "
aarch64-linux-gnu-gcc --version | head -1 || echo "NOT FOUND"

echo -n "GDB: "
gdb-multiarch --version | head -1 || echo "NOT FOUND"

echo "=== Check Complete ==="
EOF

chmod +x check-environment.sh
./check-environment.sh
```

### 7.2 簡単なテストビルド

```bash
# 最小限のRustプログラムでテスト
cd rust-tkernel-interface
cat > src/lib.rs << 'EOF'
#![no_std]

#[no_mangle]
pub extern "C" fn test_function() -> i32 {
    42
}
EOF

# ビルドテスト
cargo build --target aarch64-unknown-none --release

# ビルドが成功すれば、環境は正しく構築されています
```

---

## 8. トラブルシューティング

### 8.1 よくある問題

#### QEMUが見つからない

**Windows 11 + WSL2 / Linux**:
```bash
# Ubuntu/Debianの場合
sudo apt-get install -y qemu-system-aarch64
```

**macOS**:
```bash
brew install qemu
```

#### Rustターゲットがインストールされない

```bash
# rustupの更新
rustup self update
rustup target add aarch64-unknown-none
```

#### クロスコンパイルツールチェーンが見つからない

**Windows 11 + WSL2 / Linux**:
```bash
# Ubuntu/Debianの場合
sudo apt-get install -y gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
```

**macOS**:
```bash
brew install aarch64-elf-gcc aarch64-elf-binutils
```

#### GDBがアーキテクチャを認識しない

**Windows 11 + WSL2 / Linux**:
```bash
# gdb-multiarchを使用
gdb-multiarch -arch aarch64
```

**macOS**:
```bash
# 通常のgdbを使用
gdb -arch aarch64
```

### 8.2 環境別の注意点

#### Windows 11 + WSL2

- **ファイルパス**: Windowsファイルシステム（`/mnt/c/`）は遅いため、WSL2のLinuxファイルシステム（`~/`）を使用
- **パフォーマンス**: WSL2のメモリ制限を調整する場合、`.wslconfig`ファイルを作成
- **ネットワーク**: WSL2のネットワークはNATを使用するため、ポートフォワーディングが必要な場合がある

**`.wslconfig`の例**（Windows側の`%USERPROFILE%\.wslconfig`）:
```ini
[wsl2]
memory=8GB
processors=4
```

#### macOS

- **Apple Silicon**: ネイティブARM64のため、一部のツールは高速
- **Intel Mac**: Rosetta 2を使用してx86_64アプリケーションを実行
- **コード署名**: GDBなどの一部ツールはコード署名が必要な場合がある
- **パス**: Homebrewのパスが正しく設定されているか確認

#### Linux

- **ディストリビューション**: Ubuntu/Debian以外の場合は、パッケージマネージャーを適宜変更
- **権限**: `sudo`が必要な操作を確認

### 8.3 参考情報

- **QEMU公式ドキュメント**: https://www.qemu.org/documentation/
- **Rust Embedded Book**: https://docs.rust-embedded.org/book/
- **T-Kernel公式サイト**: T-Kernelの公式サイトを参照

---

## 9. 次のステップ

開発環境の構築が完了したら、以下のステップに進みます：

1. **T-Kernelの統合**
   - T-Kernelのソースコードを取得
   - T-Kernelのビルド設定
   - T-Kernelの動作確認

2. **Rust ↔ T-Kernel インターフェースの実装**
   - FFIバインディングの作成
   - システムコールラッパーの実装

3. **最小限のプロトタイプの実装**
   - カーネル起動
   - UART経由のHello World出力

詳細は [DEVELOPMENT_PROCESS.md](DEVELOPMENT_PROCESS.md) を参照してください。

---

## 10. 参照ドキュメント

- [REQUIREMENTS_SPEC.md](REQUIREMENTS_SPEC.md) - 要件定義書
- [ARCHITECTURE.md](ARCHITECTURE.md) - システムアーキテクチャ設計
- [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md) - T-Kernelベースアーキテクチャ設計
- [DEVELOPMENT_PROCESS.md](DEVELOPMENT_PROCESS.md) - 開発工程標準プロセス

---

## 変更履歴

| バージョン | 日付 | 変更内容 | 変更者 |
|-----------|------|----------|--------|
| 1.0 | 2024年 | 初版作成 | - |

