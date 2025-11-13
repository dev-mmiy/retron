# T-Kernel CPU依存部実装ガイド（ARM AArch64）

> **ドキュメント種別**: CPU依存部実装ガイド  
> **対象読者**: 開発者  
> **関連ドキュメント**: [TKERNEL_BUILD_CONFIG.md](TKERNEL_BUILD_CONFIG.md), [TKERNEL_ACQUISITION.md](TKERNEL_ACQUISITION.md)

## ドキュメント情報

| 項目 | 内容 |
|------|------|
| **ドキュメント名** | T-Kernel CPU依存部実装ガイド（ARM AArch64） |
| **バージョン** | 1.0 |
| **作成日** | 2024年 |
| **最終更新日** | 2024年 |
| **作成者** | - |
| **承認者** | - |
| **ステータス** | 草案 |

---

## 概要

このドキュメントでは、T-KernelのCPU依存部をARM AArch64（ARMv8-A）向けに実装する方法を説明します。

**参考実装**: `third_party/tkernel_2/kernel/sysdepend/cpu/em1d/`（ARM11/ARMv6）

---

## 1. T-Kernelが要求するCPU依存部の機能

### 1.1 必須実装ファイル

T-KernelのCPU依存部には、以下のファイルを実装する必要があります：

| ファイル名 | 説明 | 実装難易度 |
|-----------|------|-----------|
| `cpu_conf.h` | CPU設定定義 | 低 |
| `cpu_insn.h` | CPU命令定義 | 中 |
| `cpu_status.h` | CPU状態管理定義 | 中 |
| `cpu_task.h` | タスク管理定義 | 高 |
| `cpu_init.c` | CPU初期化/終了処理 | 中 |
| `cpu_calls.c` | CPU依存システムコール | 中 |
| `cpu_support.S` | アセンブリ実装（ディスパッチャ等） | 高 |
| `cache.c` | キャッシュ操作 | 中 |
| `chkplv.c` | 保護レベルチェック | 低 |
| `offset.h` | TCBオフセット定義 | 中 |

### 1.2 必須実装関数

#### 1.2.1 初期化/終了処理

```c
// CPU依存初期化
EXPORT ER cpu_initialize( void );

// CPU依存終了処理
EXPORT void cpu_shutdown( void );
```

#### 1.2.2 タスク管理

```c
// タスクコンテキストの設定
Inline void setup_context( TCB *tcb );

// タスク起動コードの設定
Inline void setup_stacd( TCB *tcb, INT stacd );

// タスクコンテキストのクリーンアップ
Inline void cleanup_context( TCB *tcb );
```

#### 1.2.3 ディスパッチャ

```c
// 強制ディスパッチ（アセンブリ実装）
EXPORT void dispatch_to_schedtsk( void );

// 通常ディスパッチエントリ（アセンブリ実装）
EXPORT void dispatch_entry( void );
```

#### 1.2.4 割り込み処理

```c
// 割り込みハンドラ起動（アセンブリ実装）
EXPORT void inthdr_startup( void );

// 例外ハンドラ起動（アセンブリ実装）
EXPORT void exchdr_startup( void );

// デフォルトハンドラ起動（アセンブリ実装）
EXPORT void defaulthdr_startup( void );
```

#### 1.2.5 キャッシュ操作

```c
// キャッシュラインサイズ取得
EXPORT INT GetCacheLineSize( void );

// キャッシュフラッシュ
EXPORT void FlushCache( CONST void *laddr, INT len );
EXPORT void FlushCacheM( CONST void *laddr, INT len, UINT mode );

// キャッシュ制御
EXPORT ER ControlCacheM( void *laddr, INT len, UINT mode );
```

#### 1.2.6 CPU状態管理

```c
// CPSR取得（ARMv6）/ PSTATE取得（ARMv8-A）
Inline UINT getCPSR( void );

// 割り込み無効化
Inline UINT disint( void );

// 割り込み有効化
Inline void enaint( UINT cpsr );
```

#### 1.2.7 タスク例外処理

```c
// タスク例外要求
EXPORT void request_tex( TCB *tcb );

// タスク例外ハンドラ設定
EXPORT void setup_texhdr( UW *ssp );
```

---

## 2. ARMv6（ARM11）とARMv8-A（AArch64）の主な違い

### 2.1 レジスタ構成

| 項目 | ARMv6（ARM11） | ARMv8-A（AArch64） |
|------|----------------|-------------------|
| **汎用レジスタ** | R0-R15（32bit） | X0-X30（64bit）、SP（スタックポインタ） |
| **プログラムカウンタ** | R15（PC） | PC（独立レジスタ） |
| **状態レジスタ** | CPSR/SPSR | PSTATE（複数のレジスタに分散） |
| **例外リンクレジスタ** | R14（LR） | LR（X30） |
| **スタックポインタ** | R13（SP） | SP（独立レジスタ） |

### 2.2 例外処理

| 項目 | ARMv6（ARM11） | ARMv8-A（AArch64） |
|------|----------------|-------------------|
| **例外レベル** | モード（USR, SVC, IRQ等） | EL0（ユーザー）、EL1（カーネル）、EL2（ハイパーバイザー）、EL3（セキュアモニター） |
| **例外ベクター** | 固定アドレス（0x00000000等） | ベクターベースアドレスレジスタ（VBAR_EL1） |
| **例外種類** | Reset, Undef, SWI, Prefetch Abort, Data Abort, IRQ, FIQ | Synchronous, IRQ, FIQ, SError |

### 2.3 MMU設定

| 項目 | ARMv6（ARM11） | ARMv8-A（AArch64） |
|------|----------------|-------------------|
| **ページテーブル** | 2レベル（セクション/ページ） | 4レベル（PAGE_SIZE=4KBの場合） |
| **TTBR** | TTBR0, TTBR1 | TTBR0_EL1, TTBR1_EL1 |
| **ASID** | CONTEXTIDR | TCR_EL1（ASID幅設定） |

### 2.4 キャッシュ操作

| 項目 | ARMv6（ARM11） | ARMv8-A（AArch64） |
|------|----------------|-------------------|
| **キャッシュ操作** | CP15コプロセッサ命令 | システムレジスタ操作（DC, IC命令） |
| **命令例** | `mcr p15, 0, r0, cr7, c14, 1` | `dc cisw, x0` |

---

## 3. 実装方針

### 3.1 段階的実装アプローチ

1. **Phase 1: 基本設定とヘッダファイル**
   - `cpu_conf.h` - CPU設定定義
   - `cpu_insn.h` - CPU命令定義（基本）
   - `cpu_status.h` - CPU状態管理定義（基本）

2. **Phase 2: CPU初期化と基本操作**
   - `cpu_init.c` - CPU初期化/終了処理
   - `cpu_insn.h` - CPU命令定義（完全版）
   - `cache.c` - キャッシュ操作

3. **Phase 3: タスク管理**
   - `cpu_task.h` - タスク管理定義
   - `offset.h` - TCBオフセット定義

4. **Phase 4: ディスパッチャと割り込み処理**
   - `cpu_support.S` - アセンブリ実装（ディスパッチャ、割り込み処理）

5. **Phase 5: システムコールと例外処理**
   - `cpu_calls.c` - CPU依存システムコール
   - `chkplv.c` - 保護レベルチェック

### 3.2 実装の優先順位

**高優先度（必須）**:
1. `cpu_init.c` - CPU初期化（最低限の動作に必要）
2. `cpu_support.S` - ディスパッチャ（タスク切り替えに必要）
3. `cpu_task.h` - タスク管理定義（タスク作成に必要）

**中優先度**:
4. `cpu_insn.h` - CPU命令定義（割り込み処理に必要）
5. `cpu_calls.c` - CPU依存システムコール
6. `cache.c` - キャッシュ操作

**低優先度**:
7. `chkplv.c` - 保護レベルチェック
8. その他の最適化

---

## 4. 主要な実装例

### 4.1 cpu_conf.h

```c
/*
 *	cpu_conf.h (ReTron OS ARM AArch64)
 *	CPU-Dependent OS Configuration Information
 */

#ifndef _CPU_CONF_
#define _CPU_CONF_

/*
 * Define 1 when using MMU
 *   0: not using MMU
 *   1: using MMU
 */
#define USE_MMU			0	/* 初期実装ではMMU無効 */

/*
 * Definition of minimum system stack size
 *	Minimum system stack size when setting the system stack size
 *	per task by 'tk_cre_tsk().'
 */
#define MIN_SYS_STACK_SIZE	2048	/* AArch64では64bitレジスタのため大きめに */

/*
 * Maximum logical space ID
 */
#define MAX_LSID		255

#endif /* _CPU_CONF_ */
```

### 4.2 cpu_insn.h（基本実装）

```c
/*
 *	cpu_insn.h (ReTron OS ARM AArch64)
 *	ARM AArch64 Dependent Operation
 */

#ifndef _CPU_INSN_
#define _CPU_INSN_

#include <sys/sysinfo.h>

/* ------------------------------------------------------------------------ */
/*
 *	Control register operation
 */

/*
 * Get PSTATE (equivalent to CPSR in ARMv6)
 *	PSTATEは複数のレジスタに分散しているため、主要な情報を取得
 */
Inline UINT getCPSR( void )
{
	UINT pstate;
	Asm("mrs %0, nzcv" : "=r"(pstate));	/* フラグのみ取得（簡易実装） */
	/* 注意: 完全なPSTATE取得には複数のレジスタ読み取りが必要 */
	return pstate;
}

/*
 * TLB invalidate (equivalent to PurgeTLB in ARMv6)
 */
Inline void PurgeTLB( void )
{
	Asm("tlbi vmalle1is" ::: "memory");	/* TLB invalidate all, inner shareable */
	DSB(); ISB();
}

/* ------------------------------------------------------------------------ */
/*
 *	EIT-related
 */

/*
 * Vector numbers used by the T-Kernel
 *	ARMv8-Aでは例外ベクターはベクターベースアドレスレジスタ（VBAR_EL1）で設定
 */
#define VECNO_DEFAULT	0	/* default handler */
#define VECNO_SVC	1	/* SVC (system call) */
#define VECNO_IRQ	2	/* IRQ interrupt */
#define VECNO_FIQ	3	/* FIQ interrupt */

/*
 * Set interrupt handler
 */
Inline void define_inthdr( INT vecno, FP inthdr )
{
	SCArea->intvec[vecno] = inthdr;
}

/*
 * If it is the task-independent part, TRUE
 */
Inline BOOL isTaskIndependent( void )
{
	return ( SCInfo.taskindp > 0 )? TRUE: FALSE;
}

/*
 * Move to/Restore task independent part
 */
Inline void EnterTaskIndependent( void )
{
	SCInfo.taskindp++;
}
Inline void LeaveTaskIndependent( void )
{
	SCInfo.taskindp--;
}

#endif /* _CPU_INSN_ */
```

### 4.3 cpu_status.h（基本実装）

```c
/*
 *	cpu_status.h (ReTron OS ARM AArch64)
 *	ARM AArch64 Dependent Definition
 */

#ifndef _CPU_STATUS_
#define _CPU_STATUS_

#include <tk/syslib.h>
#include <tk/sysdef.h>
#include "cpu_insn.h"

/*
 * Start/End critical section
 */
#define BEGIN_CRITICAL_SECTION	{ UINT _pstate_ = disint();
#define END_CRITICAL_SECTION	if ( !isDI(_pstate_)			\
				  && ctxtsk != schedtsk			\
				  && !isTaskIndependent()		\
				  && !dispatch_disabled ) {		\
					dispatch();			\
				}					\
				enaint(_pstate_); }

/*
 * Start/End interrupt disable section
 */
#define BEGIN_DISABLE_INTERRUPT	{ UINT _pstate_ = disint();
#define END_DISABLE_INTERRUPT	enaint(_pstate_); }

/*
 * Interrupt enable/disable
 */
#define ENABLE_INTERRUPT	{ enaint(0); }
#define DISABLE_INTERRUPT	{ disint(); }

/*
 * Enable interrupt nesting
 */
#define ENABLE_INTERRUPT_UPTO(level)	{ enaint(0); }

/*
 * Move to/Restore task independent part
 */
#define ENTER_TASK_INDEPENDENT	{ EnterTaskIndependent(); }
#define LEAVE_TASK_INDEPENDENT	{ LeaveTaskIndependent(); }

/* ----------------------------------------------------------------------- */
/*
 *	Check system state
 */
#define in_indp()	( isTaskIndependent() || ctxtsk == NULL )
#define in_ddsp()	( dispatch_disabled	\
			|| in_indp()		\
			|| isDI(getCPSR()) )
#define in_loc()	( isDI(getCPSR())		\
			|| in_indp() )
#define in_qtsk()	( ctxtsk->sysmode > ctxtsk->isysmode )

/* ----------------------------------------------------------------------- */
/*
 *	Task dispatcher startup routine
 */
#define dispatch_request()	/* */

Inline void force_dispatch( void )
{
IMPORT	void	dispatch_to_schedtsk();

	Asm("br %0":: "r"(&dispatch_to_schedtsk));
}

Inline void dispatch( void )
{
	Asm("svc %0":: "i"(SWI_DISPATCH): "lr");
}

/* ----------------------------------------------------------------------- */
/*
 *	Task exception
 */
IMPORT void request_tex( TCB *tcb );

/* ----------------------------------------------------------------------- */

/*
 * Task context block
 */
typedef struct {
	void	*ssp;		/* System stack pointer */
	void	*uatb;		/* Task space page table */
	INT	lsid;		/* Task space ID */
	UW	*svc_ssp;	/* ssp when SVC is called */
} CTXB;

/*
 * CPU information
 */
IMPORT ATR	available_cop;	/* Enabled coprocessor (TA_COPn) */

#endif /* _CPU_STATUS_ */
```

### 4.4 cpu_task.h（基本実装）

```c
/*
 *	cpu_task.h (ReTron OS ARM AArch64)
 *	CPU-Dependent Task Start Processing
 */

#ifndef _CPU_TASK_
#define _CPU_TASK_

#include "cpu_insn.h"

/*
 * System stack configuration at task startup
 *	AArch64では64bitレジスタ、X0-X30（31個）を使用
 */
typedef struct {
	VW	x[30];		/* X0-X29 (X30=LRは別途保存) */
	UW	taskmode;
	void	*sp;		/* SP (Stack Pointer) */
	void	*lr;		/* LR (X30, Link Register) */
	void	*pc;		/* PC (Program Counter) */
	VW	spsr_el1;	/* Saved PSTATE (EL1) */
} SStackFrame;

/*
 * User stack configuration at task startup (only RNG 1-3)
 */
typedef struct {
	/* Empty */
} UStackFrame;

/*
 * Size of system stack area destroyed by 'make_dormant()'
 */
#define DORMANT_STACK_SIZE	( sizeof(VW) * 8 )	/* To 'taskmode' */

/*
 * Size of area kept for special use from system stack
 */
#define RESERVE_SSTACK(tskatr)	0

/*
 * Initial value for task startup
 */
#if USE_MMU
#define INIT_PSTATE(rng)	( ( (rng) == 0 )? PSTATE_EL1: \
				  ( (rng) == 3 )? PSTATE_EL0: PSTATE_EL1 )
#else
#define INIT_PSTATE(rng)	( ( (rng) == 0 )? PSTATE_EL1: PSTATE_EL1 )
#endif

#define INIT_TMF(rng)	( TMF_PPL(rng) | TMF_CPL(rng) )

/*
 * Switch task space
 */
Inline void change_space( void *uatb, INT lsid )
{
	UW	ttbr;

	/* if no task space to switch to is not specified, use system default. */
	Asm("mrs %0, ttbr1_el1": "=r"(ttbr));	/* TTBR1_EL1 */
	if ( uatb != NULL ) {
		ttbr = (UW)uatb | (ttbr & 0x7f);
	}

	/* To synchronize ASID and TTBR change */
	Asm("msr ttbr0_el1, %0":: "r"(ttbr));	/* TTBR0_EL1 */
	Asm("msr contextidr_el1, %0":: "r"(lsid)); /* CONTEXTIDR_EL1 */
	ISB();
}

/*
 * Create stack frame for task startup
 */
Inline void setup_context( TCB *tcb )
{
	SStackFrame	*ssp;
	W		rng;
	UW		pc, spsr;

	rng = (tcb->tskatr & TA_RNG3) >> 8;
	ssp = tcb->isstack;
	ssp--;

	spsr = INIT_PSTATE(rng);
	pc = (UW)tcb->task;

	/* CPU context initialization */
	ssp->taskmode = INIT_TMF(rng);
	ssp->spsr_el1 = spsr;
	ssp->pc = (void*)pc;
	tcb->tskctxb.ssp = ssp;
	tcb->tskctxb.svc_ssp = NULL;

	if ( rng > 0 ) {
		ssp->sp = tcb->istack;	/* User stack */
	}
}

/*
 * Set task startup code
 */
Inline void setup_stacd( TCB *tcb, INT stacd )
{
	SStackFrame	*ssp = tcb->tskctxb.ssp;

	ssp->x[0] = stacd;
	ssp->x[1] = (VW)tcb->exinf;
}

/*
 * Delete task contexts
 */
Inline void cleanup_context( TCB *tcb )
{
}

#endif /* _CPU_TASK_ */
```

### 4.5 cpu_init.c（基本実装）

```c
/*
 *	cpu_init.c (ReTron OS ARM AArch64)
 *	CPU-Dependent Initialization/Finalization
 */

#include "kernel.h"
#include "task.h"
#include "cpu_insn.h"

EXPORT MONHDR	SaveMonHdr;
EXPORT ATR	available_cop;

/*
 * CPU-dependent initialization
 */
EXPORT ER cpu_initialize( void )
{
IMPORT void dispatch_entry( void );
IMPORT void call_entry( void );
IMPORT void _tk_ret_int( void );
IMPORT void call_dbgspt( void );
IMPORT void rettex_entry( void );

	UW	vbar;

	/* Save monitor exception handler */
	/* 注意: ARMv8-Aでは例外ベクターはVBAR_EL1で管理 */
	Asm("mrs %0, vbar_el1": "=r"(vbar));
	/* 例外ハンドラの保存処理（実装詳細は省略） */

	/* Initialize task space */
#if USE_MMU
	/* MMU設定（初期実装ではMMU無効のためスキップ） */
#else
	/* MMU無効の場合、特に設定不要 */
#endif
	PurgeTLB();

	/* available coprocessor(s) */
	available_cop = TA_NULL;

	/* install the exception handler used by the OS */
	define_inthdr(SWI_SVC,	    call_entry);
	define_inthdr(SWI_RETINT,   _tk_ret_int);
	define_inthdr(SWI_DISPATCH, dispatch_entry);
	define_inthdr(SWI_RETTEX,   rettex_entry);
#if USE_DBGSPT
	define_inthdr(SWI_DEBUG,    call_dbgspt);
#endif

	return E_OK;
}

/*
 * CPU-dependent finalization
 */
EXPORT void cpu_shutdown( void )
{
	/* Restore saved monitor exception handler */
	/* 実装詳細は省略 */
}
```

### 4.6 cache.c（基本実装）

```c
/*
 *	cache.c (ReTron OS ARM AArch64)
 *	Cache Operation
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/sysdef.h>

#define	CacheLineSZ	64	/* AArch64では通常64バイト */

/*
 * Obtain cache line size
 */
EXPORT INT GetCacheLineSize( void )
{
	return CacheLineSZ;
}

/*
 * Flush cache
 */
EXPORT void FlushCacheM( CONST void *laddr, INT len, UINT mode )
{
	CONST VB	*p, *ep;

	ep = (VB*)laddr + len;

	if ( (mode & TCM_DCACHE) != 0 ) {
		p = (VB*)((UINT)laddr & ~(CacheLineSZ - 1));
		while ( p < ep ) {
			/* Clean and Invalidate data cache to PoC */
			Asm("dc cisw, %0":: "r"(p));
			p += CacheLineSZ;
		}
	}
	if ( (mode & TCM_ICACHE) != 0 ) {
		p = (VB*)((UINT)laddr & ~(CacheLineSZ - 1));
		while ( p < ep ) {
			/* Invalidate instruction cache to PoC */
			Asm("ic ivau, %0":: "r"(p));
			p += CacheLineSZ;
		}
	}
	DSB(); ISB();
}

EXPORT void FlushCache( CONST void *laddr, INT len )
{
	FlushCacheM(laddr, len, TCM_ICACHE|TCM_DCACHE);
}

/*
 * Control cache
 */
EXPORT ER ControlCacheM( void *laddr, INT len, UINT mode )
{
	VB	*p, *ep;

	if ( (mode & ~(CC_FLUSH|CC_INVALIDATE)) != 0 ) return E_PAR;

	ep = (VB*)laddr + len;
	p = (VB*)((UINT)laddr & ~(CacheLineSZ - 1));

	while ( p < ep ) {
		switch ( mode ) {
		  case CC_FLUSH:
			/* Clean data cache to PoC */
			Asm("dc csw, %0":: "r"(p));
			break;
		  case CC_INVALIDATE:
			/* Invalidate data cache to PoC */
			Asm("dc isw, %0":: "r"(p));
			break;
		  default:
			/* Clean and Invalidate data cache to PoC */
			Asm("dc cisw, %0":: "r"(p));
		}

		/* Invalidate instruction cache to PoC */
		Asm("ic ivau, %0":: "r"(p));

		p += CacheLineSZ;
	}
	DSB(); ISB();

	return E_OK;
}
```

### 4.7 cpu_support.S（基本構造）

```assembly
/*
 *	cpu_support.S
 *
 *	CPU operation specific to ReTron OS ARM AArch64
 */
#define	_in_asm_source_

#include <machine.h>
#include <tk/errno.h>
#include <tk/sysdef.h>
#include <tk/asm.h>
#include <sys/sysinfo.h>

#include "config.h"
#include "cpu_conf.h"
#include "isysconf.h"
#include "tkdev_conf.h"
#include "offset.h"

/* ------------------------------------------------------------------------ */
/*
 * Dispatcher
 *	dispatch_to_schedtsk:
 *		Discard current context and, dispatch to schedtsk forcibly.
 *	dispatch_entry:
 *		Ordinary dispatch processing. Called by svc,  SWI_DISPATCH.
 */

	.text
	.balign	4
	.globl	Csym(dispatch_to_schedtsk)
	.type	Csym(dispatch_to_schedtsk), %function
	.globl	Csym(dispatch_entry)
	.type	Csym(dispatch_entry), %function

Csym(dispatch_to_schedtsk):
	/* EL1 mode / interrupt-disabled state */
	/* 一時スタックの設定 */
	ldr	x0, =tmp_stack_top
	mov	sp, x0

	/* dispatch_disabled = 1 */
	ldr	x1, =Csym(dispatch_disabled)
	mov	x2, #1
	str	w2, [x1]

	/* ctxtsk = NULL, taskmode = 0 */
	ldr	x1, =Csym(ctxtsk)
	mov	x2, #0
	str	x2, [x1]

	/* 割り込み有効化 */
	msr	daifclr, #2	/* Enable IRQ */

	b	l_dispatch0

Csym(dispatch_entry):
	/* EL1 mode / interrupt-disabled state */
	/* コンテキスト保存処理 */
	/* 実装詳細は省略（レジスタ保存、スタック操作等） */

	/* dispatch_disabled = 1 */
	ldr	x1, =Csym(dispatch_disabled)
	mov	x2, #1
	str	w2, [x1]

	/* 割り込み有効化 */
	msr	daifclr, #2	/* Enable IRQ */

	/* コンテキスト保存（X0-X29, SP, LR, PSTATE） */
	/* 実装詳細は省略 */

	/* ctxtsk = NULL, taskmode = 0 */
	ldr	x1, =Csym(ctxtsk)
	ldr	x8, [x1]	/* x8 = ctxtsk */
	str	xzr, [x1]	/* ctxtsk = NULL */

	/* sspをTCBに保存 */
	str	sp, [x8, #TCB_tskctxb + CTXB_ssp]

  l_dispatch0:
	/* スタックアライメント（16バイト境界） */
	and	sp, sp, #~15

	/* 割り込み有効状態 */
	/* schedtskの取得とディスパッチ処理 */
	ldr	x5, =Csym(schedtsk)
	ldr	x6, =Csym(lowpow_discnt)

  l_dispatch1:
	/* 割り込み無効化 */
	msr	daifset, #2	/* Disable IRQ */

	ldr	x8, [x5]	/* x8 = schedtsk */
	cbz	x8, l_lowpower	/* schedtsk == NULL なら低電力モードへ */

  l_dispatch2:
	/* schedtskへのディスパッチ */
	str	x8, [x1]	/* ctxtsk = schedtsk */
	ldr	sp, [x8, #TCB_tskctxb + CTXB_ssp] /* ssp復元 */

	/* タスク空間切り替え（MMU使用時） */
#if USE_MMU
	/* TTBR0_EL1, CONTEXTIDR_EL1の設定 */
	/* 実装詳細は省略 */
#endif

	/* dispatch_disabled = 0 */
	ldr	x1, =Csym(dispatch_disabled)
	mov	x2, #0
	str	w2, [x1]

	/* コンテキスト復元（X0-X29, SP, LR, PSTATE） */
	/* 実装詳細は省略 */

	/* タスク実行再開 */
	ret

  l_lowpower:
	/* 低電力モード処理 */
	/* 実装詳細は省略 */
	b	l_dispatch1

/* ------------------------------------------------------------------------ */
/* 一時スタック */
	.bss
	.balign	16
tmp_stack:
	.space	4096
tmp_stack_top:

/* ------------------------------------------------------------------------ */
```

---

## 5. 実装の注意点

### 5.1 レジスタ使用規則（AArch64）

- **呼び出し側保存レジスタ**: X19-X28
- **呼び出し先保存レジスタ**: X0-X7, X9-X15, X16-X17
- **特殊レジスタ**: X29（FP）, X30（LR）, SP, PC

### 5.2 スタックアライメント

AArch64では、スタックポインタは16バイト境界にアラインする必要があります。

### 5.3 例外処理

ARMv8-Aでは、例外ベクターは`VBAR_EL1`レジスタで設定します。例外ハンドラは、例外の種類に応じて適切なアドレスに配置する必要があります。

### 5.4 MMU設定

初期実装ではMMUを無効（`USE_MMU = 0`）にし、物理アドレスを直接使用することを推奨します。MMUの実装は後段階で追加します。

---

## 6. 実装の難易度評価

| コンポーネント | 難易度 | 理由 |
|--------------|--------|------|
| `cpu_conf.h` | 低 | 設定値の定義のみ |
| `cpu_insn.h` | 中 | ARMv8-Aの命令理解が必要 |
| `cpu_status.h` | 中 | マクロ定義と関数呼び出し |
| `cpu_task.h` | 高 | タスクコンテキスト構造の理解が必要 |
| `cpu_init.c` | 中 | 初期化処理の実装 |
| `cpu_calls.c` | 中 | システムコール実装 |
| `cpu_support.S` | 高 | アセンブリ実装、レジスタ操作の理解が必要 |
| `cache.c` | 中 | ARMv8-Aのキャッシュ命令理解が必要 |
| `chkplv.c` | 低 | 保護レベルチェックのみ |
| `offset.h` | 中 | TCB構造の理解が必要 |

---

## 7. 次のステップ

1. **基本設定ファイルの作成**
   - `cpu_conf.h`の作成
   - `cpu_insn.h`の基本実装

2. **CPU初期化の実装**
   - `cpu_init.c`の実装
   - 例外ハンドラの設定

3. **タスク管理の実装**
   - `cpu_task.h`の実装
   - `offset.h`の実装

4. **ディスパッチャの実装**
   - `cpu_support.S`の実装（最も重要）

5. **テストとデバッグ**
   - QEMU上での動作確認
   - デバッグと修正

---

## 8. 参照ドキュメント

- ARM Architecture Reference Manual ARMv8-A
- T-Kernel 2.0 仕様書
- T-Kernel 2.0 実装仕様書（tef_em1d）
- [TKERNEL_BUILD_CONFIG.md](TKERNEL_BUILD_CONFIG.md) - T-Kernelビルド設定ガイド

---

## 変更履歴

| バージョン | 日付 | 変更内容 | 変更者 |
|-----------|------|----------|--------|
| 1.0 | 2024年 | 初版作成 | - |




