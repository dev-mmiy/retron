# GitHub Actions Workflows

このディレクトリには、Retron OSのCI/CDパイプラインを構成するGitHub Actionsワークフローが含まれています。

## 📋 ワークフロー一覧

### 1. `ci.yml` - メインCI/CDパイプライン

**トリガー:**
- `main`ブランチへのpush
- `main`ブランチへのPull Request

**実行内容:**
1. **Format Check** - コードフォーマット検証（`cargo fmt`）
2. **Clippy Lint** - 静的解析とリント（`cargo clippy`）
3. **Build & Zero Warnings Check** - ビルド検証と警告ゼロチェック
   - デバッグビルド
   - リリースビルド
4. **Security Audit** - 依存関係の脆弱性スキャン（`cargo audit`）
5. **Documentation Check** - ドキュメント生成チェック
6. **Code Quality Metrics** - コード品質メトリクス収集
7. **Final Verification** - 全チェック成功の確認

**実行時間:** 約5-10分

**必須チェック:** すべてのジョブが成功する必要があります

### 2. `security.yml` - セキュリティスキャン

**トリガー:**
- 毎週日曜日 0:00 UTC（スケジュール実行）
- `Cargo.toml`/`Cargo.lock`の変更
- 手動実行

**実行内容:**
- 依存関係の脆弱性チェック
- セキュリティ監査レポートの生成

**実行時間:** 約2-3分

## 🎯 品質ゲート

すべてのPull Requestは以下の品質ゲートを通過する必要があります：

| チェック | 説明 | 失敗条件 |
|---------|------|---------|
| ✅ Format | コードフォーマット | `cargo fmt`で差分あり |
| ✅ Clippy | リント | Clippy警告あり |
| ✅ Build (Debug) | デバッグビルド | ビルド失敗または警告あり |
| ✅ Build (Release) | リリースビルド | ビルド失敗または警告あり |
| ✅ Security | セキュリティ | 既知の脆弱性あり |
| ✅ Docs | ドキュメント | ドキュメント生成失敗 |
| ✅ Metrics | コード品質 | FIXME コメントあり |

## 🚨 警告ゼロポリシー

Retron OSは**警告ゼロポリシー**を採用しています：

- すべてのビルドで`RUSTFLAGS=-D warnings`を設定
- コンパイラ警告が1つでもある場合、ビルドは失敗
- PRはマージ前に警告をすべて解消する必要があります

## 📦 アーティファクト

各ワークフローは以下のアーティファクトを生成します：

| ワークフロー | アーティファクト | 保持期間 |
|------------|----------------|---------|
| ci.yml | retron-kernel-release | 7日 |
| security.yml | security-audit-report | 30日 |

## 🔧 ローカルでのチェック

CIと同じチェックをローカルで実行できます：

```bash
# フォーマットチェック
cargo fmt --all -- --check

# Clippyリント
cd kernel && cargo clippy --all-targets -- -D warnings

# ビルド（警告をエラーとして扱う）
RUSTFLAGS="-D warnings" cargo build
RUSTFLAGS="-D warnings" cargo build --release

# セキュリティ監査
cargo install cargo-audit
cargo audit

# ドキュメント生成
cargo doc --no-deps --document-private-items
```

## ⚙️ キャッシュ戦略

ビルド時間を短縮するため、以下をキャッシュしています：

- `~/.cargo/registry` - Cargoレジストリ
- `~/.cargo/git` - Cargo gitリポジトリ
- `kernel/target` - ビルド成果物

キャッシュキー：
- OS
- ジョブタイプ（clippy, debug, release, docs）
- `Cargo.lock`のハッシュ値

## 🔄 ワークフローの更新

ワークフローを更新する場合：

1. ローカルでYAML構文を検証
2. テストブランチで動作確認
3. PRを作成してレビュー
4. マージ後、mainブランチで動作確認

## 📊 ステータスバッジ

README.mdに以下のバッジを追加できます：

```markdown
![CI/CD Pipeline](https://github.com/dev-mmiy/retron/workflows/CI%2FCD%20Pipeline/badge.svg)
![Security Scan](https://github.com/dev-mmiy/retron/workflows/Security%20Scan/badge.svg)
```

## 🆘 トラブルシューティング

### ビルドが遅い
- キャッシュが正しく機能しているか確認
- 依存関係が不必要に多くないか確認

### ランダムに失敗する
- ネットワーク関連の問題の可能性
- リトライメカニズムの追加を検討

### セキュリティ監査が失敗する
- `Cargo.lock`を更新
- 脆弱性のある依存関係を更新または置き換え

## 📚 参考資料

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [rust-toolchain Action](https://github.com/dtolnay/rust-toolchain)
- [cargo-audit](https://github.com/rustsec/rustsec/tree/main/cargo-audit)
