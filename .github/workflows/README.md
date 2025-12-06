# CI/CD ワークフロー

このディレクトリには、Retron OSのCI/CDパイプラインの設定が含まれています。

## ワークフロー一覧

### CI/CD Pipeline (`ci.yml`)
メインのCI/CDパイプラインで、プッシュとプルリクエスト時に実行されます。

**ジョブ:**
1. **Format Check** - コードフォーマットの確認
2. **Clippy Check** - Rust linterによるコード品質チェック
3. **Build** - リリースビルドの作成
4. **Tests** - テスト実行（no_std環境のため制限あり）
5. **Documentation** - ドキュメント生成
6. **Cargo Check** - 型チェックと依存関係の確認
7. **Security Audit** - セキュリティ監査

**重要な設定:**
- `RUSTFLAGS: -D warnings` - すべての警告をエラーとして扱う
- ゼロ警告ポリシーを厳格に適用

### Security Scan (`security.yml`)
セキュリティスキャンワークフローで、定期的に実行されます。

**スケジュール:**
- 毎週日曜日に自動実行
- Cargo.toml/Cargo.lock変更時に実行
- 手動実行も可能

**ジョブ:**
1. **Dependency Audit** - 依存関係の脆弱性スキャン
2. **Dependency Updates** - 古い依存関係の確認

## ローカルでの実行

### フォーマットチェック
```bash
cd kernel
cargo fmt --all -- --check
```

### Clippyチェック
```bash
cd kernel
cargo clippy --lib --bin retron-kernel -- -D warnings
```

### ビルド
```bash
cd kernel
cargo build --release --target x86_64-unknown-none
```

### セキュリティ監査
```bash
cargo install cargo-audit
cd kernel
cargo audit
```

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
