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
 *	cpu_task.h (AArch64)
 *	CPU-Dependent Task Start Processing
 */

#ifndef _CPU_TASK_
#define _CPU_TASK_

#include "cpu_insn.h"

/*
 * System stack configuration at task startup
 * AArch64 has 31 general-purpose registers (X0-X30)
 * X30 is the link register (LR)
 * SP is separate
 */
typedef struct {
	UW	x[31];		/* X0-X30 (X30 = LR) */
	UW	sp;		/* Stack pointer (SP_EL0 or SP_ELx) */
	UW	pc;		/* Program counter (ELR_EL1) */
	UW	spsr;		/* Saved Program Status Register (SPSR_EL1) */
	UW	taskmode;	/* Task mode */
} SStackFrame;

/*
 * User stack configuration at task startup (only RNG 1-3)
 */
typedef struct {
	/* Empty */
} UStackFrame;

/*
 * Size of system stack area destroyed by 'make_dormant()'
 * In other words, the size of area required to write by 'setup_context().'
 */
#define DORMANT_STACK_SIZE	( sizeof(SStackFrame) )

/*
 * Size of area kept for special use from system stack
 */
#define RESERVE_SSTACK(tskatr)	0

/*
 * SPSR initial values for AArch64
 * M[4:0] = 0b00100 = EL1h (EL1 with SP_EL1)
 * M[4:0] = 0b00000 = EL0t (EL0 with SP_EL0)
 */
#define SPSR_EL1H	0x00000004	/* EL1h mode */
#define SPSR_EL0T	0x00000000	/* EL0t mode */
#define SPSR_I_BIT	(1 << 7)	/* IRQ mask */
#define SPSR_F_BIT	(1 << 6)	/* FIQ mask */

/*
 * Initial value for task startup
 */
#if USE_MMU
#define INIT_SPSR(rng)	( ( (rng) == 0 )? SPSR_EL1H: SPSR_EL0T )
#else
#define INIT_SPSR(rng)	SPSR_EL1H
#endif

#define INIT_TMF(rng)	( TMF_PPL(rng) | TMF_CPL(rng) )

/*
 * Switch task space
 */
Inline void change_space( void *uatb, INT lsid )
{
	UW	ttbr;

	/* if no task space to switch to is not specified, use system default. */
	Asm("mrs %0, ttbr1_el1": "=r"(ttbr));
	if ( uatb != NULL ) {
		ttbr = (UW)uatb | (ttbr & 0x0fff);
	}

	/* Set ASID and TTBR */
	Asm("msr ttbr0_el1, %0":: "r"(ttbr));
	/* Set ASID in CONTEXTIDR_EL1 */
	Asm("msr contextidr_el1, %0":: "r"(lsid));
	ISB();
}

/*
 * Create stack frame for task startup
 *	Call from 'make_dormant()'
 */
Inline void setup_context( TCB *tcb )
{
	SStackFrame	*ssp;
	W		rng;
	UW		pc, spsr;

	rng = (tcb->tskatr & TA_RNG3) >> 8;
	ssp = tcb->isstack;
	ssp--;

	spsr = INIT_SPSR(rng);
	pc = (UW)tcb->task;

	/* CPU context initialization */
	/* Clear all registers */
	for (int i = 0; i < 31; i++) {
		ssp->x[i] = 0;
	}
	ssp->taskmode = INIT_TMF(rng);	/* Initial taskmode */
	ssp->spsr = spsr;		/* Initial SPSR */
	ssp->pc = pc;			/* Task startup address */
	ssp->sp = 0;			/* Will be set properly */
	tcb->tskctxb.ssp = ssp;		/* System stack */
	tcb->tskctxb.svc_ssp = NULL;	/* ssp when SVC is called */

	if ( rng > 0 ) {
		ssp->sp = (UW)tcb->istack;	/* User stack */
	}
}

/*
 * Set task startup code
 *	Called by 'tk_sta_tsk()' processing.
 */
Inline void setup_stacd( TCB *tcb, INT stacd )
{
	SStackFrame	*ssp = tcb->tskctxb.ssp;

	ssp->x[0] = stacd;		/* X0 = stacd */
	ssp->x[1] = (UW)tcb->exinf;	/* X1 = exinf */
}

/*
 * Delete task contexts
 */
Inline void cleanup_context( TCB *tcb )
{
}

#endif /* _CPU_TASK_ */
