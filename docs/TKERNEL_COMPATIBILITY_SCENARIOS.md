# T-Kernel 互換性実装が必要なシーン

> **ドキュメント種別**: 互換性実装ガイド  
> **対象読者**: 開発者  
> **関連ドキュメント**: [TKERNEL_CPU_IMPLEMENTATION.md](TKERNEL_CPU_IMPLEMENTATION.md), [TKERNEL_BUILD_CONFIG.md](TKERNEL_BUILD_CONFIG.md)

## ドキュメント情報

| 項目 | 内容 |
|------|------|
| **ドキュメント名** | T-Kernel 互換性実装が必要なシーン |
| **バージョン** | 1.0 |
| **作成日** | 2024年 |
| **最終更新日** | 2024年 |
| **作成者** | - |
| **承認者** | - |
| **ステータス** | 草案 |

---

## 概要

このドキュメントでは、T-KernelをARM AArch64（ARMv8-A）向けに実装する際に、どのようなシーンで互換性の実装が必要になるかを説明します。

**注意**: ここで言う「互換性」とは、以下の3つの観点から考えます：

1. **T-Kernel仕様との互換性** - T-KernelのAPI仕様に準拠すること
2. **アーキテクチャ間の互換性** - ARMv6とARMv8-Aの違いを吸収すること
3. **既存アプリケーションとの互換性** - 既存のT-Kernelアプリケーションが動作すること

---

## 1. T-Kernel仕様との互換性が必要なシーン

### 1.1 システムコールAPIの互換性

**シーン**: T-KernelのシステムコールAPIを呼び出すアプリケーションが動作する必要がある

**必要な実装**:
- すべてのT-Kernelシステムコールの実装
- システムコールの引数と戻り値の形式が仕様通りであること
- エラーコードが仕様通りであること

**具体例**:
```c
// T-Kernel仕様に準拠したシステムコール
ER tk_cre_tsk( CONST T_CTSK *pk_ctsk );
ER tk_sta_tsk( ID tskid, INT stacd );
ER tk_slp_tsk( TMO tmout );
```

**実装場所**:
- `kernel/tkernel/src/` - システムコール実装
- `kernel/sysdepend/cpu/aarch64/cpu_calls.c` - CPU依存システムコール

### 1.2 データ構造の互換性

**シーン**: T-Kernelのデータ構造（TCB、セマフォ、イベントフラグ等）が仕様通りである必要がある

**必要な実装**:
- T-Kernel仕様で定義されたデータ構造のサイズとレイアウト
- 構造体メンバーのオフセットが仕様通りであること

**具体例**:
```c
// T-Kernel仕様のタスク制御ブロック（TCB）
typedef struct {
    ID      tskid;
    ATR     tskatr;
    PRI     itskpri;
    // ... その他のメンバー
} TCB;
```

**実装場所**:
- `kernel/tkernel/src/task.h` - TCB定義
- `kernel/sysdepend/cpu/aarch64/offset.h` - TCBオフセット定義

### 1.3 動作仕様の互換性

**シーン**: T-Kernelの動作仕様（タスクスケジューリング、割り込み処理等）が仕様通りである必要がある

**必要な実装**:
- タスクスケジューリングアルゴリズム
- 割り込み処理の優先度
- タイマー処理の精度

**実装場所**:
- `kernel/tkernel/src/task.c` - タスクスケジューリング
- `kernel/tkernel/src/timer.c` - タイマー処理

---

## 2. アーキテクチャ間の互換性が必要なシーン

### 2.1 レジスタ表現の互換性

**シーン**: ARMv6（32bitレジスタ）とARMv8-A（64bitレジスタ）の違いを吸収する必要がある

**必要な実装**:
- レジスタサイズの違いを吸収する抽象化レイヤー
- タスクコンテキストの保存/復元処理

**具体例**:
```c
// ARMv6: 32bitレジスタ（R0-R15）
typedef struct {
    VW  r[12];      // R0-R11
    UW  taskmode;
    void *usp;      // R13_usr
    void *lr_usr;   // R14_usr
    void *pc;       // R15
} SStackFrame_ARMv6;

// ARMv8-A: 64bitレジスタ（X0-X30）
typedef struct {
    VW  x[30];      // X0-X29
    UW  taskmode;
    void *sp;       // SP
    void *lr;       // LR (X30)
    void *pc;       // PC
} SStackFrame_AArch64;
```

**実装場所**:
- `kernel/sysdepend/cpu/aarch64/cpu_task.h` - タスクコンテキスト定義
- `kernel/sysdepend/cpu/aarch64/cpu_support.S` - コンテキストスイッチング

### 2.2 例外処理の互換性

**シーン**: ARMv6の例外モードとARMv8-Aの例外レベル（EL）の違いを吸収する必要がある

**必要な実装**:
- 例外ベクターテーブルの設定方法の違いを吸収
- 例外ハンドラの登録方法の統一

**具体例**:
```c
// ARMv6: 例外ベクターテーブルは固定アドレス（0x00000000等）
// ARMv8-A: 例外ベクターテーブルはVBAR_EL1レジスタで設定

// 互換性レイヤー: 統一されたAPI
Inline void define_inthdr( INT vecno, FP inthdr )
{
    SCArea->intvec[vecno] = inthdr;
    // ARMv8-Aでは、VBAR_EL1に設定されたベクターテーブルを使用
}
```

**実装場所**:
- `kernel/sysdepend/cpu/aarch64/cpu_insn.h` - 例外処理定義
- `kernel/sysdepend/cpu/aarch64/cpu_init.c` - 例外ベクターテーブル設定

### 2.3 MMU設定の互換性

**シーン**: ARMv6の2レベルページテーブルとARMv8-Aの4レベルページテーブルの違いを吸収する必要がある

**必要な実装**:
- ページテーブル構造の抽象化
- MMU設定の統一API

**具体例**:
```c
// ARMv6: TTBR0, TTBR1（2レベルページテーブル）
// ARMv8-A: TTBR0_EL1, TTBR1_EL1（4レベルページテーブル）

// 互換性レイヤー: 統一されたAPI
Inline void change_space( void *uatb, INT lsid )
{
#if USE_MMU
    // ARMv8-A実装
    Asm("msr ttbr0_el1, %0":: "r"(uatb));
    Asm("msr contextidr_el1, %0":: "r"(lsid));
#else
    // MMU無効の場合
#endif
}
```

**実装場所**:
- `kernel/sysdepend/cpu/aarch64/cpu_task.h` - タスク空間切り替え
- `kernel/sysdepend/cpu/aarch64/cpu_init.c` - MMU初期化

### 2.4 キャッシュ操作の互換性

**シーン**: ARMv6のCP15コプロセッサ命令とARMv8-Aのシステムレジスタ操作の違いを吸収する必要がある

**必要な実装**:
- キャッシュ操作の統一API

**具体例**:
```c
// ARMv6: CP15コプロセッサ命令
// Asm("mcr p15, 0, %0, cr7, c14, 1":: "r"(p));

// ARMv8-A: システムレジスタ操作
// Asm("dc cisw, %0":: "r"(p));

// 互換性レイヤー: 統一されたAPI
EXPORT void FlushCacheM( CONST void *laddr, INT len, UINT mode )
{
    // ARMv8-A実装
    Asm("dc cisw, %0":: "r"(p));
}
```

**実装場所**:
- `kernel/sysdepend/cpu/aarch64/cache.c` - キャッシュ操作

---

## 3. 既存アプリケーションとの互換性が必要なシーン

### 3.1 バイナリ互換性

**シーン**: 既存のT-Kernelアプリケーション（ARMv6向けにコンパイル済み）をそのまま実行する必要がある

**必要な実装**:
- **通常は不要**: 異なるアーキテクチャ間ではバイナリ互換性は提供しない
- **代替案**: ソースコード互換性を提供し、再コンパイルを要求する

**注意**: ARMv6（32bit）とARMv8-A（64bit）では、バイナリ互換性は提供できません。アプリケーションは再コンパイルが必要です。

### 3.2 ソースコード互換性

**シーン**: 既存のT-Kernelアプリケーションのソースコードを変更せずに、ARMv8-A向けに再コンパイルして動作させる必要がある

**必要な実装**:
- T-Kernel APIの完全な実装
- データ型の互換性（サイズの違いを吸収）

**具体例**:
```c
// 既存アプリケーション（ARMv6向け）
#include <tk/tkernel.h>

void task_main( INT stacd, void *exinf )
{
    ID  tskid;
    T_CTSK  ctsk;
    
    // T-Kernel API呼び出し（変更不要）
    ctsk.tskatr = TA_HLNG | TA_RNG0;
    ctsk.exinf = NULL;
    ctsk.task = task_main;
    ctsk.itskpri = 10;
    ctsk.stksz = 4096;
    
    tskid = tk_cre_tsk( &ctsk );  // このAPIが動作する必要がある
    tk_sta_tsk( tskid, 0 );
}
```

**実装場所**:
- `kernel/tkernel/src/` - すべてのシステムコール実装
- `include/tk/` - ヘッダーファイル

### 3.3 動作互換性

**シーン**: 既存アプリケーションの動作が、ARMv6とARMv8-Aで同じである必要がある

**必要な実装**:
- タイマー精度の互換性
- スケジューリング動作の互換性
- 割り込み処理の互換性

**具体例**:
```c
// 既存アプリケーションが期待する動作
// - タイマー間隔: 10ms
// - タスク優先度: 0-255
// - 割り込み優先度: 0-255

// これらの動作がARMv8-Aでも同じである必要がある
```

**実装場所**:
- `kernel/tkernel/src/timer.c` - タイマー処理
- `kernel/tkernel/src/task.c` - タスクスケジューリング
- `kernel/sysdepend/cpu/aarch64/cpu_calls.c` - 割り込み処理

---

## 4. 実装時の互換性が必要なシーン

### 4.1 開発環境の互換性

**シーン**: 既存のT-Kernel開発環境（ARMv6向け）と新しい開発環境（ARMv8-A向け）を統合する必要がある

**必要な実装**:
- ビルドシステムの互換性
- デバッグ環境の互換性

**具体例**:
```makefile
# 既存のビルドシステム（ARMv6）
MACHINE = em1d
TETYPE = tef
CC := arm-linux-gnu-gcc

# 新しいビルドシステム（ARMv8-A）
MACHINE = aarch64
TETYPE = retron
CC := aarch64-linux-gnu-gcc

# 互換性: 同じビルドシステム構造を使用
include $(BD)/etc/makerules
```

**実装場所**:
- `etc/sysdepend/retron_aarch64/makerules.sysdepend` - ビルドルール

### 4.2 デバッグ環境の互換性

**シーン**: 既存のデバッグ手法（ARMv6向け）を新しい環境（ARMv8-A向け）でも使用する必要がある

**必要な実装**:
- GDBデバッグの互換性
- トレース機能の互換性

**実装場所**:
- デバッグサポートコード（USE_DBGSPTが有効な場合）

---

## 5. 互換性実装の優先順位

### 5.1 必須（高優先度）

1. **T-Kernel APIの完全実装**
   - すべてのシステムコールの実装
   - データ構造の互換性

2. **タスクコンテキストの互換性**
   - タスク切り替え処理の実装
   - レジスタ保存/復元処理

3. **例外処理の互換性**
   - 例外ベクターテーブルの設定
   - 割り込みハンドラの登録

### 5.2 推奨（中優先度）

4. **MMU設定の互換性**
   - ページテーブル構造の抽象化
   - タスク空間切り替え

5. **キャッシュ操作の互換性**
   - キャッシュ操作APIの統一

### 5.3 オプション（低優先度）

6. **デバッグ環境の互換性**
   - デバッグサポート機能

---

## 6. 互換性実装の実装方針

### 6.1 抽象化レイヤーの提供

異なるアーキテクチャ間の違いを吸収するため、抽象化レイヤーを提供します。

**例**: CPU命令の抽象化
```c
// cpu_insn.h: 抽象化レイヤー
Inline UINT getCPSR( void )
{
#if MACHINE == em1d
    // ARMv6実装
    UINT cpsr;
    Asm("mrs %0, cpsr": "=r"(cpsr));
    return cpsr;
#elif MACHINE == aarch64
    // ARMv8-A実装
    UINT pstate;
    Asm("mrs %0, nzcv": "=r"(pstate));
    return pstate;
#endif
}
```

### 6.2 条件コンパイルの使用

アーキテクチャ固有の実装を条件コンパイルで切り替えます。

**例**: タスクコンテキスト定義
```c
// cpu_task.h
#if MACHINE == em1d
    // ARMv6: 32bitレジスタ
    typedef struct {
        VW  r[12];
        // ...
    } SStackFrame;
#elif MACHINE == aarch64
    // ARMv8-A: 64bitレジスタ
    typedef struct {
        VW  x[30];
        // ...
    } SStackFrame;
#endif
```

### 6.3 統一APIの提供

アーキテクチャ固有の実装を隠蔽し、統一されたAPIを提供します。

**例**: キャッシュ操作
```c
// cache.c: 統一API
EXPORT void FlushCache( CONST void *laddr, INT len )
{
    // アーキテクチャ固有の実装を内部で処理
    FlushCacheM(laddr, len, TCM_ICACHE|TCM_DCACHE);
}
```

---

## 7. 互換性実装のテスト

### 7.1 互換性テストの項目

1. **API互換性テスト**
   - すべてのシステムコールの動作確認
   - エラーコードの確認

2. **動作互換性テスト**
   - タイマー精度の確認
   - スケジューリング動作の確認

3. **アプリケーション互換性テスト**
   - 既存アプリケーションの動作確認

### 7.2 テスト方法

```c
// 互換性テスト例
void test_tk_cre_tsk()
{
    ID  tskid;
    T_CTSK  ctsk;
    
    ctsk.tskatr = TA_HLNG | TA_RNG0;
    ctsk.exinf = NULL;
    ctsk.task = test_task;
    ctsk.itskpri = 10;
    ctsk.stksz = 4096;
    
    tskid = tk_cre_tsk( &ctsk );
    assert( tskid > 0 );  // 成功を確認
    
    tk_del_tsk( tskid );
}
```

---

## 8. まとめ

互換性の実装が必要なシーンは、主に以下の3つです：

1. **T-Kernel仕様との互換性**: すべてのシーンで必須
2. **アーキテクチャ間の互換性**: 移植時に必要
3. **既存アプリケーションとの互換性**: 既存アプリケーションを動作させる場合に必要

**実装の優先順位**:
- **必須**: T-Kernel APIの完全実装、タスクコンテキストの互換性、例外処理の互換性
- **推奨**: MMU設定の互換性、キャッシュ操作の互換性
- **オプション**: デバッグ環境の互換性

**実装方針**:
- 抽象化レイヤーの提供
- 条件コンパイルの使用
- 統一APIの提供

---

## 参照ドキュメント

- [TKERNEL_CPU_IMPLEMENTATION.md](TKERNEL_CPU_IMPLEMENTATION.md) - T-Kernel CPU依存部実装ガイド
- [TKERNEL_BUILD_CONFIG.md](TKERNEL_BUILD_CONFIG.md) - T-Kernelビルド設定ガイド
- T-Kernel 2.0 仕様書

---

## 変更履歴

| バージョン | 日付 | 変更内容 | 変更者 |
|-----------|------|----------|--------|
| 1.0 | 2024年 | 初版作成 | - |




