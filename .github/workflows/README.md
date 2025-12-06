# CI/CD ワークフロー

このディレクトリには、Retron OSのCI/CDパイプラインの設定が含まれています。

## ⚡ 方針

**ローカルファースト**: 開発者がローカルでテストを実行することを前提とし、GitHub上では必要最低限のチェックのみ実行します。

## ワークフロー一覧

### CI/CD Pipeline (`ci.yml`)
メインのCI/CDパイプラインで、プッシュとプルリクエスト時に実行されます。

**ジョブ（最小構成）:**
1. **Format Check** - コードフォーマットの確認（必須）
2. **Clippy Check** - ゼロ警告ポリシーの検証（必須）
3. **Build** - リリースビルドの確認（必須）

**削除されたジョブ（ローカルで実行）:**
- ~~Tests~~ → ローカルで実行
- ~~Documentation~~ → ローカルで確認
- ~~Cargo Check~~ → Clippyでカバー
- ~~Security Audit~~ → 週次スキャンのみ

**重要な設定:**
- `RUSTFLAGS: -D warnings` - すべての警告をエラーとして扱う
- ゼロ警告ポリシーを厳格に適用
- 高速フィードバック（約2-3分で完了）

### Security Scan (`security.yml`)
セキュリティスキャンワークフローで、定期的に実行されます。

**スケジュール:**
- 毎週日曜日に自動実行
- Cargo.toml/Cargo.lock変更時に実行
- 手動実行も可能

**ジョブ:**
1. **Dependency Audit** - 依存関係の脆弱性スキャン
2. **Dependency Updates** - 古い依存関係の確認

## 📝 プッシュ前のローカルチェックリスト

**PRを作成する前に、以下のコマンドを必ず実行してください:**

### 1. フォーマット修正
```bash
cd kernel
cargo fmt --all
```

### 2. Clippyチェック（ゼロ警告必須）
```bash
cd kernel
cargo clippy --lib --bin retron-kernel -- -D warnings
```

### 3. リリースビルド
```bash
cd kernel
cargo build --release --target x86_64-unknown-none
```

### 4. ドキュメント生成（オプション）
```bash
cd kernel
cargo doc --no-deps --document-private-items
```

### 5. セキュリティ監査（オプション）
```bash
cargo install cargo-audit  # 初回のみ
cd kernel
cargo audit
```

### ⚡ クイックチェック（全て実行）
```bash
cd kernel && \
  cargo fmt --all && \
  cargo clippy --lib --bin retron-kernel -- -D warnings && \
  cargo build --release --target x86_64-unknown-none
```

**注意**: これらすべてが成功しないと、GitHub上のCIも失敗します。

## トラブルシューティング

### フォーマットエラー
```bash
cd kernel
cargo fmt --all
```

### Clippy警告
```bash
cd kernel
cargo clippy --lib --bin retron-kernel --fix --allow-dirty
```

## ゼロ警告ポリシー

Retron OSは**ゼロ警告ポリシー**を採用しています：
- すべてのコミットはコンパイラ警告ゼロを維持
- CIパイプラインで`-D warnings`により警告をエラー扱い
- 警告が検出された場合、PRはマージ不可

## 貢献ガイドライン

PRを作成する前に：
1. `cargo fmt --all`でコードをフォーマット
2. `cargo clippy`で警告がないことを確認
3. `cargo build --release`でビルド成功を確認
4. ローカルでテスト実行（可能な場合）

詳細は[CONTRIBUTING.md](../../docs/CONTRIBUTING.md)を参照してください。
