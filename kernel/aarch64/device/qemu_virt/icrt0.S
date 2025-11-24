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
 *	icrt0.S (QEMU virt - AArch64)
 *	System Startup
 */

	.section .text.boot
	.balign	8
	.globl	_start
	.type	_start, %function

_start:
	/*
	 * QEMU virt loads kernel at 0x40000000 and jumps here
	 * CPU is in EL1 (or EL2 if -machine virt,virtualization=on)
	 */

	/* Disable interrupts */
	msr	daifset, #0xf

	/* Check current exception level */
	mrs	x0, CurrentEL
	and	x0, x0, #0xc
	cmp	x0, #0x8		/* EL2? */
	b.eq	el2_entry
	cmp	x0, #0x4		/* EL1? */
	b.eq	el1_entry
	/* EL3 not expected from QEMU */
	b	.

el2_entry:
	/* Drop to EL1 */
	/* Configure EL2 to pass through to EL1 */
	mov	x0, #(1 << 31)		/* HCR_EL2.RW = 1 (AArch64 at EL1) */
	msr	hcr_el2, x0

	/* Set SPSR_EL2 for return to EL1h */
	mov	x0, #0x3c5		/* DAIF masked, EL1h */
	msr	spsr_el2, x0

	/* Set return address to EL1 entry */
	adr	x0, el1_entry
	msr	elr_el2, x0

	eret

el1_entry:
	/* Now running at EL1 */

	/* Set up stack pointer */
	ldr	x0, =_stack_top
	mov	sp, x0

	/* Clear BSS section */
	ldr	x0, =__bss_start
	ldr	x1, =__bss_end
	mov	x2, #0
bss_clear:
	cmp	x0, x1
	b.ge	bss_done
	str	x2, [x0], #8
	b	bss_clear
bss_done:

	/* Set up exception vector table */
	ldr	x0, =exception_vector_table
	msr	vbar_el1, x0
	isb

	/* Enable FPU/SIMD */
	mov	x0, #(3 << 20)
	msr	cpacr_el1, x0
	isb

	/* Initialize MMU (disabled for now) */
	/* TODO: Set up page tables if needed */

	/* Call C entry point */
	bl	kernel_main

	/* Should not return, but if it does, halt */
halt:
	wfi
	b	halt

/* ------------------------------------------------------------------------ */
/*
 * Stack area
 */
	.section .bss
	.balign	16
	.globl	_stack_bottom
_stack_bottom:
	.space	16384		/* 16KB stack */
	.globl	_stack_top
_stack_top:

/* ------------------------------------------------------------------------ */
/*
 * Symbols for BSS clearing (defined by linker script)
 */
	.weak	__bss_start
	.weak	__bss_end
	.weak	exception_vector_table

/* ------------------------------------------------------------------------ */
/*
 * Timer frequency variable
 */
	.data
	.balign	8
	.globl	timer_freq
timer_freq:
	.quad	0
