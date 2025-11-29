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

/* Task entry point function type */
typedef void (*TASK_FP)(INT, void*);

#define E_OK			0

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
	TASK_FP	task;		/* Task entry point */
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

/*
 * MMU and Page Table Definitions
 */

/* Page table entry attributes */
#define PTE_VALID		(1ULL << 0)	/* Valid entry */
#define PTE_TABLE		(1ULL << 1)	/* Table descriptor (for L0-L2) */
#define PTE_BLOCK		(0ULL << 1)	/* Block descriptor (for L1-L2) */
#define PTE_PAGE		(1ULL << 1)	/* Page descriptor (for L3) */

/* Block/Page attributes */
#define PTE_ATTR_IDX(x)		((UW64)(x) << 2)	/* Memory attribute index */
#define PTE_NS			(1ULL << 5)		/* Non-secure */
#define PTE_AP_RW_EL1		(0ULL << 6)		/* Read/write, EL1 only */
#define PTE_AP_RW_ALL		(1ULL << 6)		/* Read/write, all ELs */
#define PTE_AP_RO_EL1		(2ULL << 6)		/* Read-only, EL1 only */
#define PTE_AP_RO_ALL		(3ULL << 6)		/* Read-only, all ELs */
#define PTE_SH_NONE		(0ULL << 8)		/* Non-shareable */
#define PTE_SH_OUTER		(2ULL << 8)		/* Outer shareable */
#define PTE_SH_INNER		(3ULL << 8)		/* Inner shareable */
#define PTE_AF			(1ULL << 10)		/* Access flag */
#define PTE_NG			(1ULL << 11)		/* Not global */
#define PTE_PXN			(1ULL << 53)		/* Privileged execute never */
#define PTE_UXN			(1ULL << 54)		/* Unprivileged execute never */

/* Memory attribute indices (for MAIR_EL1) */
#define MAIR_IDX_DEVICE_nGnRnE	0	/* Device memory, non-gathering, non-reordering, no early write ack */
#define MAIR_IDX_NORMAL_NC	1	/* Normal memory, non-cacheable */
#define MAIR_IDX_NORMAL		2	/* Normal memory, write-back cacheable */

/* MAIR_EL1 attribute values */
#define MAIR_DEVICE_nGnRnE	0x00	/* Device-nGnRnE */
#define MAIR_NORMAL_NC		0x44	/* Normal, non-cacheable */
#define MAIR_NORMAL		0xFF	/* Normal, write-back read/write-allocate */

/* TCR_EL1 bits */
#define TCR_T0SZ(x)		((UW64)(x) << 0)	/* Size offset for TTBR0_EL1 */
#define TCR_IRGN0_WBWA		(1ULL << 8)		/* Inner write-back read-allocate write-allocate */
#define TCR_ORGN0_WBWA		(1ULL << 10)		/* Outer write-back read-allocate write-allocate */
#define TCR_SH0_INNER		(3ULL << 12)		/* Inner shareable */
#define TCR_TG0_4KB		(0ULL << 14)		/* 4KB granule for TTBR0_EL1 */
#define TCR_T1SZ(x)		((UW64)(x) << 16)	/* Size offset for TTBR1_EL1 */
#define TCR_A1			(1ULL << 22)		/* ASID from TTBR1_EL1 */
#define TCR_IRGN1_WBWA		(1ULL << 24)		/* Inner write-back read-allocate write-allocate */
#define TCR_ORGN1_WBWA		(1ULL << 26)		/* Outer write-back read-allocate write-allocate */
#define TCR_SH1_INNER		(3ULL << 28)		/* Inner shareable */
#define TCR_TG1_4KB		(2ULL << 30)		/* 4KB granule for TTBR1_EL1 */
#define TCR_IPS_1TB		(2ULL << 32)		/* Intermediate physical address size: 40 bits, 1TB */

/* SCTLR_EL1 bits */
#define SCTLR_M			(1ULL << 0)	/* MMU enable */
#define SCTLR_C			(1ULL << 2)	/* Data cache enable */
#define SCTLR_I			(1ULL << 12)	/* Instruction cache enable */

/* Page table sizes and counts */
#define PAGE_SIZE		4096
#define PAGE_SHIFT		12
#define L0_ENTRIES		512
#define L1_ENTRIES		512
#define L2_ENTRIES		512

/* Page table entry type */
typedef UW64 pte_t;

/* Page tables (must be 4KB aligned) */
static pte_t page_tables_l0[L0_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static pte_t page_tables_l1[L1_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static pte_t page_tables_l2_ram[L2_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static pte_t page_tables_l2_dev[L2_ENTRIES] __attribute__((aligned(PAGE_SIZE)));

/* Forward declarations */
static void uart_puts(const char *s);
static void uart_puthex(UW val);

/* External dispatcher function (from cpu_support.S) */
extern void dispatch_to_schedtsk(void);

/*
 * Task management functions
 */

/* Initialize task context */
static void setup_task_context(TCB *tcb, TASK_FP task, void *stack, INT stksz)
{
	UW64 *sp;
	UW64 stack_top;

	/* Calculate top of stack (grows downward) */
	stack_top = (UW64)stack + stksz;

	/* Align to 16 bytes */
	stack_top &= ~0xFULL;

	tcb->isstack = (void*)stack_top;

	/* Set up stack to match RESTORE_CONTEXT expectations */
	/* RESTORE_CONTEXT expects (from low to high addresses): */
	/* [SP+0]: SPSR, padding */
	/* [SP+16]: X30 (LR), ELR_EL1 (PC) */
	/* [SP+32..254]: X28-X29, X26-X27, ..., X2-X3, X0-X1 */

	sp = (UW64*)stack_top;

	/* Reserve space for stack frame (280 bytes = 35 * 8) */
	sp -= 35;

	/* Initialize all to zero */
	for (int i = 0; i < 35; i++) {
		sp[i] = 0;
	}

	/* Set up SPSR (offset 0) */
	/* SPSR: EL1h mode (0x5), IRQ/FIQ enabled (I=0, F=0) */
	sp[0] = 0x00000005;		/* SPSR: EL1h with interrupts enabled */
	sp[1] = 0;			/* padding */

	/* Set up PC (ELR_EL1) at offset 16 */
	sp[2] = 0;			/* X30 (LR) = 0 */
	sp[3] = (UW64)task;		/* ELR_EL1 = task entry point */

	/* X0-X30 are already zero from initialization */
	/* SP field (offset 31*8 = 248) will be ignored for EL1h tasks */

	/* Save context pointer (points to SPSR) */
	tcb->tskctxb.ssp = (void*)sp;
}

/* Create a task */
static ER create_task(INT idx, ID tskid, TASK_FP task, void *stack, INT stksz)
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
 * MMU Setup Functions
 */

/*
 * Setup page tables for identity mapping using 1GB blocks at Level 1
 * Much simpler than using Level 2 tables
 * Maps:
 *   - 0x00000000-0x3FFFFFFF (1GB): Device memory (includes UART, GIC, etc.)
 *   - 0x40000000-0x7FFFFFFF (1GB): Normal cacheable memory (RAM)
 */
static void setup_page_tables(void)
{
	INT i;

	/* Clear all page tables */
	for (i = 0; i < L0_ENTRIES; i++) {
		page_tables_l0[i] = 0;
	}
	for (i = 0; i < L1_ENTRIES; i++) {
		page_tables_l1[i] = 0;
	}

	/*
	 * Level 0: Each entry covers 512GB
	 * We only need entry 0 (0x0_00000000 - 0x7F_FFFFFFFF)
	 */
	page_tables_l0[0] = (UW64)page_tables_l1 | PTE_VALID | PTE_TABLE;

	/*
	 * Level 1: Use 1GB block descriptors (simpler, no Level 2 needed)
	 * Entry 0: 0x00000000-0x3FFFFFFF (1GB) - Device memory
	 * Entry 1: 0x40000000-0x7FFFFFFF (1GB) - Normal memory (RAM)
	 */

	/* Entry 0: 1GB block for device memory (0x00000000-0x3FFFFFFF) */
	page_tables_l1[0] = 0x00000000ULL
		| PTE_VALID
		| PTE_BLOCK
		| PTE_ATTR_IDX(MAIR_IDX_DEVICE_nGnRnE)
		| PTE_AP_RW_EL1
		| PTE_SH_OUTER
		| PTE_AF
		| PTE_PXN
		| PTE_UXN;

	/* Entry 1: 1GB block for RAM (0x40000000-0x7FFFFFFF) */
	page_tables_l1[1] = 0x40000000ULL
		| PTE_VALID
		| PTE_BLOCK
		| PTE_ATTR_IDX(MAIR_IDX_NORMAL)
		| PTE_AP_RW_EL1
		| PTE_SH_INNER
		| PTE_AF;
}

/*
 * Initialize and enable MMU
 */
static void init_mmu(void)
{
	UW64 mair, tcr, sctlr;

	uart_puts("  [1] Disabling MMU...\n");
	/* Ensure MMU is disabled before configuring */
	__asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
	sctlr &= ~(SCTLR_M | SCTLR_C);  /* Disable MMU and D-cache */
	__asm__ volatile("msr sctlr_el1, %0" :: "r"(sctlr));
	__asm__ volatile("isb");

	uart_puts("  [2] Invalidating TLB...\n");
	/* Invalidate TLB */
	__asm__ volatile("tlbi vmalle1");
	__asm__ volatile("dsb sy");
	__asm__ volatile("isb");

	uart_puts("  [3] Setting up page tables...\n");
	/* Setup page tables */
	setup_page_tables();

	/* Data Synchronization Barrier - ensure page tables are written */
	__asm__ volatile("dsb sy");

	uart_puts("  [4] Configuring MAIR_EL1...\n");
	/*
	 * Configure MAIR_EL1 (Memory Attribute Indirection Register)
	 * Define memory types for indices 0, 1, 2
	 */
	mair = ((UW64)MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8))
	     | ((UW64)MAIR_NORMAL_NC << (MAIR_IDX_NORMAL_NC * 8))
	     | ((UW64)MAIR_NORMAL << (MAIR_IDX_NORMAL * 8));

	__asm__ volatile("msr mair_el1, %0" :: "r"(mair));

	uart_puts("  [5] Configuring TCR_EL1...\n");
	/*
	 * Configure TCR_EL1 (Translation Control Register)
	 * - T0SZ=25: 39-bit VA for TTBR0_EL1 (512GB)
	 * - T1SZ=25: 39-bit VA for TTBR1_EL1 (512GB)
	 * - TG0=4KB, TG1=4KB: 4KB granule
	 * - IPS=40 bits (1TB physical address space)
	 * - Shareable, cacheable attributes
	 */
	tcr = TCR_T0SZ(25)
	    | TCR_IRGN0_WBWA
	    | TCR_ORGN0_WBWA
	    | TCR_SH0_INNER
	    | TCR_TG0_4KB
	    | TCR_T1SZ(25)
	    | TCR_IRGN1_WBWA
	    | TCR_ORGN1_WBWA
	    | TCR_SH1_INNER
	    | TCR_TG1_4KB
	    | TCR_IPS_1TB;

	__asm__ volatile("msr tcr_el1, %0" :: "r"(tcr));

	uart_puts("  [6] Setting TTBR0/TTBR1...\n");
	/*
	 * Set TTBR0_EL1 and TTBR1_EL1 to point to L0 page table
	 * For simplicity, use same page table for both
	 */
	__asm__ volatile("msr ttbr0_el1, %0" :: "r"((UW64)page_tables_l0));
	__asm__ volatile("msr ttbr1_el1, %0" :: "r"((UW64)page_tables_l0));

	uart_puts("  [7] Synchronization barriers...\n");
	/* Synchronization barriers before enabling MMU */
	__asm__ volatile("dsb sy");
	__asm__ volatile("isb");

	uart_puts("  [8] Enabling MMU...\n");
	/*
	 * Enable MMU only (no caches for now to isolate the issue)
	 */
	__asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
	sctlr |= SCTLR_M;  /* Enable MMU only, no caches */
	__asm__ volatile("msr sctlr_el1, %0" :: "r"(sctlr));

	uart_puts("  [9] ISB after MMU enable...\n");
	/* Instruction Synchronization Barrier */
	__asm__ volatile("isb");

	uart_puts("  [10] MMU enabled successfully!\n");
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

	/* Initialize MMU */
	uart_puts("Initializing MMU...\n");
	init_mmu();
	uart_puts("MMU enabled (identity mapping with caches).\n");
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
