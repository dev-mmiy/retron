# QEMU Boot Issue - Root Cause and Fix

## 問題の根本原因 (Root Cause)

カーネルバイナリがPIE (Position-Independent Executable) として生成されており、DYN タイプのELFファイルになっていました。これにより：

1. **セクションの欠如**: .text, .rodata, .data, .bss セクションがバイナリから欠落
2. **エントリーポイントが0x0**: ELFヘッダーのエントリーポイントが設定されていない
3. **PVHエントリーポイントの不一致**: PVHノートのエントリーポイント (0x100493) が実際のkernel_main (0x101000) と一致していない
4. **バイナリサイズ異常**: 本来19KBであるべきところ1.9KBしかない

## 実施した修正 (Fixes Applied)

### 1. リンカーフラグの追加 (kernel/.cargo/config.toml)

```toml
rustflags = [
    "-C", "link-arg=-Tsrc/linker.ld",
    "-C", "link-arg=--oformat=elf64-x86-64",
    "-C", "link-arg=-static",
    "-C", "link-arg=-nostdlib",
    "-C", "link-arg=-no-pie",          # PIE無効化
    "-C", "relocation-model=static",
    "-C", "code-model=kernel",
    "-C", "target-feature=+soft-float",
]
```

### 2. PVHエントリーポイントの修正 (kernel/src/pvh.rs)

```rust
static PVH_NOTE: PvhNote = PvhNote {
    // 変更前: entry: 0x0000000000100493,
    entry: 0x0000000000101000,  // kernel_main の実アドレス
};
```

### 3. リンカースクリプトの改善 (kernel/src/linker.ld)

- PHDRSセクションを追加してプログラムヘッダーを明示的に定義
- PT_NOTE, PT_LOAD セグメントの適切な配置

## 修正後のバイナリ状態

### ELFヘッダー
```
Type:                EXEC (Executable file)        ✅ (以前: DYN)
Entry point:         0x101000                      ✅ (以前: 0x0)
Machine:             x86-64                         ✅
```

### セクション構成
```
.note.PVH    0x100000  (24 bytes)   - PVH boot note
.text        0x101000  (6594 bytes) - 実行可能コード
.rodata      0x103000  (1147 bytes) - 読み取り専用データ
.data        0x104000  (784 bytes)  - 初期化データ
.bss         0x105000  (64 bytes)   - 未初期化データ
```

### プログラムヘッダー
```
PT_LOAD (R+X)  .text      - コードセグメント
PT_LOAD (R)    .rodata    - 読み取り専用データ
PT_LOAD (R+W)  .data+.bss - データセグメント
PT_NOTE (R)    .note.PVH  - PVH起動ノート
```

### PVHノート
```
Owner:       Xen
Type:        0x12 (XEN_ELFNOTE_PHYS32_ENTRY)
Entry:       0x101000 (kernel_main)  ✅
```

## ローカルでのテスト手順

### 1. 最新のコードを取得

```bash
cd ~/workspace/retron
git checkout claude/phase2-memory-management-018gwNrb8YxPPXqqhuJVYoSz
git pull origin claude/phase2-memory-management-018gwNrb8YxPPXqqhuJVYoSz
```

### 2. クリーンビルド

```bash
cd kernel
cargo clean
cd ..
make build
```

### 3. バイナリの検証

```bash
# ELFタイプとエントリーポイントの確認
readelf -h kernel/target/x86_64-unknown-none/release/retron-kernel | grep -E "(Type|Entry)"
# 期待値:
#   Type: EXEC (Executable file)
#   Entry point address: 0x101000

# セクションの確認
readelf -S kernel/target/x86_64-unknown-none/release/retron-kernel | grep -E "\.text|\.rodata|\.data|\.bss|\.note\.PVH"
# 期待値: 上記5つのセクションが存在すること

# PVHノートの確認
readelf -n kernel/target/x86_64-unknown-none/release/retron-kernel
# 期待値:
#   Owner: Xen
#   description data: 00 10 10 00 00 00 00 00 (0x101000 in little-endian)

# プログラムヘッダーの確認
readelf -l kernel/target/x86_64-unknown-none/release/retron-kernel | grep -A2 "NOTE"
# 期待値: PT_NOTE セグメントが存在すること
```

### 4. QEMUでの起動テスト

```bash
make run-qemu
```

### 期待される出力

```
Starting Retron OS in QEMU...
KERNEL_MAIN: Starting...
KERNEL_MAIN: Debug test 1
KERNEL_MAIN: Debug test 2
KERNEL_MAIN: About to call init()
[初期化処理の出力]
=== Memory Management Test (Step 1) ===
Heap usage: OK
Memory total: 4MB OK
[その他のテスト出力]
==========================================
    Retron OS System Ready
==========================================
All services initialized successfully.
```

## トラブルシューティング

### 問題: まだ "Cannot load x86-64 image" エラーが出る

1. バイナリタイプを確認:
   ```bash
   file kernel/target/x86_64-unknown-none/release/retron-kernel
   ```
   "statically linked" と表示されるべき

2. cargo cleanして再ビルド:
   ```bash
   cd kernel
   cargo clean
   cd ..
   make build
   ```

3. QEMUのバージョン確認:
   ```bash
   qemu-system-x86_64 --version
   ```
   PVH対応には 4.0 以降が必要（現在: 8.2.2で対応済み）

### 問題: セクションが見つからない

- `.cargo/config.toml` の変更が反映されているか確認
- `cargo clean` を実行してから再ビルド

## コミット履歴

1. `fix(boot): add PT_NOTE program header for PVH note` (d9ccfec)
   - リンカースクリプトにPHDRSセクションを追加

2. `fix(boot): fix EXEC binary generation and PVH entry point` (08a67bf)
   - リンカーフラグを追加してEXEC型バイナリ生成
   - PVHエントリーポイントを0x101000に修正

## 次のステップ

1. ローカルでQEMU起動テストを実施
2. 成功したらStep 2の実装に進む
3. 失敗した場合はエラーログを共有

## 技術的な詳細

### なぜPIEが問題だったのか

- PIE (Position-Independent Executable) は通常のユーザーランドアプリケーション用
- カーネルは固定アドレス (1MB = 0x100000) から開始する必要がある
- DYNタイプのELFはリロケーション情報を持ち、動的リンカーが必要
- カーネルは動的リンカーを使用せず、静的にリンクされる必要がある

### リンカースクリプトの役割

- セクションを特定のアドレスに配置
- PVHノートをバイナリの先頭に配置
- 各セクションを適切なアラインメント (4KB) で配置
- プログラムヘッダーを生成してQEMUがロード可能にする

### PVHブートプロトコル

- Xen Project由来の64bitカーネル直接起動プロトコル
- Multiboot 1 (32bitのみ) の代替
- QEMUが `.note.PVH` セクションを検出してエントリーポイントにジャンプ
- Linux kernel も使用している標準的な方法
