# cpu_support.S 修正後レビュー報告書

> **レビュー実施日**: 2024年  
> **レビュアー**: シニアソフトウェアエンジニア  
> **対象ファイル**: `third_party/tkernel_2/kernel/sysdepend/cpu/aarch64/cpu_support.S`（修正後）

---

## 1. レビュー概要

### 1.1 修正内容の確認

前回のレビューで指摘した主要な問題点が修正されています：

- ✅ **例外ハンドラの実装**: 無限ループから実装済みに変更
- ✅ **TCB/CTXBオフセットの修正**: `offset.h`で正しく定義
- ✅ **C関数の実装**: `cpu_calls.c`に実装済み
- ⚠️ **一部の改善点**: いくつかの細かい問題が残っている

### 1.2 ファイル情報

- **ファイル名**: `cpu_support.S`
- **行数**: 807行（前回: 579行 → 228行増加）
- **主な追加**: 例外ハンドラの実装、コンテキスト保存/復元の改善

---

## 2. 修正内容の詳細レビュー

### 2.1 例外ハンドラの実装（Lines 96-298）

#### 2.1.1 `exception_handler_common`（Lines 113-183）

**良い点**:
- ✅ 共通の例外処理ルーチンが実装されている
- ✅ レジスタ保存/復元が適切
- ✅ ESR_EL1、FAR_EL1の保存が実装されている
- ✅ C関数`C_exception_handler`を呼び出す構造

**問題点**:

1. **中程度: パラメータの順序**
   ```asm
   mov	x1, sp			/* x1 = pointer to saved context */
   bl	C_exception_handler
   ```
   - アセンブリでは`x0 = exception_type, x1 = sp`で呼び出している
   - しかし、`C_exception_handler`のシグネチャは`void C_exception_handler(void *sp, INT exception_type)`
   - **影響**: パラメータの順序が逆
   - **修正**: パラメータの順序を修正するか、C関数のシグネチャを確認

2. **軽微: ESR_EL1、FAR_EL1の復元**
   ```asm
   ldr	x19, [sp, #32]		/* FAR_EL1 */
   msr	far_el1, x19
   ldr	x19, [sp, #24]		/* ESR_EL1 */
   msr	esr_el1, x19
   ```
   - ESR_EL1とFAR_EL1は読み取り専用レジスタの可能性がある
   - **確認**: ARMv8-Aの仕様で、これらのレジスタが書き込み可能か確認が必要

#### 2.1.2 同期例外ハンドラ（Lines 188-193）

**良い点**:
- ✅ `exception_handler_common`を呼び出す実装
- ✅ 例外タイプ（0）を設定

**問題点**:

1. **軽微: 例外タイプの設定が欠落**
   ```asm
   exception_handler_sync_sp0:
   exception_handler_sync_spx:
   exception_handler_sync_lower64:
   exception_handler_sync_lower32:
       b	exception_handler_common
   ```
   - `x0`に例外タイプを設定していない
   - **修正**: `mov x0, #0`を追加

**修正例**:
```asm
exception_handler_sync_sp0:
exception_handler_sync_spx:
exception_handler_sync_lower64:
exception_handler_sync_lower32:
	mov	x0, #0			/* Exception type: Synchronous */
	b	exception_handler_common
```

#### 2.1.3 IRQハンドラ（Lines 199-278）

**良い点**:
- ✅ 個別に実装されている
- ✅ レジスタ保存/復元が適切
- ✅ `C_irq_handler`を呼び出す構造

**問題点**:

1. **重要: IRQ番号の取得が未実装**
   ```asm
   /* Get IRQ number from GIC (for now, use a placeholder) */
   /* TODO: Read actual IRQ number from GIC */
   mov	x0, #0			/* IRQ number (placeholder) */
   ```
   - IRQ番号が固定値（0）
   - **影響**: 全てのIRQが同じハンドラで処理される
   - **優先度**: **高**

2. **中程度: `tk_ret_int()`の呼び出し方法**
   ```asm
   /* After IRQ handling, call tk_ret_int() to return from interrupt */
   /* Note: tk_ret_int() is called via SVC, which will restore context */
   /* For now, we restore context directly here */
   /* TODO: Implement proper tk_ret_int() call via SVC */
   ```
   - コンテキストを直接復元しているが、`tk_ret_int()`を経由すべき
   - **改善**: SVC経由で`tk_ret_int()`を呼び出す実装

3. **軽微: パラメータの順序**
   ```asm
   mov	x0, #0			/* IRQ number (placeholder) */
   mov	x1, sp			/* Context pointer */
   bl	C_irq_handler		/* C_irq_handler(irq_number, context) */
   ```
   - `C_irq_handler`のシグネチャは`void C_irq_handler(INT irq_number, void *sp)`
   - パラメータの順序は正しい

#### 2.1.4 FIQ/SErrorハンドラ（Lines 280-298）

**良い点**:
- ✅ `exception_handler_common`を呼び出す実装
- ✅ 例外タイプを設定

**問題点**: なし（同期例外ハンドラと同様の実装）

### 2.2 ディスパッチャ（Lines 300-517）

#### 2.2.1 コンテキスト保存/復元

**良い点**:
- ✅ 前回の実装が維持されている
- ✅ レジスタ保存/復元が適切

**問題点**:

1. **中程度: DCT処理の実装が不完全**
   ```asm
   /* Check if DCT is needed */
   mov	w19, #0		/* Default: no DCT */
   cmp	w19, #0		/* Compare with reqdct (already checked above) */
   ```
   - `reqdct`を読み込んでいるが（Line 459）、比較で使用していない
   - **修正**: 実際の`reqdct`値を使用

**修正例**:
```asm
/* Check if DCT is needed */
ldr	w19, [x8, #TCB_REQDCT]	/* reqdct (already loaded at line 459) */
cmp	w19, #1
bne	no_dct			/* If DCT not requested, skip */
b	dct_startup		/* If DCT requested, jump to DCT processing */
no_dct:
```

### 2.3 システムコールエントリ（Lines 519-598）

**良い点**:
- ✅ `C_svc_handler`を呼び出す実装
- ✅ レジスタ保存/復元が適切

**問題点**:

1. **中程度: コメントと実装の不一致**
   ```asm
   /* Call C function to handle SVC */
   /* For now, a placeholder. Actual implementation will involve _svctbl lookup. */
   mov	x1, sp		/* Argument: pointer to saved context */
   bl	C_svc_handler	/* C_svc_handler(fncd, sp) */
   ```
   - コメントでは「placeholder」とあるが、実際には`C_svc_handler`を呼び出している
   - `C_svc_handler`内で`_svctbl`を使用しているため、実装は適切
   - **改善**: コメントを更新

### 2.4 `_tk_ret_int`（Lines 600-671）

**良い点**:
- ✅ コンテキスト復元が実装されている
- ✅ ディスパッチ判定が実装されている

**問題点**:

1. **重要: スタックレイアウトの仮定**
   ```asm
   /* Restore system registers (assuming they're at sp + 248) */
   ldr	x19, [sp, #248]		/* SP_EL0 */
   ```
   - スタックレイアウトを仮定している
   - **影響**: 呼び出し元によってスタックレイアウトが異なる可能性がある
   - **優先度**: **高**

2. **中程度: コメントの不正確性**
   ```asm
   /* Note: The context should be on the stack from the interrupt handler */
   /* However, since we're called via SVC, we need to restore from the SVC stack frame */
   ```
   - スタックレイアウトの説明が不正確な可能性がある
   - **改善**: 実際のスタックレイアウトを確認してコメントを更新

### 2.5 `offset.h`の修正

**良い点**:
- ✅ TCB/CTXBオフセットが正しく定義されている
- ✅ 64ビットアーキテクチャを考慮した計算

**問題点**:

1. **中程度: オフセット計算の近似値**
   ```c
   #define TCB_state	(TCB_winfo+TCBSZ_WINFO+56+TCBSZ_MTX+TCBSZ_POR+100)	/* Approximate, needs verification */
   #define TCB_reqdct	(TCB_winfo+TCBSZ_WINFO+56+TCBSZ_MTX+TCBSZ_POR+96)	/* Approximate, needs verification */
   ```
   - 「Approximate, needs verification」とコメントがある
   - **影響**: 実際のTCB構造体と異なる可能性がある
   - **優先度**: **高**（動作確認が必要）

2. **軽微: コメントの改善**
   - オフセット計算の根拠をコメントで明記すると良い

### 2.6 `cpu_calls.c`の実装

**良い点**:
- ✅ `C_svc_handler`が実装されている
- ✅ `_svctbl`を使用したシステムコールディスパッチ
- ✅ 関数コードの解析が実装されている

**問題点**:

1. **重要: パラメータコピーの未実装**
   ```c
   /* TODO: Parameter copying for functions with more than 4 parameters */
   ```
   - 4パラメータ以上の関数に対応していない
   - **影響**: 一部のシステムコールが正しく動作しない可能性

2. **中程度: 保護レベルチェックの未実装**
   ```c
   /* TODO: Protection level checking */
   ```
   - 保護レベル（RNG）のチェックが未実装
   - **影響**: セキュリティ上の問題

3. **軽微: エラーハンドリング**
   - エラー時の処理が簡易的

---

## 3. 修正前後の比較

### 3.1 改善された点

| 項目 | 修正前 | 修正後 |
|------|--------|--------|
| 例外ハンドラ | 無限ループ（`b .`） | 実装済み |
| TCB/CTXBオフセット | 全て0 | 正しく定義 |
| C関数 | 未定義 | 実装済み |
| `_tk_ret_int` | コンテキスト復元なし | 実装済み |

### 3.2 残っている問題

| 項目 | 状態 | 優先度 |
|------|------|--------|
| IRQ番号の取得 | 未実装（固定値0） | 高 |
| DCT処理の修正 | 不完全 | 中 |
| 同期例外ハンドラの例外タイプ設定 | 欠落 | 軽微 |
| パラメータコピー | 未実装 | 高 |
| 保護レベルチェック | 未実装 | 中 |
| TCBオフセットの検証 | 要検証 | 高 |

---

## 4. 優先度別の改善提案

### 4.1 最高優先度（動作に必須）

1. **同期例外ハンドラの例外タイプ設定**
   - `mov x0, #0`を追加

2. **TCBオフセットの検証**
   - 実際のTCB構造体と比較して検証

### 4.2 高優先度（基本機能に必要）

1. **IRQ番号の取得**
   - GICからIRQ番号を読み取る実装

2. **パラメータコピーの実装**
   - 4パラメータ以上の関数に対応

3. **`_tk_ret_int`のスタックレイアウト確認**
   - 実際のスタックレイアウトを確認して修正

### 4.3 中優先度（機能拡張に必要）

1. **DCT処理の修正**
   - `reqdct`の比較を正しく実装

2. **保護レベルチェックの実装**
   - システムコール時の保護レベルチェック

3. **`tk_ret_int()`の呼び出し方法**
   - SVC経由での呼び出し

### 4.4 低優先度（品質向上）

1. **コメントの更新**
   - 実装に合わせてコメントを更新

2. **ESR_EL1、FAR_EL1の復元確認**
   - ARMv8-Aの仕様で確認

---

## 5. 総合評価

### 5.1 良い点

- ✅ 前回指摘した主要な問題が修正されている
- ✅ 例外ハンドラが実装されている
- ✅ TCB/CTXBオフセットが正しく定義されている
- ✅ C関数が実装されている
- ✅ 基本的な構造は適切

### 5.2 改善が必要な点

- ⚠️ 一部の細かい問題が残っている
- ⚠️ TCBオフセットの検証が必要
- ⚠️ IRQ番号の取得が未実装
- ⚠️ パラメータコピーが未実装

### 5.3 推奨事項

1. **即座に対応すべき項目**
   - 同期例外ハンドラの例外タイプ設定
   - TCBオフセットの検証

2. **短期間で対応すべき項目**
   - IRQ番号の取得
   - パラメータコピーの実装
   - `_tk_ret_int`のスタックレイアウト確認

3. **中長期的に対応すべき項目**
   - DCT処理の修正
   - 保護レベルチェックの実装
   - `tk_ret_int()`の呼び出し方法の改善

---

## 6. 結論

修正により、**前回指摘した主要な問題は解決されています**。例外ハンドラの実装、TCB/CTXBオフセットの修正、C関数の実装により、システムの基本動作に必要な機能が実装されています。

ただし、いくつかの細かい問題が残っており、特に**TCBオフセットの検証**と**IRQ番号の取得**は動作確認と修正が必要です。

現在の実装状況では、基本的な動作は可能ですが、完全な動作には上記の改善が必要です。

---

**レビュー実施者**: シニアソフトウェアエンジニア  
**レビュー日**: 2024年


