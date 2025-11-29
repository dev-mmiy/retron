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

/*
 * For standalone test kernel, define sizes directly.
 * In full T-Kernel build, these would come from config.h
 */
#define TCBSZ_POR	(0)	/* No rendezvous support for now */
#define TCBSZ_MTX	(0)	/* No mutex support for now */
#define TCBSZ_WINFO	(0)	/* Minimal wait info */

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
/*	offset data in TCB (simplified TCB layout for standalone kernel)	*/
/*----------------------------------------------------------------------*/
/* Simplified TCB structure:
 * typedef struct {
 *   ID      tskid;       // offset 0, 4 bytes
 *   TSTAT   state;       // offset 4, 4 bytes
 *   TASK_FP task;        // offset 8, 8 bytes
 *   void    *exinf;      // offset 16, 8 bytes
 *   INT     priority;    // offset 24, 4 bytes
 *   void    *stack;      // offset 32, 8 bytes (aligned)
 *   INT     stksz;       // offset 40, 4 bytes
 *   CTXB    tskctxb;     // offset 48, 8 bytes (aligned)
 *   void    *isstack;    // offset 56, 8 bytes
 * } TCB;
 */
#define TCB_tskid	0
#define TCB_state	4
#define TCB_task	8
#define TCB_exinf	16
#define TCB_priority	24
#define TCB_stack	32
#define TCB_stksz	40
#define TCB_tskctxb	48
#define TCB_isstack	56

#define TCB_tskatr	32	/* Not used in simplified TCB */
#define TCB_reqdct	72	/* Not used in simplified TCB */
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
