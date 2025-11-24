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
 *	cpu_init.c (AArch64)
 *	CPU-Dependent Initialization/Finalization
 */

#include "kernel.h"
#include "task.h"
#include "cpu_insn.h"

EXPORT ATR	available_cop;	/* Available coprocessor (FPU/SIMD) */

/* Exception vector table (defined in cpu_support.S) */
IMPORT void exception_vector_table( void );

/*
 * CPU-dependent initialization
 */
EXPORT ER cpu_initialize( void )
{
	UW	r;

	/* Initialize task space */
#if USE_MMU
	/* Copy TTBR1_EL1 to TTBR0_EL1 for initial state */
	Asm("mrs %0, ttbr1_el1": "=r"(r));
	Asm("msr ttbr0_el1, %0":: "r"(r));
	/* Clear CONTEXTIDR_EL1 */
	Asm("msr contextidr_el1, %0":: "r"(0));
	ISB();
	/* Invalidate TLB */
	PurgeTLB();
#endif

	/* available coprocessor(s) - FPU/SIMD for AArch64 */
	available_cop = TA_NULL;

	/* Enable FPU/SIMD access */
	Asm("mrs %0, cpacr_el1": "=r"(r));
	r |= (3 << 20);  /* FPEN: Enable FP/SIMD at EL0 and EL1 */
	Asm("msr cpacr_el1, %0":: "r"(r));
	ISB();

	/* Set exception vector base address */
	Asm("msr vbar_el1, %0":: "r"(exception_vector_table));
	ISB();

	return E_OK;
}

/*
 * CPU-dependent finalization
 */
EXPORT void cpu_shutdown( void )
{
	/* Nothing to do for now */
}

/* ------------------------------------------------------------------------- */

/*
 * Task exception handler startup reservation
 */
EXPORT void request_tex( TCB *tcb )
{
	/* Cannot set to the task operating at protected level 0 */
	if ( tcb->isysmode == 0 ) {
		tcb->reqdct = 1;
	}
}

/*
 * Setting up the start of task exception handler
 *
 *	Initial stack status (AArch64)
 *		system stack			user stack
 *		+---------------+		+---------------+
 *	ssp ->	| X0-X30	|	usp ->	| (xxxxxxxxxxx) |
 *		| SP		|		|		|
 *		| ELR_EL1	|
 *		| SPSR_EL1	|
 *		| taskmode	|
 *		+---------------+
 *
 *	Modified stack status ( modified parts are marked with * )
 *		+---------------+		+---------------+
 *	ssp ->	| X0 = texcd	|*	usp* -> | texcd		|*
 *		| X1-X29	|		| retadr	|*
 *		| X30 = texhdr	|*		| PSTATE	|*
 *		| SP		|		+---------------+
 *		| ELR_EL1	|*		| (xxxxxxxxxxx) |
 *		| SPSR_EL1	|
 *		| taskmode	|
 *		+---------------+
 */
EXPORT void setup_texhdr( UW *ssp )
{
	FP	texhdr;
	INT	texcd;
	UINT	m;
	UW	*usp;

	/* called in interrupt-disabled state */

	ctxtsk->reqdct = 0;	/* release DCT */

	/* obtain exception code */
	m = 0x00000001;
	for ( texcd = 0; texcd <= 31; texcd++ ) {
		if ( (ctxtsk->exectex & m) != 0 ) break;
		m <<= 1;
	}
	if ( texcd > 31 ) return; /* exception is not generated / released */

	ctxtsk->exectex = 0;
	ctxtsk->pendtex &= ~m;
	ctxtsk->texflg |= ( texcd == 0 )? TEX0_RUNNING: TEX1_RUNNING;
	texhdr = ctxtsk->texhdr;

	/* obtain user stack pointer from saved SP */
	usp = (UW *)ssp[31];  /* SP is at offset 31 in SStackFrame */

	/* reset user stack to the initial value if exception code is 0 */
	if ( texcd == 0 ) usp = ctxtsk->istack;

	usp -= 3;

	/* set up user stack pointer */
	ssp[31] = (UW)usp;

	ENABLE_INTERRUPT;

	/* adjust stack
	 *	we need to access user stack, and this may cause
	 *	a page fault.
	 */
	*(usp + 0) = texcd;
	*(usp + 1) = ssp[32];		/* retadr (ELR_EL1) */
	*(usp + 2) = ssp[33];		/* PSTATE (SPSR_EL1) */
	ssp[0] = texcd;			/* X0 = texcd */
	ssp[30] = (UW)texhdr;		/* X30 (LR) = texhdr */
	ssp[32] = (UW)texhdr;		/* ELR_EL1 = texhdr */
}
