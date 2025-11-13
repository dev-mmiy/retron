# ReTron OS - ドキュメントインデックス

## クイックリファレンス

### 新規参加者
1. [README.md](../README.md) - プロジェクト概要
2. [ARCHITECTURE.md](ARCHITECTURE.md) - システム設計
3. [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md) - カーネル実装方針

### カーネル開発
- [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md) - T-Kernelベースアーキテクチャ設計（主要設計ドキュメント）
- [KERNEL_INITIAL_CHOICE.md](KERNEL_INITIAL_CHOICE.md) - 初期カーネル選択の判断基準（T-Kernel採用の根拠）
- [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md) - 実装比較
- [TRON_KERNEL_OPTIONS.md](TRON_KERNEL_OPTIONS.md) - オプション一覧
- [TKERNEL_IMPLEMENTATION.md](TKERNEL_IMPLEMENTATION.md) - T-Kernel分析

### ファイルシステム開発
- [FILESYSTEM_STRATEGY.md](FILESYSTEM_STRATEGY.md) - 実装戦略
- [FILESYSTEM_IMPLEMENTATION.md](FILESYSTEM_IMPLEMENTATION.md) - 実装詳細
- [TOPPERS_FILESYSTEM.md](TOPPERS_FILESYSTEM.md) - TOPPERS実装ガイド

### ライセンス確認
- [LICENSING.md](LICENSING.md) - ライセンスと参考実装

## ドキュメント一覧

| ドキュメント | 種別 | 対象読者 | 説明 |
|------------|------|---------|------|
| [README.md](../README.md) | 概要 | 全員 | プロジェクトの概要と特徴 |
| [ARCHITECTURE.md](ARCHITECTURE.md) | 設計 | 設計者・開発者 | システムアーキテクチャの全体設計 |
| [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md) | 技術検討 | カーネル開発者 | カーネル実装の比較検討 |
| [TRON_KERNEL_OPTIONS.md](TRON_KERNEL_OPTIONS.md) | 技術リファレンス | カーネル開発者 | TRONカーネル実装オプション一覧 |
| [TKERNEL_IMPLEMENTATION.md](TKERNEL_IMPLEMENTATION.md) | 技術分析 | カーネル開発者 | T-Kernel利用時の実装難易度分析 |
| [KERNEL_EXTENSIBILITY.md](KERNEL_EXTENSIBILITY.md) | 技術分析 | システム設計者 | カーネル拡張性の比較分析（将来の拡張性） |
| [KERNEL_MIGRATION.md](KERNEL_MIGRATION.md) | 技術分析 | システム設計者、PM | TOPPERSからT-Kernelへの移行分析 |
| [KERNEL_INITIAL_CHOICE.md](KERNEL_INITIAL_CHOICE.md) | 意思決定支援 | システム設計者、PM | 初期カーネル選択の判断基準 |
| [FILESYSTEM_STRATEGY.md](FILESYSTEM_STRATEGY.md) | 実装戦略 | ファイルシステム開発者 | ファイルシステム実装戦略 |
| [FILESYSTEM_IMPLEMENTATION.md](FILESYSTEM_IMPLEMENTATION.md) | 実装詳細 | ファイルシステム開発者 | ファイルシステム実装の最適化アプローチ |
| [TOPPERS_FILESYSTEM.md](TOPPERS_FILESYSTEM.md) | 実装ガイド（参考情報） | ファイルシステム開発者 | TOPPERSベースでのファイルシステム実装（参考情報、実際はT-Kernelを使用） |
| [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md) | 設計 | システム設計者・開発者 | T-Kernelベースアーキテクチャ設計（主要設計ドキュメント） |
| [KERNEL_INITIAL_CHOICE.md](KERNEL_INITIAL_CHOICE.md) | 意思決定支援 | システム設計者・PM | 初期カーネル選択の判断基準（T-Kernel採用の根拠） |
| [DEVELOPMENT_PROCESS.md](DEVELOPMENT_PROCESS.md) | プロセス | プロジェクトマネージャー・開発者 | 開発工程標準プロセス |
| [PROGRESS.md](../PROGRESS.md) | 進捗管理 | プロジェクトマネージャー・開発者 | 開発進捗管理 |
| [DEVELOPMENT_MILESTONES.md](DEVELOPMENT_MILESTONES.md) | マイルストーン | 開発者 | 開発マイルストーン（動作確認項目） |
| [LICENSING.md](LICENSING.md) | ライセンス | 全員 | ライセンスと参考実装 |
| [docs/REQUIREMENTS_SPEC.md](docs/REQUIREMENTS_SPEC.md) | 要件定義 | プロジェクトマネージャー・設計者 | 要件定義書 |
| [docs/FUNCTIONAL_REQUIREMENTS.md](docs/FUNCTIONAL_REQUIREMENTS.md) | 要件定義 | 開発者 | 機能要件定義書（機能一覧） |
| [docs/NON_FUNCTIONAL_REQUIREMENTS.md](docs/NON_FUNCTIONAL_REQUIREMENTS.md) | 要件定義 | 開発者・QA | 非機能要件定義書 |
| [docs/DEVELOPMENT_ENVIRONMENT.md](docs/DEVELOPMENT_ENVIRONMENT.md) | 開発ガイド | 開発者 | 開発環境構築ガイド |
| [docs/TKERNEL_ACQUISITION.md](docs/TKERNEL_ACQUISITION.md) | 開発ガイド | 開発者 | T-Kernel取得ガイド |
| [docs/TKERNEL_BUILD_CONFIG.md](docs/TKERNEL_BUILD_CONFIG.md) | 開発ガイド | 開発者 | T-Kernelビルド設定ガイド |
| [docs/TKERNEL_CPU_IMPLEMENTATION.md](docs/TKERNEL_CPU_IMPLEMENTATION.md) | 開発ガイド | 開発者 | T-Kernel CPU依存部実装ガイド（ARM AArch64） |
| [docs/TKERNEL_COMPATIBILITY_SCENARIOS.md](docs/TKERNEL_COMPATIBILITY_SCENARIOS.md) | 開発ガイド | 開発者 | T-Kernel 互換性実装が必要なシーン |
| [docs/TKERNEL_ARMV8_SUPPORT.md](docs/TKERNEL_ARMV8_SUPPORT.md) | 技術説明 | 開発者・システム設計者 | T-KernelのARMv8-A（AArch64）対応について |
| [docs/DELAYED_ARMV8_ISSUES.md](docs/DELAYED_ARMV8_ISSUES.md) | リスク分析 | プロジェクトマネージャー・システム設計者 | ARMv8-A対応を後回しにした場合の問題点 |

## ドキュメントの依存関係

```
README.md
  └─ ARCHITECTURE.md
      ├─ KERNEL_COMPARISON.md
      │   ├─ TRON_KERNEL_OPTIONS.md
      │   └─ TKERNEL_IMPLEMENTATION.md
      └─ FILESYSTEM_STRATEGY.md
          ├─ FILESYSTEM_IMPLEMENTATION.md
          └─ TOPPERS_FILESYSTEM.md
  └─ LICENSING.md
```

## 更新履歴

- 2024年: ドキュメント整理整頓
  - 各ドキュメントにメタ情報（種別、対象読者、関連ドキュメント）を追加
  - ドキュメントインデックスを作成
  - 相互リンクを整理

