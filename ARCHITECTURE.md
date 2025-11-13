# ReTron OS - アーキテクチャ設計

> **ドキュメント種別**: 設計ドキュメント  
> **対象読者**: システム設計者、開発者  
> **関連ドキュメント**: [README.md](README.md), [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md), [FILESYSTEM_STRATEGY.md](FILESYSTEM_STRATEGY.md)

## 概要

ReTronは、TRONベースの組み込みOSで、スマートフォン・ロボット向けに設計されています。
Rustベースの機能拡張、ROS2互換APIを提供します。

## システムアーキテクチャ

```
┌─────────────────────────────────────────────────────────┐
│                  アプリケーション層                        │
│  (スマートフォンアプリ、ロボットアプリケーション)            │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│                  API互換層                                │
│  ┌──────────────┐  ┌──────────────┐                     │
│  │ ROS2互換API  │  │ 国際化API     │                     │
│  └──────────────┘  └──────────────┘                     │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│                  システムサービス層                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ セキュリティ  │  │ デバイス管理  │  │ ファイルシステム│ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│                  カーネル層 (TRONベース)                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ プロセス管理  │  │ メモリ管理   │  │ スケジューラ │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────┐
│                  ハードウェア抽象化層 (HAL)                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ デバイスドライバ│ │ プラットフォーム│ │ ブートローダ │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────┘
```

## プロジェクト構造

```
retron/
├── README.md
├── LICENSE
├── ARCHITECTURE.md          # このファイル
├── CONTRIBUTING.md
├── docs/                     # ドキュメント
│   ├── design/              # 設計ドキュメント
│   ├── api/                 # API仕様書
│   └── i18n/                # 国際化ガイド
├── kernel/                   # TRONベースカーネル
│   ├── tron-core/           # TRONカーネルコア
│   ├── process/             # プロセス管理
│   ├── memory/              # メモリ管理
│   ├── scheduler/           # スケジューラ
│   └── interrupt/           # 割り込み処理
├── hal/                      # ハードウェア抽象化層
│   ├── drivers/             # デバイスドライバ
│   │   ├── uart/
│   │   ├── i2c/
│   │   ├── spi/
│   │   ├── gpio/
│   │   └── display/
│   ├── platform/            # プラットフォーム固有コード
│   │   ├── arm/
│   │   ├── riscv/
│   │   └── x86/
│   └── boot/                # ブートローダ
├── syscall/                  # システムコールインターフェース
│   └── retron/              # ReTron固有システムコール
├── rust-runtime/             # Rustランタイム
│   ├── std/                 # Rust標準ライブラリ実装
│   ├── alloc/               # メモリアロケータ
│   └── core/                # コア型とトレイト
├── ros2-api/                 # ROS2互換API層 (Rust)
│   ├── nodes/
│   ├── topics/
│   ├── services/
│   ├── actions/
│   └── messages/
├── security/                 # セキュリティ機能
│   ├── access-control/      # アクセス制御
│   ├── encryption/           # 暗号化
│   ├── sandbox/             # サンドボックス
│   └── secure-boot/         # セキュアブート
├── i18n/                     # 国際化サポート
│   ├── locale/              # ロケールデータ
│   │   ├── en/
│   │   └── ja/
│   ├── text-input/          # テキスト入力メソッド
│   └── font/                # フォントデータ
├── filesystem/               # ファイルシステム
│   ├── vfs/                 # 仮想ファイルシステム
│   ├── fat32/
│   ├── ext2/
│   └── retronfs/            # ReTron専用ファイルシステム
├── network/                  # ネットワークスタック
│   ├── tcpip/               # TCP/IP実装
│   ├── protocols/           # 各種プロトコル
│   └── drivers/             # ネットワークドライバ
├── build/                    # ビルドシステム
│   ├── cargo/               # Cargo設定
│   ├── cmake/               # CMake設定（Cコード用）
│   └── scripts/             # ビルドスクリプト
├── tools/                    # 開発ツール
│   ├── simulator/           # シミュレータ
│   ├── debugger/            # デバッガ
│   └── test/                # テストツール
├── tests/                    # テストスイート
│   ├── unit/                # ユニットテスト
│   ├── integration/         # 統合テスト
│   └── benchmarks/          # ベンチマーク
└── examples/                 # サンプルコード
    ├── smartphone/           # スマートフォン向けサンプル
    └── robot/               # ロボット向けサンプル
```

## 主要コンポーネント

### 1. カーネル層 (kernel/)

**T-Kernelベースカーネル**
- **コア**: T-Kernel（T-Kernel/OS、T-Kernel/SM、T-Kernel/DS）
- TRON仕様に準拠したリアルタイムカーネル
- タスク管理、メモリ管理、割り込み処理
- マルチコア対応（標準機能）
- Rustでシステムサービスを実装

**主要モジュール:**
- `tkernel-os/`: T-Kernel/OS（タスク管理、メモリ管理、同期制御）
- `tkernel-sm/`: T-Kernel/SM（サブシステム管理、デバイス管理）
- `tkernel-ds/`: T-Kernel/DS（デバッグサポート）
- `process/`: プロセス・タスク管理（T-Kernel APIラッパー）
- `memory/`: メモリ管理（T-Kernelメモリ管理の統合）
- `scheduler/`: スケジューラ（T-Kernelスケジューラ）
- `interrupt/`: 割り込みハンドラ

**アーキテクチャ**:
- T-Kernelをコアとして使用
- ファイルシステム・ネットワークスタックをRustで独自実装
- T-Kernel/SMのサブシステムとして実装

**詳細**: 
- [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md) - T-Kernelベースアーキテクチャ設計
- [KERNEL_INITIAL_CHOICE.md](KERNEL_INITIAL_CHOICE.md) - 初期カーネル選択の判断基準（T-Kernel採用の根拠）
- [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md) - カーネル実装の比較検討
- [TRON_KERNEL_OPTIONS.md](TRON_KERNEL_OPTIONS.md) - TRONカーネル実装オプション一覧

### 2. ハードウェア抽象化層 (hal/)

**プラットフォーム非依存のハードウェアインターフェース**
- ARM、RISC-V、x86アーキテクチャ対応
- デバイスドライバフレームワーク
- ブートローダ

### 3. ROS2互換API (ros2-api/)

**ロボット向け機能**
- ROS2 DDS互換のメッセージング
- ノード、トピック、サービス、アクション
- リアルタイム通信保証

### 4. セキュリティ層 (security/)

**セキュリティ機能**
- アクセス制御（Capability-based）
- 暗号化サポート（TLS、ファイル暗号化）
- アプリケーションサンドボックス
- セキュアブート

### 5. 国際化 (i18n/)

**多言語対応**
- ロケールシステム（英語、日本語）
- 文字エンコーディング（UTF-8）
- テキスト入力メソッド（IME）
- フォントレンダリング

### 6. ファイルシステム (filesystem/)

**ファイルシステムサポート**
- 仮想ファイルシステム（VFS）: 独自実装（**最初に実装**）
- ReTron専用ファイルシステム（拡張機能対応）
- 互換ファイルシステム（FAT32、ext2など、将来追加）

**実装順序**:
1. **Phase 1**: VFS基本構造（抽象化レイヤー）
2. **Phase 2**: ReTron専用ファイルシステム実装（VFSインターフェースを使用）
3. **Phase 3**: 他のファイルシステム（FAT32、ext2など）

**実装方針**:
- 独自実装による開発
- ITRONカーネルに最適化された設計

**詳細**: 
- [FILESYSTEM_STRATEGY.md](FILESYSTEM_STRATEGY.md) - 実装順序と戦略
- [FILESYSTEM_IMPLEMENTATION.md](FILESYSTEM_IMPLEMENTATION.md) - 具体的な実装アプローチ
- [TOPPERS_FILESYSTEM.md](TOPPERS_FILESYSTEM.md) - TOPPERSカーネルベースでの実装方法

### 7. ネットワークスタック (network/)

**ネットワーク機能**
- TCP/IPスタック: 独自実装またはlwIP（軽量）
- IPv4/IPv6サポート
- ソケットAPI
- 各種プロトコル（TCP、UDP、ICMP、ARPなど）

**実装方針**:
- 独自実装による開発、またはlwIPをベースに統合
- ITRONカーネルに最適化された設計

## 開発言語とツール

- **カーネルコア**: C/C++ (TRON実装) + Rust (拡張機能)
- **システムサービス**: Rust
- **API層**: Rust
- **ドライバ**: Rust + C (必要に応じて)
- **ビルドシステム**: Cargo + CMake
- **テスト**: Rust test framework + CUnit

## ビルドとデプロイ

### ターゲットプラットフォーム

1. **スマートフォン**
   - ARM Cortex-A シリーズ
   - 初期ターゲット: ARMv8-A

2. **ロボット**
   - ARM Cortex-A/M シリーズ
   - RISC-V
   - リアルタイム要件対応

3. **将来: ラップトップ**
   - x86_64
   - ARM64

### ビルドプロセス

1. カーネルとHALのビルド
2. システムライブラリのビルド
3. API層のビルド
4. 統合とリンク
5. イメージ生成

## ライセンス

商用利用可能なライセンスを検討（Apache 2.0、MIT、または独自ライセンス）

## 開発フェーズ

### Phase 1: 基盤構築
- **T-Kernelカーネル統合**: T-Kernel/OS、T-Kernel/SM、T-Kernel/DSを統合
- **Rustインターフェース実装**: T-Kernel APIのRust FFIバインディングとラッパー
- **HAL実装**: ハードウェア抽象化層の実装
- **基本的なシステムコール**: T-Kernel APIの実装
- **リアルタイムスケジューラ**: T-Kernelのリアルタイムスケジューラを使用

### Phase 2: カーネル機能拡張
- **マルチコア対応**: T-Kernelのマルチコア機能を活用
- **メモリ管理の拡張**: T-Kernelメモリ管理の拡張
- **スケジューラの最適化**: T-Kernelスケジューラの最適化

### Phase 3: システムサービス開発
- **VFS基本構造の実装**: 仮想ファイルシステムの抽象化レイヤー（Rust）
- **ReTron専用ファイルシステム実装**: VFSインターフェースを使用したファイルシステム（Rust）
- **T-Kernel/SMサブシステム統合**: T-Kernel/SMのサブシステムとして実装
- **TCP/IPスタック実装**: 独自実装（Rust）またはlwIPベースの統合
- **ネットワークドライバフレームワーク**: ドライバフレームワークの実装（Rust）

### Phase 4: 機能拡張
- **ROS2互換API**: ロボット向けAPIの実装
- **セキュリティ機能**: アクセス制御、暗号化、サンドボックス
- **国際化**: 英語・日本語対応

### Phase 5: 最適化とテスト
- **パフォーマンス最適化**: リアルタイム性能の最適化
- **包括的なテスト**: ユニットテスト、統合テスト
- **ドキュメント整備**: API仕様書、開発ガイド

## 関連ドキュメント

### カーネル実装
- [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md) - T-Kernelベースアーキテクチャ設計（Rust実装）**← 主要設計ドキュメント**
- [KERNEL_INITIAL_CHOICE.md](KERNEL_INITIAL_CHOICE.md) - 初期カーネル選択の判断基準（T-Kernel採用の根拠）**← 最終決定の根拠**
- [KERNEL_COMPARISON.md](KERNEL_COMPARISON.md) - カーネル実装の比較検討
- [TRON_KERNEL_OPTIONS.md](TRON_KERNEL_OPTIONS.md) - TRONカーネル実装オプション一覧
- [TKERNEL_IMPLEMENTATION.md](TKERNEL_IMPLEMENTATION.md) - T-Kernel利用時の実装難易度分析
- [KERNEL_EXTENSIBILITY.md](KERNEL_EXTENSIBILITY.md) - カーネル拡張性の比較分析（将来の拡張性）
- [KERNEL_MIGRATION.md](KERNEL_MIGRATION.md) - TOPPERSからT-Kernelへの移行分析（参考）
- [DEVELOPMENT_PROCESS.md](DEVELOPMENT_PROCESS.md) - 開発工程標準プロセス

### ファイルシステム
- [FILESYSTEM_STRATEGY.md](FILESYSTEM_STRATEGY.md) - ファイルシステム実装戦略
- [FILESYSTEM_IMPLEMENTATION.md](FILESYSTEM_IMPLEMENTATION.md) - ファイルシステム実装の最適化アプローチ
- [TOPPERS_FILESYSTEM.md](TOPPERS_FILESYSTEM.md) - TOPPERSベースでのファイルシステム実装（参考情報）

### その他
- [LICENSING.md](LICENSING.md) - ライセンスと参考実装
- [README.md](README.md) - プロジェクト概要

## 参考資料

- ITRON仕様
- ROS2仕様
- Rust Embedded Book

