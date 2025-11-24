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
 *	cpu_insn.h (AArch64)
 *	AArch64 Dependent Operation
 */

#ifndef _CPU_INSN_
#define _CPU_INSN_

#include <sys/sysinfo.h>

/* ------------------------------------------------------------------------ */
/*
 *	Memory barrier operations
 */

/* Data Synchronization Barrier */
#define DSB()	Asm("dsb sy":::"memory")

/* Data Memory Barrier */
#define DMB()	Asm("dmb sy":::"memory")

/* Instruction Synchronization Barrier */
#define ISB()	Asm("isb":::"memory")

/* ------------------------------------------------------------------------ */
/*
 *	Control register operation
 */

/*
 * Get DAIF (Interrupt Mask Flags)
 */
Inline UINT getDAIF( void )
{
	UINT	daif;
	Asm("mrs %0, daif": "=r"(daif));
	return daif;
}

/*
 * Set DAIF
 */
Inline void setDAIF( UINT daif )
{
	Asm("msr daif, %0":: "r"(daif));
}

/*
 * Get Current Exception Level
 */
Inline UINT getCurrentEL( void )
{
	UINT	el;
	Asm("mrs %0, CurrentEL": "=r"(el));
	return (el >> 2) & 0x3;
}

/*
 * Get NZCV (Condition Flags)
 */
Inline UINT getNZCV( void )
{
	UINT	nzcv;
	Asm("mrs %0, nzcv": "=r"(nzcv));
	return nzcv;
}

/*
 * Disable interrupts (set I and F bits in DAIF)
 */
Inline UINT disableInt( void )
{
	UINT	daif;
	Asm("mrs %0, daif": "=r"(daif));
	Asm("msr daifset, #3"::);  /* Set I and F bits */
	return daif;
}

/*
 * Enable interrupts (clear I and F bits in DAIF)
 */
Inline void enableInt( UINT daif )
{
	Asm("msr daif, %0":: "r"(daif));
}

/*
 * TLB invalidate all
 */
Inline void PurgeTLB( void )
{
	Asm("tlbi vmalle1"::);
	DSB();
	ISB();
}

/*
 * TLB invalidate by VA
 */
Inline void PurgeTLBbyVA( void *va )
{
	Asm("tlbi vae1, %0":: "r"((UW)va >> 12));
	DSB();
	ISB();
}

/* ------------------------------------------------------------------------ */
/*
 *	Cache operations
 */

/*
 * Invalidate instruction cache
 */
Inline void InvalidateICache( void )
{
	Asm("ic iallu"::);
	DSB();
	ISB();
}

/*
 * Invalidate data cache by VA
 */
Inline void InvalidateDCacheByVA( void *va )
{
	Asm("dc ivac, %0":: "r"(va));
	DSB();
}

/*
 * Clean data cache by VA
 */
Inline void CleanDCacheByVA( void *va )
{
	Asm("dc cvac, %0":: "r"(va));
	DSB();
}

/*
 * Clean and invalidate data cache by VA
 */
Inline void CleanInvalidateDCacheByVA( void *va )
{
	Asm("dc civac, %0":: "r"(va));
	DSB();
}

/* ------------------------------------------------------------------------ */
/*
 *	EIT-related (Exception, Interrupt, Trap)
 */

/*
 * Vector numbers for AArch64
 */
#define VECNO_SYNC_EL1		0	/* Synchronous exception from EL1 */
#define VECNO_IRQ_EL1		1	/* IRQ from EL1 */
#define VECNO_FIQ_EL1		2	/* FIQ from EL1 */
#define VECNO_SERROR_EL1	3	/* SError from EL1 */
#define VECNO_SYNC_EL0		4	/* Synchronous exception from EL0 */
#define VECNO_IRQ_EL0		5	/* IRQ from EL0 */
#define VECNO_FIQ_EL0		6	/* FIQ from EL0 */
#define VECNO_SERROR_EL0	7	/* SError from EL0 */

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

/* ------------------------------------------------------------------------ */
/*
 *	Spin lock (for SMP support - placeholder)
 */

Inline void SpinLock( UINT *lock )
{
	/* For single core, no operation needed */
	(void)lock;
}

Inline void SpinUnlock( UINT *lock )
{
	/* For single core, no operation needed */
	(void)lock;
}

/* ------------------------------------------------------------------------ */

#endif /* _CPU_INSN_ */
