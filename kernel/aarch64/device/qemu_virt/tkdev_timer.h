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
 *	tkdev_timer.h (QEMU virt - AArch64)
 *	Hardware-Dependent Timer Processing
 */

#ifndef _TKDEV_TIMER_
#define _TKDEV_TIMER_

#include <tk/syslib.h>
#include <sys/sysinfo.h>
#include "tkdev_conf.h"

/*
 * Timer configuration
 *	Uses ARM Generic Timer (virtual timer)
 */

/* Timer counter frequency (read from CNTFRQ_EL0) */
IMPORT UW timer_freq;

/* Timer interval in microseconds */
#define TIMER_PERIOD_US		1000	/* 1ms = 1000us */

/*
 * Settable interval range (microsecond)
 */
#define MIN_TIMER_PERIOD	10
#define MAX_TIMER_PERIOD	50000

/*
 * Set timer
 *	Set next interrupt occurrence time
 */
Inline void init_hw_timer( void )
{
	UW freq;
	UW tval;

	/* Get timer frequency */
	Asm("mrs %0, cntfrq_el0": "=r"(freq));
	timer_freq = freq;

	/* Calculate timer value for TIMER_PERIOD_US */
	tval = (freq / 1000000) * TIMER_PERIOD_US;

	/* Set timer value */
	Asm("msr cntv_tval_el0, %0":: "r"(tval));

	/* Enable virtual timer, unmask interrupt */
	Asm("msr cntv_ctl_el0, %0":: "r"(1));
}

/*
 * Timer start processing
 *	Initialize timer and start periodic timer interrupt
 */
Inline void start_hw_timer( void )
{
	/* Initialize GIC for timer interrupt */
	volatile UW *gicd_isenabler = (volatile UW *)GICD_ISENABLER(TIMER_IRQ / 32);
	volatile UW *gicd_ipriority = (volatile UW *)GICD_IPRIORITYR(TIMER_IRQ / 4);
	volatile UW *gicc_pmr = (volatile UW *)GICC_PMR;
	volatile UW *gicc_ctlr = (volatile UW *)GICC_CTLR;
	volatile UW *gicd_ctlr = (volatile UW *)GICD_CTLR;

	/* Enable GIC Distributor */
	*gicd_ctlr = 1;

	/* Enable GIC CPU Interface */
	*gicc_ctlr = 1;

	/* Set priority mask to allow all priorities */
	*gicc_pmr = 0xFF;

	/* Set timer interrupt priority */
	*gicd_ipriority |= (0x80 << ((TIMER_IRQ % 4) * 8));

	/* Enable timer interrupt */
	*gicd_isenabler |= (1 << (TIMER_IRQ % 32));

	/* Initialize and start timer */
	init_hw_timer();
}

/*
 * Clear timer interrupt
 *	Clear interrupt and set next interrupt timing
 */
Inline void clear_hw_timer_interrupt( void )
{
	UW tval;

	/* Calculate next timer value */
	tval = (timer_freq / 1000000) * TIMER_PERIOD_US;

	/* Set timer value for next interrupt */
	Asm("msr cntv_tval_el0, %0":: "r"(tval));
}

/*
 * Timer stop processing
 *	Stop timer operation
 */
Inline void terminate_hw_timer( void )
{
	/* Disable virtual timer */
	Asm("msr cntv_ctl_el0, %0":: "r"(0));

	/* Disable timer interrupt in GIC */
	volatile UW *gicd_icenabler = (volatile UW *)GICD_ICENABLER(TIMER_IRQ / 32);
	*gicd_icenabler |= (1 << (TIMER_IRQ % 32));
}

/*
 * Get current time (microseconds)
 */
Inline UW get_hw_timer_usec( void )
{
	UW cnt;
	Asm("mrs %0, cntvct_el0": "=r"(cnt));
	return (cnt * 1000000) / timer_freq;
}

#endif /* _TKDEV_TIMER_ */
