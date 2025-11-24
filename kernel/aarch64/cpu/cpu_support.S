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
 *	cpu_support.S (AArch64)
 *
 *	CPU operation specific to AArch64
 */
#define	_in_asm_source_

#include "offset.h"

/*
 * Macros for AArch64
 */
.macro	SAVE_CONTEXT
	/* Save all general purpose registers */
	stp	x0,  x1,  [sp, #-16]!
	stp	x2,  x3,  [sp, #-16]!
	stp	x4,  x5,  [sp, #-16]!
	stp	x6,  x7,  [sp, #-16]!
	stp	x8,  x9,  [sp, #-16]!
	stp	x10, x11, [sp, #-16]!
	stp	x12, x13, [sp, #-16]!
	stp	x14, x15, [sp, #-16]!
	stp	x16, x17, [sp, #-16]!
	stp	x18, x19, [sp, #-16]!
	stp	x20, x21, [sp, #-16]!
	stp	x22, x23, [sp, #-16]!
	stp	x24, x25, [sp, #-16]!
	stp	x26, x27, [sp, #-16]!
	stp	x28, x29, [sp, #-16]!
	/* Save x30 (LR), SP, ELR_EL1, SPSR_EL1 */
	mrs	x0, elr_el1
	mrs	x1, spsr_el1
	stp	x30, x0,  [sp, #-16]!
	stp	x1,  xzr, [sp, #-16]!	/* SPSR + padding */
.endm

.macro	RESTORE_CONTEXT
	/* Restore SPSR_EL1 and ELR_EL1 */
	ldp	x0, x1, [sp], #16		/* SPSR + padding */
	msr	spsr_el1, x0
	ldp	x30, x0, [sp], #16		/* x30 (LR) + ELR */
	msr	elr_el1, x0
	/* Restore all general purpose registers */
	ldp	x28, x29, [sp], #16
	ldp	x26, x27, [sp], #16
	ldp	x24, x25, [sp], #16
	ldp	x22, x23, [sp], #16
	ldp	x20, x21, [sp], #16
	ldp	x18, x19, [sp], #16
	ldp	x16, x17, [sp], #16
	ldp	x14, x15, [sp], #16
	ldp	x12, x13, [sp], #16
	ldp	x10, x11, [sp], #16
	ldp	x8,  x9,  [sp], #16
	ldp	x6,  x7,  [sp], #16
	ldp	x4,  x5,  [sp], #16
	ldp	x2,  x3,  [sp], #16
	ldp	x0,  x1,  [sp], #16
.endm

/* ------------------------------------------------------------------------ */
/*
 * Exception Vector Table for AArch64
 * Must be aligned to 2KB (0x800)
 */
	.section .vectors, "ax"
	.balign	0x800
	.globl	exception_vector_table
exception_vector_table:

/* Current EL with SP0 */
	.balign	0x80
curr_el_sp0_sync:
	b	sync_handler

	.balign	0x80
curr_el_sp0_irq:
	b	irq_handler

	.balign	0x80
curr_el_sp0_fiq:
	b	fiq_handler

	.balign	0x80
curr_el_sp0_serror:
	b	serror_handler

/* Current EL with SPx */
	.balign	0x80
curr_el_spx_sync:
	b	sync_handler

	.balign	0x80
curr_el_spx_irq:
	b	irq_handler

	.balign	0x80
curr_el_spx_fiq:
	b	fiq_handler

	.balign	0x80
curr_el_spx_serror:
	b	serror_handler

/* Lower EL using AArch64 */
	.balign	0x80
lower_el_aarch64_sync:
	b	sync_handler

	.balign	0x80
lower_el_aarch64_irq:
	b	irq_handler

	.balign	0x80
lower_el_aarch64_fiq:
	b	fiq_handler

	.balign	0x80
lower_el_aarch64_serror:
	b	serror_handler

/* Lower EL using AArch32 */
	.balign	0x80
lower_el_aarch32_sync:
	b	sync_handler

	.balign	0x80
lower_el_aarch32_irq:
	b	irq_handler

	.balign	0x80
lower_el_aarch32_fiq:
	b	fiq_handler

	.balign	0x80
lower_el_aarch32_serror:
	b	serror_handler

/* ------------------------------------------------------------------------ */
/*
 * Exception Handlers
 */
	.text
	.balign	4

sync_handler:
	SAVE_CONTEXT
	mov	x0, sp
	bl	sync_exception_handler
	RESTORE_CONTEXT
	eret

irq_handler:
	SAVE_CONTEXT
	mov	x0, sp
	bl	irq_exception_handler
	RESTORE_CONTEXT
	eret

fiq_handler:
	SAVE_CONTEXT
	mov	x0, sp
	bl	fiq_exception_handler
	RESTORE_CONTEXT
	eret

serror_handler:
	SAVE_CONTEXT
	mov	x0, sp
	bl	serror_exception_handler
	RESTORE_CONTEXT
	eret

/* ------------------------------------------------------------------------ */
/*
 * Dispatcher
 *	dispatch_to_schedtsk:
 *		Discard current context and dispatch to schedtsk forcibly.
 *		Called when stack is not defined (ssp is undefined).
 *		Called when interrupt is disabled.
 *	dispatch_entry:
 *		Ordinary dispatch processing.
 *	_ret_int_dispatch:
 *		Called if tk_ret_int() requires dispatching.
 */

/* Temporary stack used when dispatch_to_schedtsk is called */
#define	TMP_STACK_SZ	(4*1024)
	.lcomm	tmp_stack, TMP_STACK_SZ
#define	TMP_STACK_TOP	(tmp_stack + TMP_STACK_SZ)

	.text
	.balign	4
	.globl	dispatch_to_schedtsk
	.type	dispatch_to_schedtsk, %function
	.globl	dispatch_entry
	.type	dispatch_entry, %function

dispatch_to_schedtsk:
	/* EL1 / interrupt-disabled state DAIF.I=1 */
	ldr	x0, =TMP_STACK_TOP		// set up a temporary stack
	mov	sp, x0

	ldr	x1, =dispatch_disabled
	mov	w2, #1
	str	w2, [x1]			// disable dispatch

	ldr	x4, =ctxtsk			// X4 = &ctxtsk
	ldr	x5, =TASKMODE
	mov	x0, #0
	str	x0, [x4]			// ctxtsk = NULL
	str	w0, [x5]			// taskmode = 0

	msr	daifclr, #0xf			// enable interrupt
	b	l_dispatch0

dispatch_entry:
	/* EL1 / interrupt-disabled state DAIF.I=1 */
	ldr	x1, =dispatch_disabled
	mov	w2, #1
	str	w2, [x1]			// disable dispatch

	msr	daifclr, #0xf			// enable interrupt

	/* Save context to stack */
	SAVE_CONTEXT

	ldr	x4, =ctxtsk			// X4 = &ctxtsk
	ldr	x5, =TASKMODE
	mov	x0, #0
	ldr	x8, [x4]			// X8 = ctxtsk
	mov	x9, sp				// Copy sp to x9
	str	x9, [x8, #TCB_tskctxb + CTXB_ssp] // save ssp to TCB
	str	x0, [x4]			// ctxtsk = NULL
	str	w0, [x5]			// taskmode = 0

l_dispatch0:
	/* Align stack to 16 bytes */
	mov	x9, sp
	bic	x9, x9, #0xf
	mov	sp, x9

	/* interrupt-enabled state */
	ldr	x5, =schedtsk			// X5 = &schedtsk
	ldr	x6, =lowpow_discnt		// X6 = &lowpow_discnt

l_dispatch1:
	msr	daifset, #0x3			// disable interrupt

	ldr	x8, [x5]			// X8 = schedtsk
	cbnz	x8, l_dispatch2			// is there schedtsk?

	/* enter low-power mode since there is no task to execute */
	ldr	w0, [x6]			// is low_pow disabled?
	cbnz	w0, 1f
	bl	low_pow				// call low_pow()
1:
	msr	daifclr, #0xf			// enable interrupt
	b	l_dispatch1

l_dispatch2:
	/* dispatch to schedtsk */
	/* interrupt-disabled state */
	str	x8, [x4]			// ctxtsk = schedtsk
	ldr	x9, [x8, #TCB_tskctxb + CTXB_ssp] // restore ssp from TCB
	mov	sp, x9

#if USE_MMU
	/* switch task spaces */
	ldr	x0, [x8, #TCB_tskctxb + CTXB_uatb]
	ldr	w1, [x8, #TCB_tskctxb + CTXB_lsid]
	mrs	x2, ttbr1_el1
	cbz	x0, 1f
	and	x2, x2, #0xfff
	orr	x2, x2, x0
1:
	msr	ttbr0_el1, x2
	msr	contextidr_el1, x1
	isb
#endif

	ldr	x1, =dispatch_disabled
	mov	w2, #0
	str	w2, [x1]			// enable dispatch

	/* Restore context and return */
	RESTORE_CONTEXT
	eret

/* ------------------------------------------------------------------------ */
/*
 * _tk_ret_int - Return from interrupt
 */
	.text
	.balign	4
	.globl	_tk_ret_int
	.type	_tk_ret_int, %function
_tk_ret_int:
	msr	daifset, #0x3			// disable interrupt

	ldr	x0, =TASKINDP			// is there additional multi-interrupt?
	ldr	w0, [x0]
	cbnz	w0, l_nodispatch

	ldr	x0, =dispatch_disabled		// is dispatch disabled?
	ldr	w0, [x0]
	cbnz	w0, l_nodispatch

	ldr	x0, =ctxtsk			// is dispatch necessary?
	ldr	x1, =schedtsk
	ldr	x0, [x0]
	ldr	x1, [x1]
	cmp	x0, x1
	b.ne	dispatch_entry			// goto dispatch processing

l_nodispatch:
	RESTORE_CONTEXT
	eret

/* ------------------------------------------------------------------------ */
/*
 * High-level language support routine for interrupt handler
 */
	.text
	.balign	4
	.globl	inthdr_startup
	.type	inthdr_startup, %function
inthdr_startup:
	/* Save context */
	SAVE_CONTEXT

	ldr	x4, =TASKINDP			// enter task-independent portion
	ldr	w5, [x4]
	add	w0, w5, #1
	str	w0, [x4]

	/* x0 = dintno, x1 = sp */
	mov	x1, sp
	bl	hll_inthdr_call			// call handler

	ldr	x4, =TASKINDP
	ldr	w5, [x4]
	sub	w5, w5, #1
	str	w5, [x4]			// leave task-independent portion

	b	_tk_ret_int

/* ------------------------------------------------------------------------ */
/*
 * High-level language support routine for timer handler
 */
	.text
	.balign	4
	.globl	timer_handler_startup
	.type	timer_handler_startup, %function
timer_handler_startup:
	/* Save caller-saved registers */
	stp	x29, x30, [sp, #-16]!
	stp	x0,  x1,  [sp, #-16]!
	stp	x2,  x3,  [sp, #-16]!

	ldr	x4, =TASKINDP			// enter task-independent portion
	ldr	w5, [x4]
	add	w0, w5, #1
	str	w0, [x4]

	bl	timer_handler			// call timer_handler()

	ldr	x4, =TASKINDP
	str	w5, [x4]			// leave task-independent portion

	ldp	x2,  x3,  [sp], #16
	ldp	x0,  x1,  [sp], #16
	ldp	x29, x30, [sp], #16

	b	_tk_ret_int

/* ------------------------------------------------------------------------ */
/*
 * System call entry
 */
	.text
	.balign	4
	.globl	call_entry
	.type	call_entry, %function
call_entry:
	/* Save context */
	SAVE_CONTEXT

	/* x8 contains the system call number */
	mov	x0, x8			// fncd
	mov	x1, sp			// parameter pointer

	bl	svc_handler		// call system call handler

	/* Restore and return */
	RESTORE_CONTEXT
	eret

/* ------------------------------------------------------------------------ */
/*
 * Unsupported system call
 */
	.text
	.balign	4
	.globl	no_support
	.type	no_support, %function
no_support:
	mov	x0, #-10		// E_RSFN
	ret

/* ------------------------------------------------------------------------ */
/*
 * Default exception handlers (C functions to be implemented)
 */
	.weak	sync_exception_handler
	.weak	irq_exception_handler
	.weak	fiq_exception_handler
	.weak	serror_exception_handler
	.weak	hll_inthdr_call
	.weak	svc_handler
	.weak	low_pow

sync_exception_handler:
irq_exception_handler:
fiq_exception_handler:
serror_exception_handler:
hll_inthdr_call:
svc_handler:
low_pow:
	ret

/* ------------------------------------------------------------------------ */
/*
 * Data Section - External references
 */
	.data
	.balign	8

	.globl	dispatch_disabled
dispatch_disabled:
	.word	0

	.globl	TASKINDP
TASKINDP:
	.word	0

	.globl	TASKMODE
TASKMODE:
	.word	0

/* External symbols (defined elsewhere) */
	.extern	ctxtsk
	.extern	schedtsk
	.extern	lowpow_discnt
	.extern	timer_handler
