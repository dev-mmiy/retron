/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *    Modified for AArch64 QEMU virt (ReTron OS) at 2024.
 *
 *----------------------------------------------------------------------
 */

/*
 *	kernel_main.c (QEMU virt - AArch64)
 *	Kernel Entry Point
 */

#include "tkdev_conf.h"

/* Simple type definitions for standalone build */
typedef unsigned int		UW;
typedef int			INT;
typedef int			ER;

#define E_OK			0

/*
 * UART output functions
 */
static void uart_putc(INT c)
{
	volatile UW *fr = (volatile UW *)UART_FR;
	volatile UW *dr = (volatile UW *)UART_DR;

	/* Wait until TX FIFO is not full */
	while (*fr & UART_FR_TXFF);

	/* Write character */
	*dr = (UW)c;
}

static void uart_puts(const char *s)
{
	while (*s) {
		if (*s == '\n') {
			uart_putc('\r');
		}
		uart_putc(*s++);
	}
}

static void uart_puthex(UW val)
{
	const char hex[] = "0123456789ABCDEF";
	int i;

	uart_puts("0x");
	for (i = 28; i >= 0; i -= 4) {
		uart_putc(hex[(val >> i) & 0xF]);
	}
}

/*
 * Initialize UART
 */
static void uart_init(void)
{
	volatile UW *cr = (volatile UW *)UART_CR;
	volatile UW *lcr = (volatile UW *)UART_LCR_H;
	volatile UW *imsc = (volatile UW *)UART_IMSC;

	/* Disable UART */
	*cr = 0;

	/* Set 8N1, enable FIFOs */
	*lcr = (3 << 5) | (1 << 4);

	/* Disable all interrupts */
	*imsc = 0;

	/* Enable UART, TX, RX */
	*cr = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

/*
 * Kernel main entry point
 *	Called from startup assembly (icrt0.S)
 */
void kernel_main(void)
{
	UW cnt;

	/* Initialize UART */
	uart_init();

	/* Print banner */
	uart_puts("\n");
	uart_puts("========================================\n");
	uart_puts("  ReTron OS - T-Kernel 2.0 for AArch64\n");
	uart_puts("  Running on QEMU virt machine\n");
	uart_puts("========================================\n");
	uart_puts("\n");

	/* Print system information */
	uart_puts("Hello World from T-Kernel!\n");
	uart_puts("\n");

	/* Read and display timer frequency */
	__asm__ volatile("mrs %0, cntfrq_el0" : "=r"(cnt));
	uart_puts("Timer frequency: ");
	uart_puthex(cnt);
	uart_puts(" Hz\n");

	/* Read current exception level */
	__asm__ volatile("mrs %0, CurrentEL" : "=r"(cnt));
	uart_puts("Current EL: ");
	uart_puthex((cnt >> 2) & 0x3);
	uart_puts("\n");

	/* Read MIDR_EL1 for CPU identification */
	__asm__ volatile("mrs %0, midr_el1" : "=r"(cnt));
	uart_puts("CPU ID (MIDR_EL1): ");
	uart_puthex(cnt);
	uart_puts("\n");

	uart_puts("\n");
	uart_puts("T-Kernel initialization complete.\n");
	uart_puts("System ready.\n");
	uart_puts("\n");

	/* Main loop - in a real system, this would start the scheduler */
	uart_puts("Entering idle loop...\n");
	for (;;) {
		__asm__ volatile("wfi");
	}
}

/*
 * Exception handlers (weak implementations)
 */
void sync_exception_handler(void *sp)
{
	(void)sp;
	uart_puts("SYNC EXCEPTION!\n");
	for (;;) __asm__ volatile("wfi");
}

void irq_exception_handler(void *sp)
{
	(void)sp;
	uart_puts("IRQ!\n");
}

void fiq_exception_handler(void *sp)
{
	(void)sp;
	uart_puts("FIQ!\n");
}

void serror_exception_handler(void *sp)
{
	(void)sp;
	uart_puts("SERROR!\n");
	for (;;) __asm__ volatile("wfi");
}
