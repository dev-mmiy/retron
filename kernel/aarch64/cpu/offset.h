/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by TRON Forum(http://www.tron.org/) at 2015/06/01.
 *    Modified for AArch64 (ReTron OS) at 2024.
 *
 *----------------------------------------------------------------------
 */

/*
 *	offset.h (AArch64)
 *	TCB Offset Definition
 */

#ifndef _OFFSET_
#define _OFFSET_

/* Adjust offset of TCB member variables in offset.h for cpu_support.S */
#include "config.h"

/*	TCB.wrdvno	*/
#if defined(NUM_PORID)
#define TCBSZ_POR	(8)	/* = sizeof(RNO) - 64bit aligned */
#else
#define TCBSZ_POR	(0)
#endif

/*	TCB.mtxlist	*/
#if defined(NUM_MTXID)
#define TCBSZ_MTX	(8)	/* = sizeof(MTXCB*) - 64bit pointer */
#else
#define TCBSZ_MTX	(0)
#endif

/*	TCB.winfo.xxx	*/
#if defined(NUM_PORID)
#define TCBSZ_WINFO	(32)	/* 64bit aligned */
#else
#if defined(NUM_FLGID)
#define TCBSZ_WINFO	(24)
#else
#if defined(NUM_MBFID)||defined(NUM_MPLID)
#define TCBSZ_WINFO	(16)
#else
#if defined(NUM_SEMID)||defined(NUM_MBXID)||defined(NUM_MPFID)
#define TCBSZ_WINFO	(8)
#else
#define TCBSZ_WINFO	(0)
#endif
#endif
#endif
#endif

/* AArch64: 64 bit CPU alignment */
#define _ALIGN_CPU(x)	(((x)+7)&0xFFFFFFF8)


/*----------------------------------------------------------------------*/
/*	machine dependent data						*/
/*----------------------------------------------------------------------*/
#define	TCB_winfo	(136)		/* tskque - wercd (64bit adjusted) */
#define	TCB_wtmeb	_ALIGN_CPU(TCB_winfo+TCBSZ_WINFO)
#define	TCBsz_wtmeb2istack	(56+TCBSZ_MTX+TCBSZ_POR+152)
					/* wtmeb - istack (64bit adjusted) */
#define TCBSZ_GP	(0)		/* No global pointer support	*/

/*----------------------------------------------------------------------*/
/*	offset data in TCB						*/
/*----------------------------------------------------------------------*/
#define TCB_isstack	(TCB_wtmeb+TCBsz_wtmeb2istack)
#define TCB_tskctxb	_ALIGN_CPU(TCB_isstack+8+TCBSZ_GP)

#define TCB_tskid	16
#define TCB_tskatr	32
#define TCB_state	86
#define TCB_reqdct	72
#define TA_FPU		0

/*----------------------------------------------------------------------*/
/*	offset data in CTXB						*/
/*----------------------------------------------------------------------*/
#define CTXB_ssp	0		/* System stack pointer */
#define CTXB_uatb	8		/* User address translation base */
#define CTXB_lsid	16		/* Logical space ID */
#define CTXB_svc_ssp	24		/* SSP when SVC is called */
#define CTXB_spsr	32		/* Saved PSTATE */

/*----------------------------------------------------------------------*/
/*	offset data in SStackFrame					*/
/*----------------------------------------------------------------------*/
#define SSF_X0		0		/* X0 */
#define SSF_X1		8		/* X1 */
#define SSF_X2		16		/* X2 */
#define SSF_X3		24		/* X3 */
#define SSF_X4		32		/* X4 */
#define SSF_X5		40		/* X5 */
#define SSF_X6		48		/* X6 */
#define SSF_X7		56		/* X7 */
#define SSF_X8		64		/* X8 */
#define SSF_X9		72		/* X9 */
#define SSF_X10		80		/* X10 */
#define SSF_X11		88		/* X11 */
#define SSF_X12		96		/* X12 */
#define SSF_X13		104		/* X13 */
#define SSF_X14		112		/* X14 */
#define SSF_X15		120		/* X15 */
#define SSF_X16		128		/* X16 */
#define SSF_X17		136		/* X17 */
#define SSF_X18		144		/* X18 */
#define SSF_X19		152		/* X19 */
#define SSF_X20		160		/* X20 */
#define SSF_X21		168		/* X21 */
#define SSF_X22		176		/* X22 */
#define SSF_X23		184		/* X23 */
#define SSF_X24		192		/* X24 */
#define SSF_X25		200		/* X25 */
#define SSF_X26		208		/* X26 */
#define SSF_X27		216		/* X27 */
#define SSF_X28		224		/* X28 */
#define SSF_X29		232		/* X29 (FP) */
#define SSF_X30		240		/* X30 (LR) */
#define SSF_SP		248		/* Stack pointer */
#define SSF_PC		256		/* Program counter (ELR_EL1) */
#define SSF_SPSR	264		/* SPSR_EL1 */
#define SSF_TASKMODE	272		/* Task mode */
#define SSF_SIZE	280		/* Total size */

#endif /* _OFFSET_ */
