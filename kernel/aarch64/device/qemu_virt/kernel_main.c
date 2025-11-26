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
#include "tkdev_timer.h"

/* Simple type definitions for standalone build */
typedef unsigned int		UW;
typedef unsigned long		UW64;
typedef int			INT;
typedef int			ER;
typedef int			ID;
typedef unsigned long long	UD;
typedef void			(*FP)(INT, void*);

#define E_OK			0
#define NULL			((void*)0)

/* Timer counter */
extern UW timer_freq;		/* Timer frequency (defined in icrt0.S) */
static volatile UD timer_tick_count = 0;	/* Timer tick counter */

/*
 * Task state
 */
typedef enum {
	TS_NONEXIST	= 0,	/* Non-existent */
	TS_DORMANT	= 1,	/* Dormant */
	TS_READY	= 2,	/* Ready */
	TS_RUN		= 3	/* Running */
} TSTAT;

/*
 * Task context (minimal version)
 */
typedef struct {
	void	*ssp;		/* System stack pointer */
} CTXB;

/*
 * Stack frame for task (from cpu_task.h)
 */
typedef struct {
	UW64	x[31];		/* X0-X30 */
	UW64	sp;
	UW64	pc;
	UW64	spsr;
	UW64	taskmode;
} SStackFrame;

/*
 * Task Control Block (simplified)
 */
typedef struct {
	ID	tskid;		/* Task ID */
	TSTAT	state;		/* Task state */
	FP	task;		/* Task entry point */
	void	*exinf;		/* Extended information */
	INT	priority;	/* Priority (not used yet) */
	void	*stack;		/* Stack area */
	INT	stksz;		/* Stack size */
	CTXB	tskctxb;	/* Task context */
	void	*isstack;	/* Initial system stack pointer */
} TCB;

/*
 * Task management
 */
#define MAX_TASKS	3
static TCB task_table[MAX_TASKS];
extern void *ctxtsk;			/* Current task (defined in kernel_stubs.c) */
extern void *schedtsk;			/* Task to be scheduled (defined in kernel_stubs.c) */
static INT current_task_idx = -1;	/* Current task index */
static INT task_switch_interval = 100;	/* Switch tasks every 100ms (100 ticks) */

/* Task stacks (1KB each) */
#define TASK_STACK_SIZE	1024
static UW64 task1_stack[TASK_STACK_SIZE / sizeof(UW64)] __attribute__((aligned(16)));
static UW64 task2_stack[TASK_STACK_SIZE / sizeof(UW64)] __attribute__((aligned(16)));
static UW64 idle_stack[TASK_STACK_SIZE / sizeof(UW64)] __attribute__((aligned(16)));

/* Forward declarations */
static void uart_puts(const char *s);
static void uart_puthex(UW val);

/* External dispatcher function (from cpu_support.S) */
extern void dispatch_to_schedtsk(void);

/*
 * Task management functions
 */

/* Initialize task context */
static void setup_task_context(TCB *tcb, FP task, void *stack, INT stksz)
{
	SStackFrame *ssp;

	/* Set up initial stack pointer (top of stack) */
	tcb->isstack = (void*)((UW64)stack + stksz);

	/* Reserve space for SStackFrame at top of stack */
	ssp = (SStackFrame*)tcb->isstack;
	ssp--;

	/* Initialize stack frame */
	for (int i = 0; i < 31; i++) {
		ssp->x[i] = 0;
	}
	ssp->pc = (UW64)task;		/* Task entry point */
	ssp->sp = 0;			/* Not used for kernel tasks */
	ssp->spsr = 0x00000004;		/* EL1h, interrupts enabled */
	ssp->taskmode = 0;

	/* Save context */
	tcb->tskctxb.ssp = ssp;
}

/* Create a task */
static ER create_task(INT idx, ID tskid, FP task, void *stack, INT stksz)
{
	TCB *tcb;

	if (idx >= MAX_TASKS) {
		return -1;
	}

	tcb = &task_table[idx];
	tcb->tskid = tskid;
	tcb->state = TS_DORMANT;
	tcb->task = task;
	tcb->exinf = NULL;
	tcb->priority = 1;
	tcb->stack = stack;
	tcb->stksz = stksz;

	setup_task_context(tcb, task, stack, stksz);

	return E_OK;
}

/* Start a task */
static ER start_task(INT idx)
{
	TCB *tcb;

	if (idx >= MAX_TASKS) {
		return -1;
	}

	tcb = &task_table[idx];
	if (tcb->state != TS_DORMANT) {
		return -1;
	}

	tcb->state = TS_READY;

	/* If this is the first task, make it the scheduled task */
	if (schedtsk == NULL) {
		schedtsk = (void*)tcb;
	}

	return E_OK;
}

/* Simple round-robin scheduler */
static void schedule(void)
{
	INT i, next_idx;
	TCB *next_task = NULL;

	/* Find next READY task */
	next_idx = current_task_idx + 1;
	for (i = 0; i < MAX_TASKS; i++) {
		if (next_idx >= MAX_TASKS) {
			next_idx = 0;
		}

		if (task_table[next_idx].state == TS_READY) {
			next_task = &task_table[next_idx];
			current_task_idx = next_idx;
			break;
		}
		next_idx++;
	}

	/* Set scheduled task */
	if (next_task != NULL) {
		schedtsk = (void*)next_task;
	}
}

/*
 * Demo task functions
 */
static void task1_main(INT stacd, void *exinf)
{
	(void)stacd;
	(void)exinf;

	while (1) {
		uart_puts("[Task1] Running...\n");

		/* Busy wait */
		for (volatile int i = 0; i < 1000000; i++);
	}
}

static void task2_main(INT stacd, void *exinf)
{
	(void)stacd;
	(void)exinf;

	while (1) {
		uart_puts("[Task2] Running...\n");

		/* Busy wait */
		for (volatile int i = 0; i < 1000000; i++);
	}
}

static void idle_task_main(INT stacd, void *exinf)
{
	(void)stacd;
	(void)exinf;

	while (1) {
		uart_puts("[Idle] Running...\n");

		/* Wait for interrupt */
		__asm__ volatile("wfi");
	}
}

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

	/* Start timer interrupts */
	uart_puts("Starting timer interrupts (1ms period)...\n");
	start_hw_timer();
	uart_puts("Timer started.\n");
	uart_puts("\n");

	/* Enable IRQ interrupts */
	uart_puts("Enabling IRQ interrupts...\n");
	__asm__ volatile("msr daifclr, #2");  // Clear IRQ mask (bit 1)
	uart_puts("IRQ enabled.\n");
	uart_puts("\n");

	/* Initialize tasks */
	uart_puts("Creating tasks...\n");
	create_task(0, 1, task1_main, task1_stack, TASK_STACK_SIZE);
	create_task(1, 2, task2_main, task2_stack, TASK_STACK_SIZE);
	create_task(2, 3, idle_task_main, idle_stack, TASK_STACK_SIZE);

	uart_puts("Starting tasks...\n");
	start_task(0);
	start_task(1);
	start_task(2);

	uart_puts("Tasks created and started.\n");
	uart_puts("Task switching interval: ");
	uart_puthex(task_switch_interval);
	uart_puts(" ms\n");
	uart_puts("\n");

	/* Start multitasking by dispatching to first task */
	uart_puts("Starting multitasking...\n");
	uart_puts("\n");

	/* This will switch to the first ready task and never return */
	dispatch_to_schedtsk();

	/* Should never reach here */
	uart_puts("ERROR: Returned from dispatcher!\n");
	for (;;) {
		__asm__ volatile("wfi");
	}
}

/*
 * Timer interrupt handler
 */
void timer_handler(void)
{
	/* Increment tick counter */
	timer_tick_count++;

	/* Print message every 1000 ticks (1 second at 1ms per tick) */
	if ((timer_tick_count % 1000) == 0) {
		uart_puts("Timer tick: ");
		uart_puthex((UW)(timer_tick_count / 1000));
		uart_puts(" seconds\n");
	}

	/* Task switching: schedule next task every task_switch_interval ticks */
	if ((timer_tick_count % task_switch_interval) == 0) {
		schedule();
	}

	/* Clear timer interrupt and rearm for next period */
	clear_hw_timer_interrupt();
}

/*
 * Exception handlers
 */
void sync_exception_handler(void *sp)
{
	(void)sp;
	uart_puts("SYNC EXCEPTION!\n");
	for (;;) __asm__ volatile("wfi");
}

void irq_exception_handler(void *sp)
{
	volatile UW *gicc_iar = (volatile UW *)GICC_IAR;
	volatile UW *gicc_eoir = (volatile UW *)GICC_EOIR;
	UW intno;

	(void)sp;

	/* Read interrupt number from GIC */
	intno = *gicc_iar & 0x3FF;

	/* Handle timer interrupt */
	if (intno == TIMER_IRQ) {
		timer_handler();
	} else {
		/* Unknown interrupt */
		uart_puts("IRQ: ");
		uart_puthex(intno);
		uart_puts("\n");
	}

	/* Acknowledge interrupt */
	*gicc_eoir = intno;
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
