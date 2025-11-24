# Retron OS 開発ガイド

## 開発環境のセットアップ

### 必要なツール

1. **Rust** (nightly版)
   ```bash
   curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
   rustup default nightly
   rustup target add x86_64-unknown-none
   ```

2. **QEMU**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qemu-system-x86
   
   # macOS
   brew install qemu
   
   # Windows
   # QEMU for Windows をダウンロード・インストール
   ```

3. **GCC** (クロスコンパイル用)
   ```bash
   # Ubuntu/Debian
   sudo apt-get install gcc-multilib
   
   # macOS
   brew install gcc
   ```

### プロジェクトのクローン

```bash
git clone <repository-url>
cd retron
```

## ビルド方法

### 全プロジェクトのビルド

```bash
make build
# または
./scripts/build.sh
```

### 個別レイヤーのビルド

```bash
# カーネル
cd kernel
cargo build --release

# Coreレイヤー
cd core
cargo build --release

# UIレイヤー
cd ui
cargo build --release

# Robotレイヤー
cd robot
cargo build --release
```

## 実行方法

### QEMUでの実行

```bash
make run-qemu
# または
./qemu/run.sh
```

### デバッグ実行

```bash
./qemu/debug.sh
```

別のターミナルでGDBを接続:
```bash
gdb -ex 'target remote localhost:1234'
```

## テスト実行

```bash
make test
# または
./scripts/test.sh
```

## 開発ワークフロー

### 1. 新機能の追加

1. 適切なレイヤーに機能を追加
2. テストを書く
3. ドキュメントを更新
4. プルリクエストを作成

### 2. デバッグ

1. ログレベルを設定: `RUST_LOG=debug`
2. GDBを使用してデバッグ
3. QEMUのモニター機能を活用

### 3. パフォーマンス最適化

1. プロファイリングツールを使用
2. メモリ使用量を監視
3. リアルタイム性を確認

## コーディング規約

### Rust

- `rustfmt`を使用してコードをフォーマット
- `clippy`を使用してリント
- ドキュメントコメントを書く

```bash
cargo fmt
cargo clippy
```

### ファイル構成

```
src/
├── main.rs          # エントリーポイント
├── lib.rs           # ライブラリ定義
├── memory.rs        # メモリ管理
├── task.rs          # タスク管理
├── device.rs        # デバイス管理
├── interrupt.rs     # 割り込み処理
└── utkernel.rs      # μT-Kernel互換
```

### 命名規則

- **構造体**: PascalCase (`TaskManager`)
- **関数**: snake_case (`create_task`)
- **定数**: UPPER_SNAKE_CASE (`MAX_TASKS`)
- **変数**: snake_case (`task_id`)

## デバッグ手法

### 1. ログ出力

```rust
use log::{info, warn, error};

info!("Task created: {}", task_id);
warn!("Low memory: {} bytes", free_memory);
error!("Device error: {}", error_code);
```

### 2. アサーション

```rust
assert!(task_id < MAX_TASKS, "Invalid task ID: {}", task_id);
debug_assert!(memory_size > 0, "Invalid memory size");
```

### 3. パニック処理

```rust
#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    error!("Panic: {}", info);
    // システムを安全に停止
    loop {}
}
```

## パフォーマンス最適化

### 1. メモリ管理

- メモリプールを使用
- スタックオーバーフローを防ぐ
- メモリリークをチェック

### 2. タスクスケジューリング

- 優先度を適切に設定
- コンテキストスイッチを最小化
- デッドロックを防ぐ

### 3. 割り込み処理

- 割り込みハンドラーを短く保つ
- 重い処理はタスクに委譲
- 割り込み優先度を設定

## トラブルシューティング

### よくある問題

1. **ビルドエラー**
   - Rustツールチェーンを確認
   - 依存関係を更新

2. **実行時エラー**
   - メモリ不足
   - スタックオーバーフロー
   - デバイス初期化失敗

3. **QEMUエラー**
   - QEMUのバージョンを確認
   - カーネルファイルの存在を確認

### デバッグツール

- **GDB**: デバッガー
- **QEMU Monitor**: システム状態確認
- **Log**: ログ出力
- **Memory Map**: メモリマップ確認

## 貢献方法

1. フォーク
2. ブランチを作成
3. 変更をコミット
4. プルリクエストを作成

### コミットメッセージ

```
feat: 新機能を追加
fix: バグを修正
docs: ドキュメントを更新
style: コードスタイルを修正
refactor: リファクタリング
test: テストを追加
```

## リリース

### バージョニング

- **Major**: 破壊的変更
- **Minor**: 新機能追加
- **Patch**: バグ修正

### リリース手順

1. バージョンを更新
2. 変更履歴を更新
3. タグを作成
4. リリースノートを作成


