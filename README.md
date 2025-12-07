# Retron OS

![CI/CD](https://github.com/dev-mmiy/retron/workflows/CI%2FCD%20Pipeline/badge.svg)
![Code Quality](https://img.shields.io/badge/warnings-0-brightgreen)
![Rust](https://img.shields.io/badge/rust-nightly-orange)

TRONベースのモダンOSプロジェクト

## 概要

Retron OSは、μT-Kernel 3.xをベースとしたモダンなオペレーティングシステムです。以下の特徴を持ちます：

- **ベース**: μT-Kernel 3.x
- **実装言語**: Rust (nightly)
- **対象プラットフォーム**: Laptop, モバイル, ロボット制御
- **アーキテクチャ**: モジュラー設計
- **コード品質**: ✨ **ゼロ警告達成**

## 特徴

### 🚀 高性能
- リアルタイムカーネル（μT-Kernel 3.xベース）
- メモリ安全なRust実装
- 効率的なタスクスケジューリング

### 🎯 マルチプラットフォーム
- **Laptop**: キーボード、マウス、ディスプレイ対応
- **Mobile**: タッチスクリーン、センサー、カメラ対応
- **Robot**: アクチュエーター、センサー、ナビゲーション対応

### 🎨 モダンUI
- 2Dグラフィックス描画
- ウィンドウ管理
- ウィジェットシステム
- テーマ対応

### 🤖 ロボット制御
- アクチュエーター制御
- センサー管理
- モーション制御
- ナビゲーション
- 通信機能

## プロジェクト構造

```
retron/
├── kernel/           # カーネルコア（μT-Kernel 3.xベース）
├── core/            # デバイス抽象化レイヤー
├── ui/              # UIフレームワーク
├── robot/           # ロボット制御機能
├── drivers/         # デバイスドライバー
├── tools/           # 開発ツール
├── docs/            # ドキュメント
├── qemu/            # QEMU設定
└── scripts/         # ビルドスクリプト
```

## クイックスタート

### 必要な環境

- Rust (nightly版)
- QEMU
- GCC (クロスコンパイル用)

### セットアップ

```bash
# リポジトリをクローン
git clone <repository-url>
cd retron

# 依存関係をインストール
rustup default nightly
rustup target add x86_64-unknown-none

# ビルド
make build

# QEMUで実行
make run-qemu
```

## コード品質

### ✨ ゼロ警告達成

Retron OSは**コンパイラ警告ゼロ**を達成し、維持しています。

| 指標 | 状態 |
|------|------|
| Clippy警告 | ✅ **0個** |
| Rust Edition | ✅ 2024対応 |
| CI/CD | ✅ 自動チェック |
| フォーマット | ✅ 統一済み |

### 🔍 品質方針

- **ローカルファースト**: 開発者がローカルで品質チェックを実行
- **厳格なCI/CD**: GitHub上で必須3チェック（Format, Clippy, Build）
- **RUSTFLAGS=-D warnings**: すべての警告をエラーとして扱う

## 開発

### ローカル開発環境のセットアップ

```bash
# Rust nightlyのインストール
rustup default nightly
rustup component add rustfmt clippy

# ターゲットの追加
rustup target add x86_64-unknown-none
```

### ビルド前のチェック（必須）

**プッシュ前に必ず実行：**

```bash
cd kernel

# 1. フォーマット
cargo fmt --all

# 2. Clippyチェック（ゼロ警告必須）
cargo clippy --lib --bins -- -D warnings

# 3. リリースビルド
cargo build --release --target x86_64-unknown-none
```

**クイックチェック（一括実行）:**

```bash
cd kernel && \
  cargo fmt --all && \
  cargo clippy --lib --bins -- -D warnings && \
  cargo build --release --target x86_64-unknown-none
```

### ビルド

```bash
# カーネルのビルド
cd kernel
cargo build --release --target x86_64-unknown-none

# 全プロジェクトをビルド
make build
```

### テスト

**注意**: `no_std`カーネルのため、標準的な`cargo test`は使用できません。

```bash
# QEMUで統合テスト
make run-qemu

# ビルドの検証
cd kernel
cargo build --release --target x86_64-unknown-none
```

### デバッグ

```bash
# デバッグビルド
cargo build

# QEMUでデバッグ実行
./qemu/debug.sh

# GDBで接続
gdb -ex 'target remote localhost:1234'
```

## アーキテクチャ

Retron OSは4つの主要レイヤーで構成されています：

1. **Kernel Layer**: メモリ管理、タスク管理、デバイス管理
2. **Core Layer**: プラットフォーム固有の機能（Laptop/Mobile）
3. **UI Framework**: グラフィックス、入力、ウィンドウ管理
4. **Robot Control**: アクチュエーター、センサー、ナビゲーション

詳細は [アーキテクチャドキュメント](docs/ARCHITECTURE.md) を参照してください。

## API リファレンス

- [カーネルAPI](docs/API.md#カーネルapi)
- [Core API](docs/API.md#core-api)
- [UI API](docs/API.md#ui-api)
- [Robot API](docs/API.md#robot-api)

## ドキュメント

- [アーキテクチャ](docs/ARCHITECTURE.md)
- [開発ガイド](docs/DEVELOPMENT.md)
- [API リファレンス](docs/API.md)

## 貢献

1. フォーク
2. ブランチを作成 (`git checkout -b feature/amazing-feature`)
3. 変更をコミット (`git commit -m 'Add amazing feature'`)
4. ブランチにプッシュ (`git push origin feature/amazing-feature`)
5. プルリクエストを作成

## ライセンス

MIT License

## 謝辞

- μT-Kernel プロジェクト
- Rust コミュニティ
- QEMU プロジェクト
