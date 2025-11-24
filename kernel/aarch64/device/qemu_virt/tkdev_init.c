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
 *    Modified for AArch64 QEMU virt (ReTron OS) at 2024.
 *
 *----------------------------------------------------------------------
 */

/*
 *	tkdev_init.c (QEMU virt - AArch64)
 *	Device-Dependent Initialization/Finalization
 */

#include "kernel.h"
#include <tk/sysdef.h>
#include "tkdev_conf.h"
#include "tkdev_timer.h"

/*
 * Target system-dependent initialization
 */
EXPORT ER tkdev_initialize( void )
{
	/* Initialize GIC */
	volatile UW *gicd_ctlr = (volatile UW *)GICD_CTLR;
	volatile UW *gicc_ctlr = (volatile UW *)GICC_CTLR;
	volatile UW *gicc_pmr = (volatile UW *)GICC_PMR;

	/* Disable GIC */
	*gicd_ctlr = 0;
	*gicc_ctlr = 0;

	/* Set priority mask */
	*gicc_pmr = 0xFF;

	/* Enable GIC */
	*gicd_ctlr = 1;
	*gicc_ctlr = 1;

	return E_OK;
}

/*
 * Target system-dependent finalization
 */
EXPORT void tkdev_exit( void )
{
	/* Disable interrupts */
	volatile UW *gicd_ctlr = (volatile UW *)GICD_CTLR;
	volatile UW *gicc_ctlr = (volatile UW *)GICC_CTLR;

	*gicd_ctlr = 0;
	*gicc_ctlr = 0;
}

/*
 * Interrupt handler definition
 */
EXPORT ER define_inthdr( INT vecno, FP inthdr )
{
	/* For simplicity, use a static handler table */
	/* In a full implementation, this would manage the vector table */
	(void)vecno;
	(void)inthdr;
	return E_OK;
}

/*
 * Enable interrupt
 */
EXPORT void EnableInt( UINT intno )
{
	volatile UW *gicd_isenabler = (volatile UW *)GICD_ISENABLER(intno / 32);
	*gicd_isenabler |= (1 << (intno % 32));
}

/*
 * Disable interrupt
 */
EXPORT void DisableInt( UINT intno )
{
	volatile UW *gicd_icenabler = (volatile UW *)GICD_ICENABLER(intno / 32);
	*gicd_icenabler |= (1 << (intno % 32));
}

/*
 * Clear interrupt
 */
EXPORT void ClearInt( UINT intno )
{
	volatile UW *gicd_icpendr = (volatile UW *)GICD_ICPENDR(intno / 32);
	*gicd_icpendr |= (1 << (intno % 32));
}

/*
 * Check interrupt status
 */
EXPORT BOOL CheckInt( UINT intno )
{
	volatile UW *gicd_ispendr = (volatile UW *)GICD_ISPENDR(intno / 32);
	return (*gicd_ispendr & (1 << (intno % 32))) != 0;
}

/*
 * End of interrupt
 */
EXPORT void EndOfInt( UINT intno )
{
	volatile UW *gicc_eoir = (volatile UW *)GICC_EOIR;
	*gicc_eoir = intno;
}

/*
 * Get current interrupt
 */
EXPORT UINT GetCurrentInt( void )
{
	volatile UW *gicc_iar = (volatile UW *)GICC_IAR;
	return *gicc_iar & 0x3FF;
}
