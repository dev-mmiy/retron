# T-Kernel 取得ガイド

> **ドキュメント種別**: T-Kernel取得ガイド  
> **対象読者**: 開発者  
> **関連ドキュメント**: [DEVELOPMENT_ENVIRONMENT.md](DEVELOPMENT_ENVIRONMENT.md), [LICENSING.md](LICENSING.md), [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md)

## ドキュメント情報

| 項目 | 内容 |
|------|------|
| **ドキュメント名** | T-Kernel 取得ガイド |
| **バージョン** | 1.0 |
| **作成日** | 2024年 |
| **最終更新日** | 2024年 |
| **作成者** | - |
| **承認者** | - |
| **ステータス** | 草案 |

---

## 概要

このドキュメントでは、ReTron OSで使用するT-Kernelの取得方法を説明します。

**T-Kernel**: TRONフォーラムが開発・提供するオープンソースのリアルタイムオペレーティングシステム（RTOS）

---

## 1. T-Kernelについて

### 1.1 T-Kernelとは

T-Kernelは、TRONプロジェクトの標準カーネル実装で、以下の特徴があります：

- **リアルタイムOS**: 組み込みシステム向けのリアルタイムOS
- **ITRON準拠**: ITRON（Industrial TRON）仕様に準拠
- **マルチコア対応**: T-Kernel 2.0以降でマルチコア対応
- **商用利用可能**: T-License 2.2の下で商用利用が可能

### 1.2 バージョン情報

- **最新バージョン**: T-Kernel 2.02.00（2015年6月24日リリース）
- **推奨バージョン**: T-Kernel 2.0以降（マルチコア対応のため）

---

## 2. ライセンス情報

### 2.1 T-License 2.2

T-Kernelは**T-License 2.2**の下で提供されています。

**ライセンスの特徴**:
- ✅ **商用利用可能**: 商用製品に組み込み可能
- ✅ **改変自由**: ソースコードの改変が可能
- ✅ **再配布自由**: 改変後のソースコードの再配布が可能
- ✅ **派生物のライセンス制約なし**: 派生物を独自ライセンスで配布可能
- ✅ **GPLとは異なる**: 派生物にGPLを適用する必要がない

**注意点**:
- ライセンス表記の保持が必要
- 著作権表示の保持が必要
- 詳細なライセンス条件は公式ドキュメントを確認

### 2.2 ライセンス確認

T-License 2.2の詳細は以下のリンクから確認できます：

- **T-License 2.2**: [Wikipedia - T-License](https://en.wikipedia.org/wiki/T-License)
- **TRONフォーラム公式サイト**: [https://www.tron.org/](https://www.tron.org/)

---

## 3. T-Kernelの取得方法

### 3.1 方法1: GitHubリポジトリから取得（推奨）

**推奨理由**:
- ✅ 最新のソースコードを取得可能
- ✅ バージョン管理が容易
- ✅ 更新を追跡しやすい

#### 手順

```bash
# third_partyディレクトリに移動
cd third_party

# T-Kernel 2.0のリポジトリをクローン
git clone https://github.com/tron-forum/tkernel_2.git

# または、特定のバージョン/タグをチェックアウト
cd tkernel_2
git tag  # 利用可能なタグを確認
git checkout <tag名>  # 例: git checkout v2.02.00
```

**リポジトリURL**:
- **T-Kernel 2.0**: [https://github.com/tron-forum/tkernel_2](https://github.com/tron-forum/tkernel_2)

### 3.2 方法2: TRONフォーラム公式サイトから取得

**公式サイト**:
- **TRONフォーラム**: [https://www.tron.org/](https://www.tron.org/)

#### 手順

1. TRONフォーラムの公式サイトにアクセス
2. ダウンロードセクションからT-Kernelのソースコードをダウンロード
3. アーカイブを展開

```bash
# third_partyディレクトリに移動
cd third_party

# ダウンロードしたアーカイブを展開
tar -xf t-kernel-*.tar.gz
# または
unzip t-kernel-*.zip
```

### 3.3 方法3: T-Kernelトレーサビリティサービスから取得

**トレーサビリティサービス**:
- **URL**: [https://trace.tron.org/](https://trace.tron.org/)

**特徴**:
- T-License 2.0に基づくソースコードの変遷情報を提供
- ディストリビューションの検索・取得が可能
- ソースコードの入手方法を提供

#### 手順

1. T-Kernelトレーサビリティサービスにアクセス
2. 必要なディストリビューションを検索
3. 入手方法に従って取得

---

## 4. T-Kernelの配置

### 4.1 推奨ディレクトリ構造

```
retron/
└── third_party/
    └── tkernel_2/          # T-Kernel 2.0のソースコード
        ├── kernel/         # カーネル実装
        ├── include/        # ヘッダーファイル
        ├── lib/            # ライブラリ
        └── ...
```

### 4.2 取得後の確認

```bash
# T-Kernelのソースコードを確認
cd third_party/tkernel_2
ls -la

# 主要なディレクトリを確認
ls -d */ | head -10
```

---

## 5. T-Kernelのビルド設定

### 5.1 ビルド設定の確認

T-Kernelのビルド設定を確認します。

```bash
cd third_party/tkernel_2

# ビルド設定ファイルを確認
ls -la Makefile* config* configure* 2>/dev/null

# ドキュメントを確認
ls -la README* INSTALL* 2>/dev/null
```

### 5.2 ARM Generic AArch64向けの設定

#### クロスコンパイルツールチェーンの設定

```bash
# 環境変数の設定
export ARCH=aarch64
export CROSS_COMPILE=aarch64-linux-gnu-
export PATH=$PATH:/usr/bin  # クロスコンパイルツールチェーンのパス
```

#### Makefileの設定例

```makefile
# 例: Makefile または config ファイル
ARCH = aarch64
CROSS_COMPILE = aarch64-linux-gnu-
```

### 5.3 ビルドの実行

```bash
# T-Kernelのビルド（設定に応じて）
cd third_party/tkernel_2
make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu-

# または、T-Kernelのビルドシステムに従う
# 通常、T-Kernelには独自のビルドシステムがあります
```

**注意**: T-Kernelのビルド方法は、T-Kernelのバージョンやディストリビューションによって異なる場合があります。公式ドキュメントを参照してください。

---

## 6. T-Kernelの統合準備

### 6.1 ディレクトリ構造の確認

ReTron OSのプロジェクト構造に合わせて、T-Kernelを統合します。

```
retron/
├── kernel/                    # T-Kernelカーネル（third_party/tkernel_2から参照）
├── rust-tkernel-interface/   # Rust ↔ T-Kernel インターフェース
├── hal/                       # ハードウェア抽象化層
└── third_party/
    └── tkernel_2/             # T-Kernel 2.0のソースコード
```

### 6.2 統合方針

1. **T-Kernelのソースコード**: `third_party/tkernel_2/`に配置
2. **ReTron OSのカーネル層**: `kernel/`ディレクトリでT-Kernelをラップ
3. **Rustインターフェース**: `rust-tkernel-interface/`でT-Kernel APIのRustバインディングを実装

---

## 7. トラブルシューティング

### 7.1 よくある問題

#### GitHubリポジトリが見つからない

```bash
# リポジトリURLを確認
git ls-remote https://github.com/tron-forum/tkernel_2.git

# または、公式サイトから最新の情報を確認
```

#### ビルドが失敗する

```bash
# クロスコンパイルツールチェーンが正しくインストールされているか確認
aarch64-linux-gnu-gcc --version

# T-Kernelのビルド要件を確認
cat third_party/tkernel_2/README* INSTALL* 2>/dev/null
```

#### ライセンス情報が見つからない

```bash
# T-Kernelのソースコード内のライセンスファイルを確認
find third_party/tkernel_2 -name "*LICENSE*" -o -name "*COPYING*" -o -name "*LICENCE*"
```

---

## 8. 参考情報

### 8.1 公式リソース

- **TRONフォーラム公式サイト**: [https://www.tron.org/](https://www.tron.org/)
- **T-Kernel GitHubリポジトリ**: [https://github.com/tron-forum/tkernel_2](https://github.com/tron-forum/tkernel_2)
- **T-Kernelトレーサビリティサービス**: [https://trace.tron.org/](https://trace.tron.org/)
- **T-Kernel 2.0 仕様書**: [TEF020-S001-02.01.00_ja.pdf](https://www.tron.org/ja/wp-content/themes/dp-magjam/pdf/specifications/TEF020-S001-02.01.00_ja.pdf)

### 8.2 ライセンス情報

- **T-License 2.2**: [Wikipedia - T-License](https://en.wikipedia.org/wiki/T-License)
- **TRONフォーラムライセンス情報**: TRONフォーラム公式サイトを参照

### 8.3 開発環境

- **eBinder**: eSol社のT-Kernel開発環境（参考情報）
- **QEMU**: T-Kernel 2.0はQEMUエミュレータをサポート

---

## 9. 次のステップ

T-Kernelの取得が完了したら、以下のステップに進みます：

1. **T-Kernelのビルド確認**
   - T-Kernelが正しくビルドできることを確認
   - ARM Generic AArch64向けのビルド設定を確認

2. **Rust ↔ T-Kernel インターフェースの実装**
   - T-Kernel APIのRust FFIバインディングの作成
   - システムコールラッパーの実装

3. **最小限のプロトタイプの実装**
   - カーネル起動
   - UART経由のHello World出力

詳細は [DEVELOPMENT_ENVIRONMENT.md](DEVELOPMENT_ENVIRONMENT.md) と [TKERNEL_ARCHITECTURE.md](TKERNEL_ARCHITECTURE.md) を参照してください。

---

## 変更履歴

| バージョン | 日付 | 変更内容 | 変更者 |
|-----------|------|----------|--------|
| 1.0 | 2024年 | 初版作成 | - |

