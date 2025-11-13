# ReTron OS - ドキュメント一覧

## ドキュメント構造

ReTron OSのドキュメントは以下のように分類されています。

## 概要・設計

### [README.md](../README.md)
プロジェクトの概要、特徴、基本的な情報

### [ARCHITECTURE.md](ARCHITECTURE.md)
システムアーキテクチャの全体設計
- システムアーキテクチャ図
- プロジェクト構造
- 主要コンポーネントの説明
- 開発フェーズ

### [Prompt.txt](../Prompt.txt)
プロジェクトの目的と実装方針

## カーネル実装

### [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md)
カーネル実装の比較検討
- TOPPERS ITRONカーネル（参考）
- T-Kernel（最終決定）
- 推奨アプローチ

### [TRON_KERNEL_OPTIONS.md](TRON_KERNEL_OPTIONS.md)
TRONカーネル実装オプション一覧
- TOPPERSプロジェクト
- T-Kernel
- μT-Kernel
- 商用ITRON実装

### [TKERNEL_IMPLEMENTATION.md](TKERNEL_IMPLEMENTATION.md)
T-Kernel利用時の実装難易度分析
- 実装難易度の評価
- TOPPERSとの比較
- リスク評価

### [KERNEL_EXTENSIBILITY.md](KERNEL_EXTENSIBILITY.md)
カーネル拡張性の比較分析（将来の拡張性）
- TOPPERSとT-Kernelの将来の拡張性の比較
- 拡張シナリオ別のPros/Cons
- 推奨事項

### [KERNEL_MIGRATION.md](KERNEL_MIGRATION.md)
TOPPERSからT-Kernelへの移行分析
- 移行の難易度評価
- 移行コストの見積もり
- 移行戦略（段階的移行、ハイブリッドアプローチ）
- 移行の判断基準

### [KERNEL_INITIAL_CHOICE.md](KERNEL_INITIAL_CHOICE.md)
初期カーネル選択の判断基準
- 初期実装からのT-Kernel採用の評価
- マルチコア対応の重要性
- 総コストの比較（TOPPERS→移行 vs T-Kernel初期）
- 推奨事項と実装戦略

### [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md)
T-Kernelベースアーキテクチャ設計
- T-Kernel + Rust実装の全体像
- レイヤー別の詳細設計
- システムサービス層（ファイルシステム、ネットワーク）
- UI層の実装
- オプションと選択肢

### [DEVELOPMENT_PROCESS.md](DEVELOPMENT_PROCESS.md)
開発工程標準プロセス
- OS開発の標準工程
- フェーズ別の詳細（Phase 0-8）
- 開発プロセス（アジャイル、コードレビュー、テスト）
- リスク管理
- 品質保証

### [PROGRESS.md](../PROGRESS.md)
開発進捗管理
- 全体進捗サマリー
- フェーズ別の進捗状況
- 完了した作業、進行中の作業
- 課題・リスク管理
- マイルストーン管理

### [HELLO_WORLD_MILESTONE.md](HELLO_WORLD_MILESTONE.md)
Hello World表示の実現フェーズ
- Hello World表示に必要な要素
- フェーズ別の実現可能性
- 推奨実現フェーズ（Phase 1後半）
- 実装例（UARTドライバ、文字出力API）

### [DEVELOPMENT_MILESTONES.md](DEVELOPMENT_MILESTONES.md)
開発マイルストーン（動作確認項目）
- OS開発の主要な動作確認マイルストーン
- カテゴリ別のマイルストーン（基本動作、カーネル機能、システムサービス、デバイスドライバ、アプリケーション）
- フェーズ別の推奨マイルストーン
- マイルストーンの実装順序

## ファイルシステム

### [FILESYSTEM_STRATEGY.md](FILESYSTEM_STRATEGY.md)
ファイルシステム実装の戦略
- VFS vs ReTron専用ファイルシステム
- 実装順序
- 実装戦略の詳細

### [FILESYSTEM_IMPLEMENTATION.md](FILESYSTEM_IMPLEMENTATION.md)
ファイルシステム実装の最適化アプローチ
- 段階的実装戦略
- 実装の詳細
- ベストプラクティス

### [TOPPERS_FILESYSTEM.md](TOPPERS_FILESYSTEM.md)
TOPPERSカーネルベースでのファイルシステム実装方法（参考情報）
- TOPPERSモジュールとしての実装
- ITRONタスクとVFSの統合
- メモリ管理の統合
- リアルタイム性の確保
- **注意**: 実際の実装ではT-Kernelを使用します

## ライセンス・参考実装

### [LICENSING.md](LICENSING.md)
ライセンスと参考実装
- ライセンス要件
- 参考実装（FreeBSD、lwIP、musl libcなど）
- ライセンス表記の義務

## 要件定義

### [REQUIREMENTS_SPEC.md](REQUIREMENTS_SPEC.md)
要件定義書
- プロジェクト概要
- ターゲットプラットフォーム
- ステークホルダー
- ユースケース概要
- 機能要件・非機能要件の概要
- 制約事項・前提条件
- リスクと課題
- 成功基準

### [FUNCTIONAL_REQUIREMENTS.md](FUNCTIONAL_REQUIREMENTS.md)
機能要件定義書（機能一覧）
- カーネル機能
- システムサービス
- デバイスドライバ
- UI層
- アプリケーションAPI
- 開発・デバッグ機能

### [NON_FUNCTIONAL_REQUIREMENTS.md](NON_FUNCTIONAL_REQUIREMENTS.md)
非機能要件定義書
- パフォーマンス要件
- セキュリティ要件
- 信頼性・可用性要件
- 保守性要件
- 移植性要件
- 拡張性要件
- 互換性要件
- 使いやすさ要件
- ライセンス要件

### [DEVELOPMENT_ENVIRONMENT.md](DEVELOPMENT_ENVIRONMENT.md)
開発環境構築ガイド
- 必要なソフトウェア
- QEMU環境のセットアップ
- Rustクロスコンパイル環境の構築
- T-Kernelの取得と準備
- デバッグ環境のセットアップ
- トラブルシューティング

### [TKERNEL_ACQUISITION.md](TKERNEL_ACQUISITION.md)
T-Kernel取得ガイド
- T-Kernelについて
- ライセンス情報（T-License 2.2）
- 取得方法（GitHub、公式サイト、トレーサビリティサービス）
- ビルド設定
- 統合準備
- トラブルシューティング

### [TKERNEL_BUILD_CONFIG.md](TKERNEL_BUILD_CONFIG.md)
T-Kernelビルド設定ガイド
- T-Kernelのビルドシステム構造
- 現在の設定（tef_em1d）の確認
- ARM AArch64向け設定の作成方針
- ビルド設定ファイルの作成方法
- CPU依存部・デバイス依存部の実装
- 実装の難易度と課題

### [TKERNEL_CPU_IMPLEMENTATION.md](TKERNEL_CPU_IMPLEMENTATION.md)
T-Kernel CPU依存部実装ガイド（ARM AArch64）
- T-Kernelが要求するCPU依存部の機能
- ARMv6（ARM11）とARMv8-A（AArch64）の主な違い
- 実装方針と段階的アプローチ
- 主要な実装例（コード付き）
- 実装の注意点と難易度評価

## ドキュメントの読み方

### 新規参加者向け
1. [README.md](../README.md) - プロジェクトの概要を理解
2. [ARCHITECTURE.md](ARCHITECTURE.md) - システム全体の設計を理解
3. [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md) - カーネル実装の方針を理解

### カーネル開発者向け
1. [TRON_KERNEL_OPTIONS.md](TRON_KERNEL_OPTIONS.md) - TRONカーネルの選択肢
2. [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md) - 実装の比較検討
3. [TKERNEL_IMPLEMENTATION.md](TKERNEL_IMPLEMENTATION.md) - T-Kernel利用時の難易度（参考）

### ファイルシステム開発者向け
1. [FILESYSTEM_STRATEGY.md](FILESYSTEM_STRATEGY.md) - 実装戦略の概要
2. [FILESYSTEM_IMPLEMENTATION.md](FILESYSTEM_IMPLEMENTATION.md) - 具体的な実装アプローチ
3. [TOPPERS_FILESYSTEM.md](TOPPERS_FILESYSTEM.md) - TOPPERSカーネルでの実装方法

### ライセンス確認
1. [LICENSING.md](LICENSING.md) - ライセンス要件と参考実装

