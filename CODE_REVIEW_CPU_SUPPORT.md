# cpu_support.S コードレビュー報告書

> **レビュー実施日**: 2024年  
> **レビュアー**: シニアソフトウェアエンジニア  
> **対象ファイル**: `third_party/tkernel_2/kernel/sysdepend/cpu/aarch64/cpu_support.S`

---

## 1. レビュー概要

### 1.1 ファイル情報

- **ファイル名**: `cpu_support.S`
- **行数**: 579行
- **目的**: ARM AArch64向けのCPU依存サポートルーチン（コンテキストスイッチング、ディスパッチャ、システムコール処理等）

### 1.2 実装状況

- ✅ 例外ベクターテーブルの定義（構造は完成）
- ✅ ディスパッチャの基本実装（`dispatch_entry`, `dispatch_to_schedtsk`）
- ✅ システムコールエントリの基本実装（`call_entry`）
- ⚠️ 例外ハンドラが未実装（スタブのみ）
- ⚠️ 一部の関数が未完成

---

## 2. 詳細レビュー結果

### 2.1 例外ベクターテーブル（Lines 33-94）

**良い点**:
- ✅ ARMv8-Aの仕様に準拠した16エントリのベクターテーブル
- ✅ 適切なアライメント（2KB境界、各エントリ128バイト）
- ✅ 全ての例外タイプ（同期、IRQ、FIQ、SError）と例外レベル（Current EL、Lower EL）をカバー

**問題点**:

1. **重大: 例外ハンドラが未実装**
   ```asm
   exception_handler_sync_sp0:
   exception_handler_irq_sp0:
   ...
       /* TODO: Implement exception handlers */
       b	.
   ```
   - 全ての例外ハンドラが無限ループ（`b .`）になっている
   - **影響**: 例外発生時にシステムがハングする
   - **優先度**: **最高**

2. **中程度: 例外ハンドラの実装方針が不明確**
   - 各例外タイプに応じた適切な処理が必要
   - 参考実装（em1d）では、例外ハンドラがC関数を呼び出す構造になっている

**推奨実装例**:
```asm
exception_handler_sync_spx:
    /* Save exception context */
    sub	sp, sp, #(34*8)	/* Space for X0-X30, SP_EL0, ELR_EL1, SPSR_EL1, ESR_EL1 */
    /* Save all registers */
    stp	x0, x1, [sp, #0]
    ...
    /* Read ESR_EL1 to determine exception type */
    mrs	x0, esr_el1
    mov	x1, sp		/* Context pointer */
    bl	C_exception_handler	/* C function */
    /* Restore and return */
    ...
    eret
```

### 2.2 ディスパッチャ（Lines 119-337）

#### 2.2.1 `dispatch_to_schedtsk`（Lines 147-167）

**良い点**:
- ✅ 一時スタックの設定が適切
- ✅ `dispatch_disabled`フラグの設定が正しい
- ✅ `ctxtsk`と`TASKMODE`のクリアが適切

**問題点**:

1. **中程度: レジスタの使用**
   ```asm
   ldr	x19, =TMP_STACK_TOP
   mov	sp, x19
   ```
   - `x19`は呼び出し先保存レジスタだが、この関数は呼び出し元に戻らないため問題なし
   - ただし、コメントで明記すると良い

#### 2.2.2 `dispatch_entry`（Lines 169-324）

**良い点**:
- ✅ コンテキスト保存/復元の実装が適切
- ✅ スタックアライメントの処理が正しい
- ✅ 低電力モード処理が実装されている

**問題点**:

1. **重大: コンテキスト保存の不整合**
   ```asm
   /* Save context (X0-X29, SP_EL0, LR, PC, PSTATE) */
   sub	sp, sp, #(15*8)	/* Space for X0-X14 (15 registers) */
   ...
   sub	sp, sp, #(16*8)	/* Space for X15-X29 (15 registers) + TASKMODE + SP_EL0 + LR + PC + PSTATE */
   ```
   - コメントでは「X0-X29, SP_EL0, LR, PC, PSTATE」とあるが、実際には「X0-X29, TASKMODE, SP_EL0, PC, PSTATE」を保存
   - LR（X30）は別途保存されている（Line 177, 312）
   - **改善**: コメントを正確にするか、実装をコメントに合わせる

2. **重要: TCBオフセットの確認が必要**
   ```asm
   str	sp, [x8, #TCB_TSKCTXB + CTXB_SSP] /* Save ssp to TCB */
   ```
   - `TCB_TSKCTXB`と`CTXB_SSP`が`offset.h`で0に定義されている
   - **影響**: 正しいオフセットが設定されていないと、TCBへの保存が失敗する
   - **優先度**: **最高**（前回のレビューで指摘済み）

3. **中程度: DCT処理の実装が不完全**
   ```asm
   mov	w19, #0		/* Default: no DCT */
   cmp	w19, #0		/* Compare with reqdct (already checked above) */
   ```
   - `reqdct`を読み込んでいるが（Line 278）、比較で使用していない
   - **改善**: 実際の`reqdct`値を使用して比較する

4. **軽微: レジスタの使用**
   ```asm
   ldr	x8, [x5]	/* x8 = schedtsk */
   ...
   ldr	x4, =Csym(ctxtsk)
   str	x8, [x4]	/* ctxtsk = schedtsk */
   ```
   - `x4`と`x8`の使い分けが適切

### 2.3 システムコールエントリ（Lines 339-417）

**良い点**:
- ✅ レジスタ保存/復元が適切
- ✅ SVC命令からの関数コード抽出が正しい

**問題点**:

1. **重大: C関数`C_svc_handler`が未定義**
   ```asm
   bl	C_svc_handler	/* C_svc_handler(fncd, sp) */
   ```
   - `C_svc_handler`が宣言されているが（Line 572）、実装が見当たらない
   - **影響**: リンクエラーが発生する
   - **優先度**: **最高**

2. **重要: システムコールテーブルの参照方法**
   ```asm
   /* For now, a placeholder. Actual implementation will involve _svctbl lookup. */
   ```
   - `_svctbl`は定義されているが（Line 560）、使用されていない
   - **改善**: `_svctbl`を使用してシステムコールをディスパッチする実装が必要

3. **中程度: 保護レベルチェックが未実装**
   - 参考実装（em1d）では、保護レベル（RNG）のチェックが実装されている
   - **改善**: 保護レベルチェックを追加

**推奨実装例**:
```asm
Csym(call_entry):
    /* Save context */
    ...
    /* Extract function code */
    sub	x19, x19, #4
    ldr	w0, [x19]
    and	w0, w0, #0x00FFFFFF
    
    /* Check protection level */
    ldr	x19, =TASKMODE
    ldr	w19, [x19]
    and	w19, w19, #TMF_CPL(3)
    /* Compare with limit */
    ...
    
    /* Lookup system call table */
    ldr	x19, =_svctbl
    ldr	x19, [x19, w0, lsl #3]	/* 64-bit pointer = 8 bytes */
    blr	x19
    ...
```

### 2.4 `_tk_ret_int`（Lines 419-443）

**問題点**:

1. **重大: コンテキスト復元が未実装**
   ```asm
   /* TODO: Implement full context restore for tk_ret_int */
   /* For now, this is a placeholder */
   eret				/* Return from exception */
   ```
   - コンテキスト復元が実装されていない
   - **影響**: 割り込み復帰時にレジスタが破壊される
   - **優先度**: **最高**

2. **中程度: ディスパッチ判定の実装**
   ```asm
   cmp	x1, x2
   bne	_ret_int_dispatch	/* If ctxtsk != schedtsk, dispatch */
   ```
   - ディスパッチ判定は実装されているが、コンテキスト復元がないため動作しない

**推奨実装**:
- 参考実装（em1d）を参照して、適切なコンテキスト復元を実装

### 2.5 DCT処理（Lines 445-516）

**良い点**:
- ✅ `setup_texhdr`の呼び出しが適切
- ✅ レジスタ保存/復元が実装されている

**問題点**:

1. **軽微: `setup_texhdr`の実装確認が必要**
   - `setup_texhdr`がC関数として実装されているか確認が必要

### 2.6 `rettex_entry`（Lines 518-536）

**問題点**:

1. **重大: 実装が未完成**
   ```asm
   /* TODO: Implement full context restore for rettex_entry */
   /* For now, this is a placeholder */
   eret				/* Return from exception */
   ```
   - 実装が未完成
   - **優先度**: **高**

### 2.7 システムコールテーブル（Lines 550-568）

**良い点**:
- ✅ `_svctbl`の定義が適切
- ✅ 64ビットポインタ（`.quad`）を使用している

**問題点**:

1. **中程度: `_SVC_ENTRY`マクロの再定義**
   ```asm
   #undef _SVC_ENTRY
   #define _SVC_ENTRY(name)	.quad	Csym(_##name)
   ```
   - マクロの再定義は適切だが、元の定義がどうなっているか確認が必要

### 2.8 インクルードファイル（Lines 28-31）

**問題点**:

1. **重要: インクルードパスの確認**
   ```asm
   #include <tk/sysdepend/tef_em1d/sysdef_depend.h> /* Temporarily use em1d's sysdef_depend.h */
   #include <sys/sysdepend/retron_aarch64/sysinfo_depend.h>
   ```
   - `tef_em1d`のヘッダを一時的に使用している
   - **改善**: ARM AArch64専用の`sysdef_depend.h`を作成する必要がある

2. **中程度: `sysinfo_depend.h`の存在確認**
   - `sysinfo_depend.h`が存在するか確認が必要
   - `TASKMODE`と`TASKINDP`の定義が含まれている必要がある

---

## 3. アーキテクチャ固有の問題

### 3.1 ARMv8-Aの仕様準拠

**良い点**:
- ✅ 例外ベクターテーブルの構造が正しい
- ✅ EL1レジスタ（`sp_el0`, `elr_el1`, `spsr_el1`）の使用が適切
- ✅ `eret`命令の使用が正しい

**問題点**:

1. **中程度: 例外レベル（EL）の切り替え**
   - 現在の実装はEL1のみを想定
   - EL0（ユーザーモード）への切り替え処理が未実装

2. **軽微: PSTATEの扱い**
   ```asm
   mrs	x19, spsr_el1
   str	x19, [sp, #144]	/* PSTATE (SPSR_EL1) */
   ```
   - PSTATEの保存は適切だが、復元時に個別のビットを設定する必要がある場合がある

### 3.2 レジスタの使用

**良い点**:
- ✅ 呼び出し先保存レジスタ（X19-X28）の使用が適切
   - ただし、一部の関数で`x19`, `x20`を一時レジスタとして使用している

**問題点**:

1. **軽微: レジスタの使い分け**
   - `x19`, `x20`を一時レジスタとして多用している
   - コメントで明記すると良い

---

## 4. セキュリティ・安全性の観点

### 4.1 スタックオーバーフロー

**問題点**:

1. **中程度: スタックサイズの確認**
   ```asm
   #define	TMP_STACK_SZ	(4*1024)
   ```
   - 一時スタックサイズが4KB
   - 十分なサイズだが、使用状況を監視する必要がある

### 4.2 競合状態

**良い点**:
- ✅ 割り込み無効化のタイミングが適切
- ✅ `dispatch_disabled`フラグの使用が適切

---

## 5. パフォーマンスの観点

### 5.1 コンテキストスイッチング

**良い点**:
- ✅ レジスタ保存/復元が効率的（`stp`/`ldp`を使用）
- ✅ 不要なメモリアクセスを最小限に

**改善提案**:

1. **軽微: レジスタ保存の最適化**
   - 一部のレジスタ（例: X18）はプラットフォームレジスタとして予約されている可能性がある
   - 使用前に確認が必要

---

## 6. 優先度別の問題点

### 6.1 最高優先度（システム動作に必須）

1. **例外ハンドラの実装**
   - 現在全て無限ループ（`b .`）
   - **影響**: 例外発生時にシステムがハング

2. **TCB/CTXBオフセットの修正**
   - `offset.h`のオフセットが全て0
   - **影響**: コンテキスト保存/復元が失敗

3. **`C_svc_handler`の実装**
   - 未定義の関数を呼び出している
   - **影響**: リンクエラー

4. **`_tk_ret_int`のコンテキスト復元**
   - 未実装
   - **影響**: 割り込み復帰時にレジスタが破壊される

### 6.2 高優先度（基本機能に必要）

1. **システムコールテーブルの使用**
   - `_svctbl`が定義されているが使用されていない

2. **`rettex_entry`の実装**
   - 未完成

3. **保護レベルチェック**
   - システムコール時の保護レベルチェックが未実装

### 6.3 中優先度（機能拡張に必要）

1. **DCT処理の改善**
   - `reqdct`の比較が正しく実装されていない

2. **インクルードファイルの整理**
   - `tef_em1d`のヘッダへの依存を解消

3. **例外ハンドラの詳細実装**
   - 各例外タイプに応じた適切な処理

### 6.4 低優先度（品質向上）

1. **コメントの改善**
   - コンテキスト保存のコメントを正確に

2. **レジスタ使用の明記**
   - 一時レジスタの使用をコメントで明記

---

## 7. 総合評価

### 7.1 良い点

- ✅ ARMv8-Aの仕様を正しく理解した実装
- ✅ 基本的な構造は適切
- ✅ ディスパッチャの基本実装が完成している
- ✅ レジスタ保存/復元の実装が効率的

### 7.2 改善が必要な点

- ❌ 例外ハンドラが未実装（無限ループ）
- ❌ TCB/CTXBオフセットが正しくない
- ❌ 一部の関数が未完成
- ❌ C関数の実装が不足

### 7.3 推奨事項

1. **即座に対応すべき項目**
   - 例外ハンドラの実装（最低限、IRQハンドラ）
   - TCB/CTXBオフセットの修正
   - `C_svc_handler`の実装または`_svctbl`の使用

2. **短期間で対応すべき項目**
   - `_tk_ret_int`のコンテキスト復元
   - `rettex_entry`の実装
   - システムコールテーブルの使用

3. **中長期的に対応すべき項目**
   - 保護レベルチェック
   - インクルードファイルの整理
   - 例外ハンドラの詳細実装

---

## 8. 参考実装

### 8.1 em1d（ARMv6）との比較

**類似点**:
- ディスパッチャの基本構造
- コンテキスト保存/復元の方法

**相違点**:
- ARMv6は32ビット、ARMv8-Aは64ビット
- 例外処理の方法が異なる（例外ベクターテーブル vs 固定アドレス）
- レジスタ数が異なる（16個 vs 31個）

### 8.2 実装の参考

- `third_party/tkernel_2/kernel/sysdepend/cpu/em1d/cpu_support.S`を参考に
- ARMv8-Aの仕様書を参照
- LinuxカーネルのARMv8-A実装を参考（ただし、ライセンスに注意）

---

## 9. 結論

`cpu_support.S`は基本的な構造は適切ですが、**重要な部分が未実装**です。特に例外ハンドラとTCBオフセットの問題を解決しないと、システムは動作しません。

現在の実装状況では、システムを起動することはできませんが、設計方針は正しく、実装を完了すれば動作する見込みです。

**次のステップ**:
1. 例外ハンドラの実装（最優先）
2. TCB/CTXBオフセットの修正
3. `C_svc_handler`の実装または`_svctbl`の使用
4. `_tk_ret_int`のコンテキスト復元

---

**レビュー実施者**: シニアソフトウェアエンジニア  
**レビュー日**: 2024年




