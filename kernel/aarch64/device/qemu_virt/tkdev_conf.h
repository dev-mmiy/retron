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
 *	tkdev_conf.h (QEMU virt - AArch64)
 *	Target System Configuration
 */

#ifndef _TKDEV_CONF_
#define _TKDEV_CONF_
/* Also included from assembler source */

/*
 * QEMU virt machine memory map (AArch64)
 *	0x00000000 - 0x08000000 : Flash (128MB)
 *	0x08000000 - 0x08001000 : GIC Distributor
 *	0x08010000 - 0x08020000 : GIC CPU Interface
 *	0x09000000 - 0x09001000 : UART0 (PL011)
 *	0x09010000 - 0x09011000 : RTC
 *	0x09030000 - 0x09031000 : GPIO
 *	0x0a000000 - 0x0a000200 : VirtIO (multiple devices)
 *	0x10000000 - 0x3effffff : PCIe MMIO
 *	0x40000000 - 0xffffffff : RAM (up to 3GB when using 32-bit addressing)
 */

/*
 * UART (PL011)
 */
#define	UART_BASE		0x09000000
#define	UART_DR			(UART_BASE + 0x000)	/* Data Register */
#define	UART_FR			(UART_BASE + 0x018)	/* Flag Register */
#define	UART_IBRD		(UART_BASE + 0x024)	/* Integer Baud Rate */
#define	UART_FBRD		(UART_BASE + 0x028)	/* Fractional Baud Rate */
#define	UART_LCR_H		(UART_BASE + 0x02C)	/* Line Control Register */
#define	UART_CR			(UART_BASE + 0x030)	/* Control Register */
#define	UART_IMSC		(UART_BASE + 0x038)	/* Interrupt Mask */
#define	UART_ICR		(UART_BASE + 0x044)	/* Interrupt Clear */

/* UART Flag Register bits */
#define	UART_FR_TXFF		(1 << 5)	/* TX FIFO Full */
#define	UART_FR_RXFE		(1 << 4)	/* RX FIFO Empty */
#define	UART_FR_BUSY		(1 << 3)	/* UART Busy */

/* UART Control Register bits */
#define	UART_CR_RXE		(1 << 9)	/* Receive Enable */
#define	UART_CR_TXE		(1 << 8)	/* Transmit Enable */
#define	UART_CR_UARTEN		(1 << 0)	/* UART Enable */

/*
 * GIC (Generic Interrupt Controller) v2
 */
#define	GICD_BASE		0x08000000	/* Distributor */
#define	GICC_BASE		0x08010000	/* CPU Interface */

/* GIC Distributor registers */
#define	GICD_CTLR		(GICD_BASE + 0x000)
#define	GICD_TYPER		(GICD_BASE + 0x004)
#define	GICD_ISENABLER(n)	(GICD_BASE + 0x100 + (n) * 4)
#define	GICD_ICENABLER(n)	(GICD_BASE + 0x180 + (n) * 4)
#define	GICD_ISPENDR(n)		(GICD_BASE + 0x200 + (n) * 4)
#define	GICD_ICPENDR(n)		(GICD_BASE + 0x280 + (n) * 4)
#define	GICD_IPRIORITYR(n)	(GICD_BASE + 0x400 + (n) * 4)
#define	GICD_ITARGETSR(n)	(GICD_BASE + 0x800 + (n) * 4)
#define	GICD_ICFGR(n)		(GICD_BASE + 0xC00 + (n) * 4)

/* GIC CPU Interface registers */
#define	GICC_CTLR		(GICC_BASE + 0x000)
#define	GICC_PMR		(GICC_BASE + 0x004)
#define	GICC_IAR		(GICC_BASE + 0x00C)
#define	GICC_EOIR		(GICC_BASE + 0x010)

/*
 * Timer (ARM Generic Timer)
 *	Uses the virtual timer (CNTVCT_EL0, CNTV_CTL_EL0, CNTV_CVAL_EL0)
 */
#define	TIMER_IRQ		27	/* Virtual timer PPI */
#define	VECNO_TIMER		TIMER_IRQ

/* Timer interrupt level */
#define	TIMER_INTLEVEL		0

/*
 * Memory Configuration
 */
#define	RAM_BASE		0x40000000
#define	RAM_SIZE		0x08000000	/* 128MB default */

/*
 * Kernel load address
 */
#define	KERNEL_BASE		0x40000000

/*
 * Exception vector numbers
 *	SPI (Shared Peripheral Interrupt): 32-1019
 *	PPI (Private Peripheral Interrupt): 16-31
 *	SGI (Software Generated Interrupt): 0-15
 */
#define	N_INTVEC		256	/* Number of interrupt vectors */

#define	EIT_DEFAULT		0
#define	EIT_IRQ(n)		(n)

#endif /* _TKDEV_CONF_ */
