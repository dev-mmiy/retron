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
 *	devinit.c (QEMU virt - AArch64)
 *	Device-Dependent Initialization
 */

#include "kernel.h"
#include <tk/sysdef.h>
#include <sys/sysinfo.h>
#include "tkdev_conf.h"

/*
 * UART output for debugging
 */
LOCAL void uart_putc(INT c)
{
	volatile UW *fr = (volatile UW *)UART_FR;
	volatile UW *dr = (volatile UW *)UART_DR;

	/* Wait until TX FIFO is not full */
	while (*fr & UART_FR_TXFF);

	/* Write character */
	*dr = (UW)c;
}

LOCAL void uart_puts(const char *s)
{
	while (*s) {
		if (*s == '\n') {
			uart_putc('\r');
		}
		uart_putc(*s++);
	}
}

/*
 * Display the progress of start processing
 */
EXPORT void DispProgress( W n )
{
	char buf[16];
	int i = 0;

	buf[i++] = '[';
	buf[i++] = '0' + (n >> 4);
	buf[i++] = '0' + (n & 0xf);
	buf[i++] = ']';
	buf[i++] = '\n';
	buf[i] = '\0';

	uart_puts(buf);
}

/* ------------------------------------------------------------------------ */

/*
 * Initialization at ROM startup
 */
EXPORT ER ROM_startup( void )
{
	/* Initialize UART */
	volatile UW *cr = (volatile UW *)UART_CR;
	volatile UW *lcr = (volatile UW *)UART_LCR_H;
	volatile UW *imsc = (volatile UW *)UART_IMSC;

	/* Disable UART */
	*cr = 0;

	/* Set 8N1, enable FIFOs */
	*lcr = (3 << 5) | (1 << 4);  /* 8 bits, FIFO enable */

	/* Disable all interrupts */
	*imsc = 0;

	/* Enable UART, TX, RX */
	*cr = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;

	uart_puts("\n=== ReTron OS (T-Kernel/AArch64) ===\n");
	uart_puts("ROM_startup: UART initialized\n");

	return E_OK;
}

/* ------------------------------------------------------------------------ */

/*
 * Initialization before T-Kernel starts
 */
EXPORT ER init_device( void )
{
	uart_puts("init_device: Starting device initialization\n");

	/* Nothing special needed for QEMU virt */

	uart_puts("init_device: Complete\n");
	return E_OK;
}

/* ------------------------------------------------------------------------ */
/*
 * Start processing after T-Kernel starts
 */
EXPORT ER start_device( void )
{
	uart_puts("start_device: T-Kernel started successfully\n");
	return E_OK;
}

/* ------------------------------------------------------------------------ */
/*
 * System finalization
 */
EXPORT ER finish_device( void )
{
	uart_puts("finish_device: System shutdown\n");
	return E_OK;
}

/* ------------------------------------------------------------------------ */
/*
 * Re-starting processing
 */
EXPORT ER restart_device( W mode )
{
	if ( mode == -1 ) {
		/* Reset and re-start (cold boot) */
		uart_puts("\n<< SYSTEM RESTART >>\n");
		/* QEMU: Use PSCI or loop forever */
		for (;;) {
			Asm("wfi":::);
		}
		return E_OBJ;
	}

	return E_NOSPT;
}

/* ------------------------------------------------------------------------ */
/*
 * Low-level debug output (tm_printf support)
 */
EXPORT void tm_putchar( INT c )
{
	if (c == '\n') {
		uart_putc('\r');
	}
	uart_putc(c);
}

EXPORT void tm_putstring( const UB *str )
{
	uart_puts((const char *)str);
}
