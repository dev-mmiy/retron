# ReTron OS

ReTronは、TRONベースの組み込みオペレーティングシステムです。スマートフォンとロボット向けに設計され、将来的にはラップトップ向けOSの基盤となることを目指しています。

## 特徴

- **T-Kernelベース**: TRON仕様に準拠したリアルタイムカーネル（T-Kernel/OS、T-Kernel/SM、T-Kernel/DS）
- **Rust実装**: 安全性とパフォーマンスを両立したシステムコンポーネント（ファイルシステム、ネットワークスタック、UI層）
- **マルチコア対応**: T-Kernelの標準マルチコア機能を活用
- **ROS2互換**: ロボットアプリケーション開発をサポート
- **国際化対応**: 英語・日本語をサポート（UTF-8）
- **セキュリティ**: アクセス制御、暗号化、サンドボックス機能
- **商用利用可能**: オープンソースライセンス

## プロジェクト構造

詳細は [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) を参照してください。

## ドキュメント

詳細なドキュメントは [docs/README.md](docs/README.md) または [docs/INDEX.md](docs/INDEX.md) を参照してください。

### 主要ドキュメント

- **設計**: [docs/TKERNEL_ARCHITECTURE.md](docs/TKERNEL_ARCHITECTURE.md) - T-Kernelベースアーキテクチャ設計（主要設計ドキュメント）
- **設計**: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) - システムアーキテクチャの全体設計
- **進捗管理**: [PROGRESS.md](PROGRESS.md) - 開発進捗管理
- **開発工程**: [docs/DEVELOPMENT_PROCESS.md](docs/DEVELOPMENT_PROCESS.md) - 開発工程標準プロセス
- **マイルストーン**: [docs/DEVELOPMENT_MILESTONES.md](docs/DEVELOPMENT_MILESTONES.md) - 開発マイルストーン（動作確認項目）
- **カーネル選択**: [docs/KERNEL_INITIAL_CHOICE.md](docs/KERNEL_INITIAL_CHOICE.md) - 初期カーネル選択の判断基準（T-Kernel採用の根拠）
- **カーネル**: [docs/KERNEL_COMPARISON.md](docs/KERNEL_COMPARISON.md) - カーネル実装の比較検討
- **ファイルシステム**: [docs/FILESYSTEM_STRATEGY.md](docs/FILESYSTEM_STRATEGY.md) - ファイルシステム実装戦略
- **ライセンス**: [docs/LICENSING.md](docs/LICENSING.md) - ライセンスと参考実装

主要なディレクトリ:
- `kernel/`: T-Kernelカーネル（T-Kernel/OS、T-Kernel/SM、T-Kernel/DS）
- `rust-tkernel-interface/`: Rust ↔ T-Kernel インターフェース層（FFIバインディング、ラッパー）
- `hal/`: ハードウェア抽象化層
- `ros2-api/`: ROS2互換API（Rust）
- `security/`: セキュリティ機能（Rust）
- `i18n/`: 国際化サポート（Rust）
- `filesystem/`: ファイルシステム（Rust実装、VFS + ReTronFS）
- `network/`: ネットワークスタック（Rust実装、lwIPベースまたは独自実装）
- `ui/`: UI層（Rust実装、グラフィックレンダリング、UIフレームワーク）

## 開発状況

現在、設計・計画フェーズです。

## ライセンス

（ライセンスを決定次第、記載予定）

## コントリビューション

コントリビューションを歓迎します。詳細は [CONTRIBUTING.md](CONTRIBUTING.md) を参照してください。

## 参考資料

- [ITRON仕様](http://www.ertl.jp/ITRON/)
- [ROS2 Documentation](https://docs.ros.org/en/humble/)

