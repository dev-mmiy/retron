# 開発環境セットアップ手順

## 現在の状況

✅ **完了**:
- Rust 1.91.0 インストール済み
- Rustターゲット (aarch64-unknown-none) インストール済み

❌ **要インストール** (sudo権限が必要):
- QEMU
- クロスコンパイルツールチェーン (gcc-aarch64-linux-gnu)
- GDB (multiarch)

## インストール方法

### 方法1: セットアップスクリプトを使用（推奨）

```bash
cd /home/miyasaka/source/retron
./setup-environment.sh
```

このスクリプトは、必要なパッケージを自動的にインストールします。

### 方法2: 手動でインストール

以下のコマンドを順番に実行してください：

```bash
# 1. パッケージリストの更新
sudo apt-get update

# 2. QEMUのインストール
sudo apt-get install -y qemu-system-aarch64 qemu-utils

# 3. クロスコンパイルツールチェーンのインストール
sudo apt-get install -y gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu

# 4. GDB (multiarch)のインストール
sudo apt-get install -y gdb-multiarch
```

## インストール後の確認

インストールが完了したら、環境確認スクリプトを実行してください：

```bash
./check-environment.sh
```

すべてのツールが正しくインストールされていることを確認してください。

## 次のステップ

インストールが完了したら、以下のステップに進みます：

1. T-Kernelの取得と準備
2. プロジェクト構造の作成
3. 最小限のプロトタイプの実装

詳細は [docs/DEVELOPMENT_ENVIRONMENT.md](docs/DEVELOPMENT_ENVIRONMENT.md) を参照してください。

