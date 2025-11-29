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
	TS_RUN		= 3,	/* Running */
	TS_WAIT		= 4	/* Waiting */
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
typedef struct tcb {
	ID		tskid;		/* Task ID */
	TSTAT		state;		/* Task state */
	TASK_FP		task;		/* Task entry point */
	void		*exinf;		/* Extended information */
	INT		priority;	/* Priority (not used yet) */
	void		*stack;		/* Stack area */
	INT		stksz;		/* Stack size */
	CTXB		tskctxb;	/* Task context */
	void		*isstack;	/* Initial system stack pointer */

	/* Waiting information */
	struct tcb	*wait_next;	/* Next task in wait queue */
	void		*wait_obj;	/* Object being waited on (semaphore, etc.) */
	UW		wait_flgptn;	/* Flag pattern for event flag wait */
	UW		wait_mode;	/* Wait mode for event flag wait */
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

/*
 * Semaphore Control Block
 */
typedef struct {
	ID	semid;		/* Semaphore ID */
	INT	semcnt;		/* Semaphore counter */
	INT	maxsem;		/* Maximum semaphore count */
	TCB	*wait_queue;	/* Queue of waiting tasks */
} SEMCB;

/*
 * Semaphore management
 */
#define MAX_SEMAPHORES	4
static SEMCB semaphore_table[MAX_SEMAPHORES];
static INT next_semid = 1;	/* Next semaphore ID to allocate */

/* Shared semaphore for demo */
static ID demo_sem = 0;	/* Will be initialized in kernel_main */

/*
 * Mutex Control Block
 */
typedef struct {
	ID	mtxid;		/* Mutex ID */
	INT	locked;		/* Lock state: 0=unlocked, 1=locked */
	TCB	*owner;		/* Task that owns the mutex */
	TCB	*wait_queue;	/* Queue of waiting tasks */
} MTXCB;

/*
 * Mutex management
 */
#define MAX_MUTEXES	4
static MTXCB mutex_table[MAX_MUTEXES];
static INT next_mtxid = 1;	/* Next mutex ID to allocate */

/* Shared mutex for demo */
static ID demo_mtx = 0;		/* Will be initialized in kernel_main */

/*
 * Event Flag Control Block
 */
typedef struct {
	ID	flgid;		/* Event flag ID */
	UW	flgptn;		/* Current flag pattern (bit pattern) */
	TCB	*wait_queue;	/* Queue of waiting tasks */
} FLGCB;

/*
 * Event flag management
 */
#define MAX_EVENTFLAGS	4
static FLGCB eventflag_table[MAX_EVENTFLAGS];
static INT next_flgid = 1;	/* Next event flag ID to allocate */

/* Shared event flag for demo */
static ID demo_flg = 0;		/* Will be initialized in kernel_main */

/*
 * Event flag wait modes
 */
#define TWF_ANDW	0x00	/* AND wait (wait for all bits) */
#define TWF_ORW		0x01	/* OR wait (wait for any bit) */
#define TWF_CLR		0x10	/* Clear matched bits after wakeup */

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
	tcb->wait_next = NULL;
	tcb->wait_obj = NULL;
	tcb->wait_flgptn = 0;
	tcb->wait_mode = 0;

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
 * System Call Implementation
 */

/* System call numbers */
#define SVC_GET_TID		1	/* Get current task ID */
#define SVC_DLY_TSK		2	/* Task delay */
#define SVC_GET_TIM		3	/* Get system time */
#define SVC_EXT_TSK		4	/* Exit task */
#define SVC_CRE_SEM		5	/* Create semaphore */
#define SVC_SIG_SEM		6	/* Signal semaphore */
#define SVC_WAI_SEM		7	/* Wait semaphore */
#define SVC_CRE_MTX		8	/* Create mutex */
#define SVC_LOC_MTX		9	/* Lock mutex */
#define SVC_UNL_MTX		10	/* Unlock mutex */
#define SVC_CRE_FLG		11	/* Create event flag */
#define SVC_SET_FLG		12	/* Set event flag */
#define SVC_CLR_FLG		13	/* Clear event flag */
#define SVC_WAI_FLG		14	/* Wait event flag */

/* Stack frame structure (must match cpu_support.S SAVE_CONTEXT layout) */
typedef struct {
	UW64	spsr;		/* Offset 0 */
	UW64	padding;	/* Offset 8 */
	UW64	x30;		/* Offset 16 */
	UW64	elr;		/* Offset 24 */
	UW64	x28, x29;	/* Offset 32 */
	UW64	x26, x27;
	UW64	x24, x25;
	UW64	x22, x23;
	UW64	x20, x21;
	UW64	x18, x19;
	UW64	x16, x17;
	UW64	x14, x15;
	UW64	x12, x13;
	UW64	x10, x11;
	UW64	x8, x9;
	UW64	x6, x7;
	UW64	x4, x5;
	UW64	x2, x3;
	UW64	x0, x1;		/* Offset 272 */
} SVC_REGS;

/*
 * System call: Get current task ID
 * Returns: task ID in x0
 */
static ER svc_get_tid(SVC_REGS *regs)
{
	TCB *tcb = (TCB *)ctxtsk;
	if (tcb == NULL) {
		regs->x0 = 0;  /* No current task */
		return E_OK;
	}
	regs->x0 = (UW64)tcb->tskid;
	return E_OK;
}

/*
 * System call: Task delay
 * Input: x0 = delay time in milliseconds
 */
static ER svc_dly_tsk(SVC_REGS *regs)
{
	UW64 dlytim = regs->x0;
	UW64 start_tick = timer_tick_count;

	/* Simple busy wait (don't use WFI in SVC handler - interrupts may not work correctly) */
	while ((timer_tick_count - start_tick) < dlytim) {
		/* Just spin - timer interrupts will update timer_tick_count */
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Get system time
 * Returns: current timer tick count in x0
 */
static ER svc_get_tim(SVC_REGS *regs)
{
	regs->x0 = timer_tick_count;
	return E_OK;
}

/*
 * System call: Exit task (placeholder)
 */
static ER svc_ext_tsk(SVC_REGS *regs)
{
	uart_puts("[SVC] Task exit requested\n");
	/* For now, just put task in infinite loop */
	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Create semaphore
 * Input: x0 = initial count, x1 = max count
 * Output: x0 = semaphore ID (or negative error)
 */
static ER svc_cre_sem(SVC_REGS *regs)
{
	INT isemcnt = (INT)regs->x0;
	INT maxsem = (INT)regs->x1;
	INT i;

	/* Find free semaphore slot */
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphore_table[i].semid == 0) {
			/* Initialize semaphore */
			semaphore_table[i].semid = next_semid++;
			semaphore_table[i].semcnt = isemcnt;
			semaphore_table[i].maxsem = maxsem;
			semaphore_table[i].wait_queue = NULL;

			regs->x0 = semaphore_table[i].semid;
			return E_OK;
		}
	}

	/* No free slots */
	regs->x0 = -1;
	return -1;
}

/*
 * System call: Signal semaphore (release resource)
 * Input: x0 = semaphore ID
 * Output: x0 = E_OK or error
 */
static ER svc_sig_sem(SVC_REGS *regs)
{
	ID semid = (ID)regs->x0;
	SEMCB *sem = NULL;
	TCB *waiting_task;
	INT i;

	/* Find semaphore */
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphore_table[i].semid == semid) {
			sem = &semaphore_table[i];
			break;
		}
	}

	if (sem == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Check if any task is waiting */
	if (sem->wait_queue != NULL) {
		/* Wake up first waiting task */
		waiting_task = sem->wait_queue;
		sem->wait_queue = waiting_task->wait_next;

		/* Change task state to READY */
		waiting_task->state = TS_READY;
		waiting_task->wait_next = NULL;
		waiting_task->wait_obj = NULL;
	} else {
		/* No waiting tasks, increment counter */
		if (sem->semcnt < sem->maxsem) {
			sem->semcnt++;
		}
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Wait semaphore (acquire resource)
 * Input: x0 = semaphore ID
 * Output: x0 = E_OK or error
 */
static ER svc_wai_sem(SVC_REGS *regs)
{
	ID semid = (ID)regs->x0;
	SEMCB *sem = NULL;
	TCB *current;
	TCB **queue_ptr;
	INT i;

	/* Find semaphore */
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphore_table[i].semid == semid) {
			sem = &semaphore_table[i];
			break;
		}
	}

	if (sem == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Check if resource is available */
	if (sem->semcnt > 0) {
		/* Resource available, decrement counter */
		sem->semcnt--;
		regs->x0 = E_OK;
		return E_OK;
	}

	/* No resource available, put task in wait queue */
	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Add to end of wait queue */
	current->state = TS_WAIT;
	current->wait_next = NULL;
	current->wait_obj = sem;

	if (sem->wait_queue == NULL) {
		sem->wait_queue = current;
	} else {
		/* Find end of queue */
		queue_ptr = &sem->wait_queue;
		while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
			queue_ptr = &((*queue_ptr)->wait_next);
		}
		(*queue_ptr)->wait_next = current;
	}

	/* Force task switch - don't return E_OK yet */
	/* Task will be woken up by sig_sem and will return E_OK then */
	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Create mutex
 * Input: none
 * Output: x0 = mutex ID or error (-1)
 */
static ER svc_cre_mtx(SVC_REGS *regs)
{
	INT i;

	/* Find free mutex slot */
	for (i = 0; i < MAX_MUTEXES; i++) {
		if (mutex_table[i].mtxid == 0) {
			/* Initialize mutex */
			mutex_table[i].mtxid = next_mtxid++;
			mutex_table[i].locked = 0;
			mutex_table[i].owner = NULL;
			mutex_table[i].wait_queue = NULL;

			regs->x0 = mutex_table[i].mtxid;
			return E_OK;
		}
	}

	/* No free slots */
	regs->x0 = -1;
	return -1;
}

/*
 * System call: Lock mutex (acquire)
 * Input: x0 = mutex ID
 * Output: x0 = E_OK or error
 */
static ER svc_loc_mtx(SVC_REGS *regs)
{
	ID mtxid = (ID)regs->x0;
	MTXCB *mtx = NULL;
	TCB *current;
	TCB **queue_ptr;
	INT i;

	/* Find mutex */
	for (i = 0; i < MAX_MUTEXES; i++) {
		if (mutex_table[i].mtxid == mtxid) {
			mtx = &mutex_table[i];
			break;
		}
	}

	if (mtx == NULL) {
		regs->x0 = -1;
		return -1;
	}

	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Check if mutex is available */
	if (mtx->locked == 0) {
		/* Mutex available, lock it */
		mtx->locked = 1;
		mtx->owner = current;
		regs->x0 = E_OK;
		return E_OK;
	}

	/* Mutex is locked */
	/* Check if current task already owns it (recursive lock - not allowed) */
	if (mtx->owner == current) {
		regs->x0 = -1;  /* Error: recursive lock not supported */
		return -1;
	}

	/* Mutex is locked by another task, add to wait queue */
	current->state = TS_WAIT;
	current->wait_next = NULL;
	current->wait_obj = mtx;

	if (mtx->wait_queue == NULL) {
		mtx->wait_queue = current;
	} else {
		/* Find end of queue */
		queue_ptr = &mtx->wait_queue;
		while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
			queue_ptr = &((*queue_ptr)->wait_next);
		}
		(*queue_ptr)->wait_next = current;
	}

	/* TODO: Priority inheritance - boost owner's priority if needed */

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Unlock mutex (release)
 * Input: x0 = mutex ID
 * Output: x0 = E_OK or error
 */
static ER svc_unl_mtx(SVC_REGS *regs)
{
	ID mtxid = (ID)regs->x0;
	MTXCB *mtx = NULL;
	TCB *current;
	TCB *waiting_task;
	INT i;

	/* Find mutex */
	for (i = 0; i < MAX_MUTEXES; i++) {
		if (mutex_table[i].mtxid == mtxid) {
			mtx = &mutex_table[i];
			break;
		}
	}

	if (mtx == NULL) {
		regs->x0 = -1;
		return -1;
	}

	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Check if current task owns the mutex */
	if (mtx->owner != current) {
		regs->x0 = -1;  /* Error: task doesn't own this mutex */
		return -1;
	}

	/* Check if any task is waiting */
	if (mtx->wait_queue != NULL) {
		/* Wake up first waiting task and transfer ownership */
		waiting_task = mtx->wait_queue;
		mtx->wait_queue = waiting_task->wait_next;

		/* Transfer ownership to waiting task */
		mtx->owner = waiting_task;
		mtx->locked = 1;  /* Keep locked, new owner now owns it */

		/* Change task state to READY */
		waiting_task->state = TS_READY;
		waiting_task->wait_next = NULL;
		waiting_task->wait_obj = NULL;
	} else {
		/* No waiting tasks, unlock mutex */
		mtx->locked = 0;
		mtx->owner = NULL;
	}

	/* TODO: Priority inheritance - restore original priority if needed */

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Create event flag
 * Input: x0 = initial flag pattern
 * Output: x0 = event flag ID or error (-1)
 */
static ER svc_cre_flg(SVC_REGS *regs)
{
	UW iflgptn = (UW)regs->x0;
	INT i;

	/* Find free event flag slot */
	for (i = 0; i < MAX_EVENTFLAGS; i++) {
		if (eventflag_table[i].flgid == 0) {
			/* Initialize event flag */
			eventflag_table[i].flgid = next_flgid++;
			eventflag_table[i].flgptn = iflgptn;
			eventflag_table[i].wait_queue = NULL;

			regs->x0 = eventflag_table[i].flgid;
			return E_OK;
		}
	}

	/* No free slots */
	regs->x0 = -1;
	return -1;
}

/*
 * System call: Set event flag bits
 * Input: x0 = event flag ID, x1 = set pattern
 * Output: x0 = E_OK or error
 */
static ER svc_set_flg(SVC_REGS *regs)
{
	ID flgid = (ID)regs->x0;
	UW setptn = (UW)regs->x1;
	FLGCB *flg = NULL;
	TCB *waiting_task;
	TCB *next_task;
	TCB **prev_ptr;
	INT i;
	UW match;

	/* Find event flag */
	for (i = 0; i < MAX_EVENTFLAGS; i++) {
		if (eventflag_table[i].flgid == flgid) {
			flg = &eventflag_table[i];
			break;
		}
	}

	if (flg == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Set the bits */
	flg->flgptn |= setptn;

	/* Check all waiting tasks */
	prev_ptr = &flg->wait_queue;
	waiting_task = flg->wait_queue;

	while (waiting_task != NULL) {
		next_task = waiting_task->wait_next;

		/* Check if condition is satisfied */
		if (waiting_task->wait_mode & TWF_ORW) {
			/* OR wait: at least one bit must match */
			match = flg->flgptn & waiting_task->wait_flgptn;
		} else {
			/* AND wait: all bits must match */
			match = (flg->flgptn & waiting_task->wait_flgptn) == waiting_task->wait_flgptn;
		}

		if (match) {
			/* Condition satisfied, wake up task */
			*prev_ptr = waiting_task->wait_next;

			/* Clear matched bits if TWF_CLR is set */
			if (waiting_task->wait_mode & TWF_CLR) {
				if (waiting_task->wait_mode & TWF_ORW) {
					/* OR wait: clear only the matched bits */
					flg->flgptn &= ~(flg->flgptn & waiting_task->wait_flgptn);
				} else {
					/* AND wait: clear all waited bits */
					flg->flgptn &= ~waiting_task->wait_flgptn;
				}
			}

			/* Change task state to READY */
			waiting_task->state = TS_READY;
			waiting_task->wait_next = NULL;
			waiting_task->wait_obj = NULL;
			waiting_task->wait_flgptn = 0;
			waiting_task->wait_mode = 0;
		} else {
			/* Condition not satisfied, move to next */
			prev_ptr = &waiting_task->wait_next;
		}

		waiting_task = next_task;
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Clear event flag bits
 * Input: x0 = event flag ID, x1 = clear pattern (inverted mask)
 * Output: x0 = E_OK or error
 */
static ER svc_clr_flg(SVC_REGS *regs)
{
	ID flgid = (ID)regs->x0;
	UW clrptn = (UW)regs->x1;
	FLGCB *flg = NULL;
	INT i;

	/* Find event flag */
	for (i = 0; i < MAX_EVENTFLAGS; i++) {
		if (eventflag_table[i].flgid == flgid) {
			flg = &eventflag_table[i];
			break;
		}
	}

	if (flg == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Clear the bits (clrptn is an inverted mask) */
	flg->flgptn &= clrptn;

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Wait for event flag
 * Input: x0 = event flag ID, x1 = wait pattern, x2 = wait mode
 * Output: x0 = E_OK or error
 */
static ER svc_wai_flg(SVC_REGS *regs)
{
	ID flgid = (ID)regs->x0;
	UW waiptn = (UW)regs->x1;
	UW wfmode = (UW)regs->x2;
	FLGCB *flg = NULL;
	TCB *current;
	TCB **queue_ptr;
	INT i;
	UW match;

	/* Find event flag */
	for (i = 0; i < MAX_EVENTFLAGS; i++) {
		if (eventflag_table[i].flgid == flgid) {
			flg = &eventflag_table[i];
			break;
		}
	}

	if (flg == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Check if condition is already satisfied */
	if (wfmode & TWF_ORW) {
		/* OR wait: at least one bit must match */
		match = flg->flgptn & waiptn;
	} else {
		/* AND wait: all bits must match */
		match = (flg->flgptn & waiptn) == waiptn;
	}

	if (match) {
		/* Condition already satisfied */
		if (wfmode & TWF_CLR) {
			/* Clear matched bits */
			if (wfmode & TWF_ORW) {
				/* OR wait: clear only the matched bits */
				flg->flgptn &= ~(flg->flgptn & waiptn);
			} else {
				/* AND wait: clear all waited bits */
				flg->flgptn &= ~waiptn;
			}
		}
		regs->x0 = E_OK;
		return E_OK;
	}

	/* Condition not satisfied, put task in wait queue */
	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Add to end of wait queue */
	current->state = TS_WAIT;
	current->wait_next = NULL;
	current->wait_obj = flg;
	current->wait_flgptn = waiptn;
	current->wait_mode = wfmode;

	if (flg->wait_queue == NULL) {
		flg->wait_queue = current;
	} else {
		/* Find end of queue */
		queue_ptr = &flg->wait_queue;
		while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
			queue_ptr = &((*queue_ptr)->wait_next);
		}
		(*queue_ptr)->wait_next = current;
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call handler (called from assembly)
 * x0 = SVC number
 * x1 = pointer to saved register context
 */
void svc_handler_c(UW svc_num, SVC_REGS *regs)
{
	switch (svc_num) {
	case SVC_GET_TID:
		svc_get_tid(regs);
		break;
	case SVC_DLY_TSK:
		svc_dly_tsk(regs);
		break;
	case SVC_GET_TIM:
		svc_get_tim(regs);
		break;
	case SVC_EXT_TSK:
		svc_ext_tsk(regs);
		break;
	case SVC_CRE_SEM:
		svc_cre_sem(regs);
		break;
	case SVC_SIG_SEM:
		svc_sig_sem(regs);
		break;
	case SVC_WAI_SEM:
		svc_wai_sem(regs);
		break;
	case SVC_CRE_MTX:
		svc_cre_mtx(regs);
		break;
	case SVC_LOC_MTX:
		svc_loc_mtx(regs);
		break;
	case SVC_UNL_MTX:
		svc_unl_mtx(regs);
		break;
	case SVC_CRE_FLG:
		svc_cre_flg(regs);
		break;
	case SVC_SET_FLG:
		svc_set_flg(regs);
		break;
	case SVC_CLR_FLG:
		svc_clr_flg(regs);
		break;
	case SVC_WAI_FLG:
		svc_wai_flg(regs);
		break;
	default:
		uart_puts("[SVC] Unknown system call: ");
		uart_puthex((UW)svc_num);
		uart_puts("\n");
		regs->x0 = -1;  /* Error */
		break;
	}
}

/*
 * User-space system call wrappers
 * These functions invoke system calls using the SVC instruction
 */

/* Get current task ID */
static inline ID tk_get_tid(void)
{
	register UW64 ret __asm__("x0");
	__asm__ volatile(
		"svc %1"
		: "=r"(ret)
		: "i"(SVC_GET_TID)
		: "memory"
	);
	return (ID)ret;
}

/* Get system time (timer tick count) */
static inline UW64 tk_get_tim(void)
{
	register UW64 ret __asm__("x0");
	__asm__ volatile(
		"svc %1"
		: "=r"(ret)
		: "i"(SVC_GET_TIM)
		: "memory"
	);
	return ret;
}

/* Task delay (in milliseconds) - implemented in user space */
static inline void tk_dly_tsk(UW64 dlytim)
{
	UW64 start = tk_get_tim();
	/* Busy wait in user space (interrupts are enabled here) */
	while ((tk_get_tim() - start) < dlytim) {
		/* Timer interrupts will update timer_tick_count */
	}
}

/* Create semaphore */
static inline ID tk_cre_sem(INT isemcnt, INT maxsem)
{
	register UW64 ret __asm__("x0") = isemcnt;
	register UW64 max __asm__("x1") = maxsem;
	__asm__ volatile(
		"svc %2"
		: "+r"(ret)
		: "r"(max), "i"(SVC_CRE_SEM)
		: "memory"
	);
	return (ID)ret;
}

/* Signal semaphore (release resource) */
static inline ER tk_sig_sem(ID semid)
{
	register UW64 ret __asm__("x0") = semid;
	__asm__ volatile(
		"svc %1"
		: "+r"(ret)
		: "i"(SVC_SIG_SEM)
		: "memory"
	);
	return (ER)ret;
}

/* Wait semaphore (acquire resource) */
static inline ER tk_wai_sem(ID semid)
{
	register UW64 ret __asm__("x0") = semid;
	__asm__ volatile(
		"svc %1"
		: "+r"(ret)
		: "i"(SVC_WAI_SEM)
		: "memory"
	);
	return (ER)ret;
}

/* Create mutex */
static inline ID tk_cre_mtx(void)
{
	register UW64 ret __asm__("x0");
	__asm__ volatile(
		"svc %1"
		: "=r"(ret)
		: "i"(SVC_CRE_MTX)
		: "memory"
	);
	return (ID)ret;
}

/* Lock mutex (acquire) */
static inline ER tk_loc_mtx(ID mtxid)
{
	register UW64 ret __asm__("x0") = mtxid;
	__asm__ volatile(
		"svc %1"
		: "+r"(ret)
		: "i"(SVC_LOC_MTX)
		: "memory"
	);
	return (ER)ret;
}

/* Unlock mutex (release) */
static inline ER tk_unl_mtx(ID mtxid)
{
	register UW64 ret __asm__("x0") = mtxid;
	__asm__ volatile(
		"svc %1"
		: "+r"(ret)
		: "i"(SVC_UNL_MTX)
		: "memory"
	);
	return (ER)ret;
}

/* Create event flag */
static inline ID tk_cre_flg(UW iflgptn)
{
	register UW64 ret __asm__("x0") = iflgptn;
	__asm__ volatile(
		"svc %1"
		: "+r"(ret)
		: "i"(SVC_CRE_FLG)
		: "memory"
	);
	return (ID)ret;
}

/* Set event flag bits */
static inline ER tk_set_flg(ID flgid, UW setptn)
{
	register UW64 ret __asm__("x0") = flgid;
	register UW64 ptn __asm__("x1") = setptn;
	__asm__ volatile(
		"svc %2"
		: "+r"(ret)
		: "r"(ptn), "i"(SVC_SET_FLG)
		: "memory"
	);
	return (ER)ret;
}

/* Clear event flag bits */
static inline ER tk_clr_flg(ID flgid, UW clrptn)
{
	register UW64 ret __asm__("x0") = flgid;
	register UW64 ptn __asm__("x1") = clrptn;
	__asm__ volatile(
		"svc %2"
		: "+r"(ret)
		: "r"(ptn), "i"(SVC_CLR_FLG)
		: "memory"
	);
	return (ER)ret;
}

/* Wait for event flag */
static inline ER tk_wai_flg(ID flgid, UW waiptn, UW wfmode)
{
	register UW64 ret __asm__("x0") = flgid;
	register UW64 ptn __asm__("x1") = waiptn;
	register UW64 mode __asm__("x2") = wfmode;
	__asm__ volatile(
		"svc %3"
		: "+r"(ret)
		: "r"(ptn), "r"(mode), "i"(SVC_WAI_FLG)
		: "memory"
	);
	return (ER)ret;
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
	UW64 mair, tcr, sctlr, mmfr0;
	UW parange;

	/* Ensure MMU is disabled before configuring */
	__asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
	sctlr &= ~(SCTLR_M | SCTLR_C | SCTLR_I);  /* Disable MMU and caches */
	__asm__ volatile("msr sctlr_el1, %0" :: "r"(sctlr));
	__asm__ volatile("isb");

	/* Invalidate TLB */
	__asm__ volatile("tlbi vmalle1");
	__asm__ volatile("dsb sy");
	__asm__ volatile("isb");

	/* Setup page tables */
	setup_page_tables();
	__asm__ volatile("dsb sy");

	/*
	 * Configure MAIR_EL1 (Memory Attribute Indirection Register)
	 * Index 0: Device-nGnRnE
	 * Index 1: Normal memory, non-cacheable
	 * Index 2: Normal memory, write-back cacheable
	 */
	mair = ((UW64)MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8))
	     | ((UW64)MAIR_NORMAL_NC << (MAIR_IDX_NORMAL_NC * 8))
	     | ((UW64)MAIR_NORMAL << (MAIR_IDX_NORMAL * 8));
	__asm__ volatile("msr mair_el1, %0" :: "r"(mair));

	/*
	 * Read CPU capabilities and configure TCR_EL1
	 */
	__asm__ volatile("mrs %0, id_aa64mmfr0_el1" : "=r"(mmfr0));
	parange = mmfr0 & 0xF;  /* Physical address range */

	/*
	 * TCR_EL1 configuration:
	 * - T0SZ=16: 48-bit VA (standard)
	 * - TG0=4KB: 4KB granule
	 * - IPS: from CPU capabilities
	 */
	tcr = TCR_T0SZ(16) | TCR_TG0_4KB | (parange << 32);
	__asm__ volatile("msr tcr_el1, %0" :: "r"(tcr));

	/* Set TTBR0_EL1 to page table base */
	__asm__ volatile("msr ttbr0_el1, %0" :: "r"((UW64)page_tables_l0));

	/* Synchronization barriers */
	__asm__ volatile("dsb sy");
	__asm__ volatile("isb");

	/*
	 * Enable MMU, instruction cache, and data cache
	 */
	__asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
	sctlr |= SCTLR_M | SCTLR_C | SCTLR_I;
	__asm__ volatile("msr sctlr_el1, %0" :: "r"(sctlr));
	__asm__ volatile("isb");
}

/*
 * Demo task functions - Event Flag demonstration
 */
static void task1_main(INT stacd, void *exinf)
{
	ID tid;
	UW64 time;
	INT count = 0;

	(void)stacd;
	(void)exinf;

	/* Get task ID using system call */
	tid = tk_get_tid();

	uart_puts("[Task1] Event flag setter started\n");

	while (1) {
		/* Simulate work */
		for (volatile int i = 0; i < 300000; i++);

		time = tk_get_tim();
		count++;

		/* Set different event flags */
		if (count % 3 == 1) {
			/* Set bit 0 (0x01) */
			uart_puts("[Task1] Setting flag bit 0 at ");
			uart_puthex((UW)time);
			uart_puts("ms\n");
			tk_set_flg(demo_flg, 0x01);
		} else if (count % 3 == 2) {
			/* Set bit 1 (0x02) */
			uart_puts("[Task1] Setting flag bit 1 at ");
			uart_puthex((UW)time);
			uart_puts("ms\n");
			tk_set_flg(demo_flg, 0x02);
		} else {
			/* Set bit 2 (0x04) */
			uart_puts("[Task1] Setting flag bit 2 at ");
			uart_puthex((UW)time);
			uart_puts("ms\n");
			tk_set_flg(demo_flg, 0x04);
		}

		/* Wait before next set */
		for (volatile int i = 0; i < 500000; i++);
	}
}

static void task2_main(INT stacd, void *exinf)
{
	ID tid;
	UW64 time;
	INT count = 0;

	(void)stacd;
	(void)exinf;

	/* Get task ID using system call */
	tid = tk_get_tid();

	uart_puts("[Task2] Event flag waiter started\n");

	while (1) {
		count++;
		time = tk_get_tim();

		if (count % 2 == 1) {
			/* Wait for bit 0 OR bit 1 (with auto-clear) */
			uart_puts("[Task2] Waiting for bit 0 OR bit 1 (TWF_ORW|TWF_CLR) at ");
			uart_puthex((UW)time);
			uart_puts("ms\n");
			tk_wai_flg(demo_flg, 0x03, TWF_ORW | TWF_CLR);

			time = tk_get_tim();
			uart_puts("[Task2] Woken up! (OR condition satisfied) at ");
			uart_puthex((UW)time);
			uart_puts("ms\n");
		} else {
			/* Wait for bit 2 (without auto-clear) */
			uart_puts("[Task2] Waiting for bit 2 (TWF_ANDW) at ");
			uart_puthex((UW)time);
			uart_puts("ms\n");
			tk_wai_flg(demo_flg, 0x04, TWF_ANDW);

			time = tk_get_tim();
			uart_puts("[Task2] Woken up! (AND condition satisfied) at ");
			uart_puthex((UW)time);
			uart_puts("ms\n");

			/* Manually clear bit 2 */
			tk_clr_flg(demo_flg, ~0x04);
		}

		/* Simulate processing */
		for (volatile int i = 0; i < 500000; i++);
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

	/* Initialize MMU with CPU feature detection */
	init_mmu();
	uart_puts("MMU enabled with I-cache and D-cache.\n");
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

	/* Initialize semaphores */
	uart_puts("Initializing semaphores...\n");
	for (INT i = 0; i < MAX_SEMAPHORES; i++) {
		semaphore_table[i].semid = 0;
	}

	/* Create demo semaphore (initial count=0, max=10) */
	demo_sem = tk_cre_sem(0, 10);
	uart_puts("Demo semaphore created: ID=");
	uart_puthex((UW)demo_sem);
	uart_puts("\n");

	/* Initialize mutexes */
	uart_puts("Initializing mutexes...\n");
	for (INT i = 0; i < MAX_MUTEXES; i++) {
		mutex_table[i].mtxid = 0;
	}

	/* Create demo mutex */
	demo_mtx = tk_cre_mtx();
	uart_puts("Demo mutex created: ID=");
	uart_puthex((UW)demo_mtx);
	uart_puts("\n");

	/* Initialize event flags */
	uart_puts("Initializing event flags...\n");
	for (INT i = 0; i < MAX_EVENTFLAGS; i++) {
		eventflag_table[i].flgid = 0;
	}

	/* Create demo event flag (initial pattern = 0) */
	demo_flg = tk_cre_flg(0);
	uart_puts("Demo event flag created: ID=");
	uart_puthex((UW)demo_flg);
	uart_puts("\n\n");

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
