# ReTron OS - プロジェクト構造

## ディレクトリ構造

```
retron/
├── kernel/                    # T-Kernelカーネル
├── rust-tkernel-interface/   # Rust ↔ T-Kernel インターフェース
├── hal/                       # ハードウェア抽象化層
├── system-services/           # システムサービス（ファイルシステム、ネットワークなど）
├── drivers/                   # デバイスドライバ
├── ui/                        # UI層
├── apps/                      # アプリケーション
├── tools/                     # 開発ツール
├── build/                     # ビルド成果物
├── third_party/               # サードパーティライブラリ（T-Kernelなど）
└── docs/                      # ドキュメント
```

## ビルドスクリプト

- `build.sh`: プロジェクト全体のビルドスクリプト
- `run-qemu.sh`: QEMU起動スクリプト
- `run-qemu-gdb.sh`: QEMU + GDBデバッグ用起動スクリプト

## Rustワークスペース

ルートの`Cargo.toml`でワークスペースを定義しています。

### メンバー

- `rust-tkernel-interface`: T-Kernel APIのRustバインディング
- `system-services`: システムサービス実装
- `ui`: UI層実装
- `apps`: アプリケーション

## 次のステップ

1. T-Kernelの取得と統合
2. 最小限のプロトタイプの実装
