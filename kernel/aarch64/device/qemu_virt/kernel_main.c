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
typedef int			bool;
#define true			1
#define false			0

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
	TS_WAIT		= 4,	/* Waiting */
	TS_SUSPEND	= 5,	/* Suspended */
	TS_WAITSUS	= 6	/* Waiting and Suspended */
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
	void		*wait_regs;	/* Saved register context (SVC_REGS*) for blocked tasks */

	/* Timeout information */
	SYSTIM		wait_timeout;	/* Timeout absolute time (0 = no timeout) */

	/* Task sleep/wakeup information */
	INT		wakeup_count;	/* Pending wakeup count */

	/* Task suspend/resume information */
	INT		suspend_count;	/* Suspend count (0=not suspended, >0=suspended) */
} TCB;

/*
 * Task management
 */
#define MAX_TASKS	4
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

/*
 * Message Buffer Control Block
 */
typedef struct {
	ID	mbfid;		/* Message buffer ID */
	UW	bufsz;		/* Buffer size in bytes */
	UW	maxmsz;		/* Maximum message size */
	UW	*buffer;	/* Ring buffer for messages */
	UW	head;		/* Head index (read position) */
	UW	tail;		/* Tail index (write position) */
	UW	freesz;		/* Free space in buffer */
	TCB	*send_queue;	/* Queue of sending tasks (buffer full) */
	TCB	*recv_queue;	/* Queue of receiving tasks (buffer empty) */
} MBFCB;

/*
 * Message buffer management
 */
#define MAX_MSGBUFFERS	4
#define MSGBUF_SIZE	256	/* 256 bytes per message buffer */
static MBFCB msgbuffer_table[MAX_MSGBUFFERS];
static UW msgbuffer_storage[MAX_MSGBUFFERS][MSGBUF_SIZE / sizeof(UW)] __attribute__((aligned(4)));
static INT next_mbfid = 1;	/* Next message buffer ID to allocate */

/* Shared message buffer for demo */
static ID demo_mbf = 0;		/* Will be initialized in kernel_main */

/*
 * Message Header for Mailbox
 */
typedef struct t_msg {
	struct t_msg	*next;		/* Next message in queue */
	INT		msgpri;		/* Message priority (higher = more urgent) */
	/* User data follows this header */
} T_MSG;

/*
 * Custom message structure extending T_MSG
 */
typedef struct {
	T_MSG	header;		/* Message header (must be first) */
	UW	sequence;	/* Message sequence number */
	UW	timestamp;	/* Timestamp when message was created */
	UW	value;		/* Data value */
	INT	pool_index;	/* Index in message pool (for deallocation) */
} MY_MSG;

/*
 * Message pool management
 */
#define MSG_POOL_SIZE	10
static MY_MSG msg_storage[MSG_POOL_SIZE];
static bool msg_in_use[MSG_POOL_SIZE];	/* Track which messages are allocated */

/*
 * Mailbox Control Block
 */
typedef struct {
	ID	mbxid;		/* Mailbox ID */
	T_MSG	*msg_queue;	/* Queue of messages (priority-ordered) */
	TCB	*recv_queue;	/* Queue of receiving tasks (mailbox empty) */
} MBXCB;

/*
 * Mailbox management
 */
#define MAX_MAILBOXES	4
static MBXCB mailbox_table[MAX_MAILBOXES];
static INT next_mbxid = 1;	/* Next mailbox ID to allocate */

/* Shared mailbox for demo */
static ID demo_mbx_comm = 0;	/* Will be initialized in kernel_main */

/* Task stacks (1KB each) */
#define TASK_STACK_SIZE	1024
static UW64 task1_stack[TASK_STACK_SIZE / sizeof(UW64)] __attribute__((aligned(16)));
static UW64 task2_stack[TASK_STACK_SIZE / sizeof(UW64)] __attribute__((aligned(16)));
static UW64 task3_stack[TASK_STACK_SIZE / sizeof(UW64)] __attribute__((aligned(16)));
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

	/* Set up task parameters */
	/* RESTORE_CONTEXT restores X0-X1 from sp[32-33] */
	sp[32] = 0;			/* X0 = stacd = 0 */
	sp[33] = (UW64)tcb->exinf;	/* X1 = exinf */

	/* X2-X29 are already zero from initialization */
	/* SP field (offset 31*8 = 248) will be ignored for EL1h tasks */

	/* Save context pointer (points to SPSR) */
	tcb->tskctxb.ssp = (void*)sp;
}

/* Create a task */
static ER create_task(INT idx, ID tskid, TASK_FP task, void *stack, INT stksz, INT priority)
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
	tcb->priority = priority;
	tcb->stack = stack;
	tcb->stksz = stksz;
	tcb->wait_next = NULL;
	tcb->wait_obj = NULL;
	tcb->wait_flgptn = 0;
	tcb->wait_mode = 0;
	tcb->wait_timeout = 0;
	tcb->wait_regs = NULL;
	tcb->wakeup_count = 0;
	tcb->suspend_count = 0;

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
	INT i;
	TCB *next_task = NULL;
	INT highest_priority = 999;  /* Lower number = higher priority */

	/* Find READY task with highest priority (lowest priority number) */
	for (i = 0; i < MAX_TASKS; i++) {
		if (task_table[i].state == TS_READY) {
			/* Select task with highest priority (lowest number) */
			if (task_table[i].priority < highest_priority) {
				highest_priority = task_table[i].priority;
				next_task = &task_table[i];
				current_task_idx = i;
			}
		}
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
#define SVC_CRE_MBF		15	/* Create message buffer */
#define SVC_SND_MBF		16	/* Send message to buffer */
#define SVC_RCV_MBF		17	/* Receive message from buffer */
#define SVC_CRE_MBX		18	/* Create mailbox */
#define SVC_SND_MBX		19	/* Send message to mailbox */
#define SVC_RCV_MBX		20	/* Receive message from mailbox */
#define SVC_WAI_SEM_U		21	/* Wait semaphore with timeout */
#define SVC_LOC_MTX_U		22	/* Lock mutex with timeout */
#define SVC_RCV_MBX_U		23	/* Receive mailbox with timeout */
#define SVC_WAI_FLG_U		24	/* Wait event flag with timeout */
#define SVC_SND_MBF_U		25	/* Send message buffer with timeout */
#define SVC_RCV_MBF_U		26	/* Receive message buffer with timeout */
#define SVC_SLP_TSK_U		27	/* Sleep task with timeout */
#define SVC_WUP_TSK		28	/* Wakeup task */
#define SVC_SUS_TSK		29	/* Suspend task */
#define SVC_RSM_TSK		30	/* Resume task */
#define SVC_FRSM_TSK		31	/* Force resume task */
#define SVC_EXT_TSK		32	/* Exit task (self termination) */
#define SVC_TER_TSK		33	/* Terminate task */
#define SVC_CHG_PRI		34	/* Change task priority */
#define SVC_ROT_RDQ		35	/* Rotate ready queue */

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
	TMO dlytim = (TMO)regs->x0;
	TCB *current;

	/* Handle zero delay - return immediately */
	if (dlytim == 0) {
		regs->x0 = E_OK;
		return E_OK;
	}

	/* Get current task */
	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = E_SYS;
		return E_SYS;
	}

	/* Put task to sleep with timeout */
	current->state = TS_WAIT;
	current->wait_next = NULL;
	current->wait_obj = NULL;  /* Not waiting on any object */
	current->wait_regs = regs;  /* Save register context */
	current->wait_timeout = timer_tick_count + dlytim;  /* Set absolute wakeup time */

	/* Schedule next task since current task is now waiting */
	schedule();

	/* When woken up by timeout, return E_OK */
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

		/* Reschedule to allow the woken task to run */
		schedule();
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

	/* Schedule next task since current task is now waiting */
	schedule();

	/* Force task switch - don't return E_OK yet */
	/* Task will be woken up by sig_sem and will return E_OK then */
	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Wait semaphore with timeout (acquire resource)
 * Input: x0 = semaphore ID, x1 = timeout (TMO)
 * Output: x0 = E_OK, E_TMOUT, or error
 */
static ER svc_wai_sem_u(SVC_REGS *regs)
{
	ID semid = (ID)regs->x0;
	TMO tmo = (TMO)regs->x1;
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
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Handle polling case (TMO_POL = 0) */
	if (tmo == TMO_POL) {
		if (sem->semcnt > 0) {
			sem->semcnt--;
			regs->x0 = E_OK;
			return E_OK;
		} else {
			regs->x0 = E_TMOUT;
			return E_TMOUT;
		}
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
		regs->x0 = E_SYS;
		return E_SYS;
	}

	/* Add to end of wait queue */
	current->state = TS_WAIT;
	current->wait_next = NULL;
	current->wait_obj = sem;
	current->wait_regs = regs;  /* Save register context for timeout handling */

	/* Set timeout: calculate absolute timeout time */
	if (tmo == TMO_FEVR) {
		current->wait_timeout = 0;  /* 0 means no timeout */
	} else {
		current->wait_timeout = timer_tick_count + tmo;
	}

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

	/* Schedule next task since current task is now waiting */
	schedule();

	/* Force task switch - don't return E_OK yet */
	/* Task will be woken up by sig_sem (return E_OK) or timeout (return E_TMOUT) */
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

	/* Schedule next task since current task is now waiting */
	schedule();

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
 * System call: Lock mutex with timeout
 * Input: x0 = mutex ID, x1 = timeout (TMO)
 * Output: x0 = E_OK, E_TMOUT, or error
 */
static ER svc_loc_mtx_u(SVC_REGS *regs)
{
	ID mtxid = (ID)regs->x0;
	TMO tmo = (TMO)regs->x1;
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
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = E_SYS;
		return E_SYS;
	}

	/* Handle polling case (TMO_POL = 0) */
	if (tmo == TMO_POL) {
		if (mtx->locked == 0) {
			/* Mutex available, lock it */
			mtx->locked = 1;
			mtx->owner = current;
			regs->x0 = E_OK;
			return E_OK;
		} else if (mtx->owner == current) {
			/* Error: recursive lock not supported */
			regs->x0 = E_SYS;
			return E_SYS;
		} else {
			/* Mutex locked by another task */
			regs->x0 = E_TMOUT;
			return E_TMOUT;
		}
	}

	/* Check if mutex is available */
	if (mtx->locked == 0) {
		/* Mutex available, lock it */
		mtx->locked = 1;
		mtx->owner = current;
		regs->x0 = E_OK;
		return E_OK;
	}

	/* Check if current task already owns it (recursive lock - not allowed) */
	if (mtx->owner == current) {
		regs->x0 = E_SYS;  /* Error: recursive lock not supported */
		return E_SYS;
	}

	/* Mutex is locked by another task, add to wait queue */
	current->state = TS_WAIT;
	current->wait_next = NULL;
	current->wait_obj = mtx;
	current->wait_regs = regs;  /* Save register context for timeout handling */

	/* Set timeout: calculate absolute timeout time */
	if (tmo == TMO_FEVR) {
		current->wait_timeout = 0;  /* 0 means no timeout */
	} else {
		current->wait_timeout = timer_tick_count + tmo;
	}

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

	/* Schedule next task since current task is now waiting */
	schedule();

	/* Task will be woken up by unl_mtx (E_OK) or timeout (E_TMOUT) */
	/* Note: The return value is set by either unl_mtx or check_timeouts */
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

			/* Set return value to E_OK if task has saved register context (timeout variant) */
			if (waiting_task->wait_regs != NULL) {
				((SVC_REGS *)waiting_task->wait_regs)->x0 = E_OK;
			}

			/* Clear timeout since task is being woken up successfully */
			waiting_task->wait_timeout = 0;

			/* Change task state to READY */
			waiting_task->state = TS_READY;
			waiting_task->wait_next = NULL;
			waiting_task->wait_obj = NULL;
			waiting_task->wait_flgptn = 0;
			waiting_task->wait_mode = 0;
			waiting_task->wait_regs = NULL;
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

	/* Schedule next task since current task is now waiting */
	schedule();

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Wait event flag with timeout
 * Input: x0 = event flag ID, x1 = wait pattern, x2 = wait mode, x3 = timeout
 * Output: x0 = E_OK, E_TMOUT, or error
 */
static ER svc_wai_flg_u(SVC_REGS *regs)
{
	ID flgid = (ID)regs->x0;
	UW waiptn = (UW)regs->x1;
	UW wfmode = (UW)regs->x2;
	TMO tmo = (TMO)regs->x3;
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
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Handle polling case (TMO_POL = 0) */
	if (tmo == TMO_POL) {
		/* Check if condition is already satisfied */
		if (wfmode & TWF_ORW) {
			/* OR wait: at least one bit must match */
			match = flg->flgptn & waiptn;
		} else {
			/* AND wait: all bits must match */
			match = (flg->flgptn & waiptn) == waiptn;
		}

		if (match) {
			/* Condition satisfied */
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
		} else {
			/* Condition not satisfied, return timeout immediately */
			regs->x0 = E_TMOUT;
			return E_TMOUT;
		}
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

	/* Condition not satisfied, add to wait queue with timeout */
	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = E_SYS;
		return E_SYS;
	}

	current->state = TS_WAIT;
	current->wait_next = NULL;
	current->wait_obj = flg;
	current->wait_flgptn = waiptn;
	current->wait_mode = wfmode;
	current->wait_regs = regs;  /* Save register context for timeout handling */

	/* Set timeout: calculate absolute timeout time */
	if (tmo == TMO_FEVR) {
		current->wait_timeout = 0;  /* 0 means no timeout */
	} else {
		current->wait_timeout = timer_tick_count + tmo;
	}

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

	/* Schedule next task since current task is now waiting */
	schedule();

	/* Task will be woken up by set_flg (E_OK) or timeout (E_TMOUT) */
	/* Note: The return value is set by either set_flg or check_timeouts */
	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Create message buffer
 * Arguments: x0 = bufsz (buffer size), x1 = maxmsz (max message size)
 * Returns: message buffer ID in x0 (or error)
 */
static ER svc_cre_mbf(SVC_REGS *regs)
{
	UW bufsz = (UW)regs->x0;
	UW maxmsz = (UW)regs->x1;
	MBFCB *mbf = NULL;
	INT i;

	/* Validate parameters */
	if (bufsz == 0 || bufsz > MSGBUF_SIZE || maxmsz == 0 || maxmsz > (bufsz - 2)) {
		regs->x0 = -1;
		return -1;
	}

	/* Find free message buffer */
	for (i = 0; i < MAX_MSGBUFFERS; i++) {
		if (msgbuffer_table[i].mbfid == 0) {
			mbf = &msgbuffer_table[i];
			break;
		}
	}

	if (mbf == NULL) {
		regs->x0 = -1;  /* No free message buffer */
		return -1;
	}

	/* Initialize message buffer */
	mbf->mbfid = next_mbfid++;
	mbf->bufsz = bufsz;
	mbf->maxmsz = maxmsz;
	mbf->buffer = msgbuffer_storage[i];
	mbf->head = 0;
	mbf->tail = 0;
	mbf->freesz = bufsz;
	mbf->send_queue = NULL;
	mbf->recv_queue = NULL;

	regs->x0 = mbf->mbfid;
	return E_OK;
}

/*
 * System call: Send message to buffer
 * Arguments: x0 = mbfid, x1 = msg pointer, x2 = msgsz
 * Returns: E_OK or error in x0
 */
static ER svc_snd_mbf(SVC_REGS *regs)
{
	ID mbfid = (ID)regs->x0;
	UW *msg = (UW *)regs->x1;
	UW msgsz = (UW)regs->x2;
	MBFCB *mbf = NULL;
	TCB *current;
	TCB *recv_task;
	TCB **queue_ptr;
	INT i;
	UW msg_total_sz;
	UW byte_idx;
	unsigned char *src_bytes;
	unsigned char *dst_bytes;

	/* Find message buffer */
	for (i = 0; i < MAX_MSGBUFFERS; i++) {
		if (msgbuffer_table[i].mbfid == mbfid) {
			mbf = &msgbuffer_table[i];
			break;
		}
	}

	if (mbf == NULL || msgsz == 0 || msgsz > mbf->maxmsz) {
		regs->x0 = -1;
		return -1;
	}

	/* Message format: [2-byte size][data] */
	msg_total_sz = 2 + msgsz;

	/* Check if there's a task waiting to receive */
	if (mbf->recv_queue != NULL) {
		/* Direct transfer to waiting task */
		recv_task = mbf->recv_queue;
		mbf->recv_queue = recv_task->wait_next;

		/* Copy message to receiver's buffer (stored in wait_obj as UW*) */
		UW *recv_buf = (UW *)recv_task->wait_obj;
		src_bytes = (unsigned char *)msg;
		dst_bytes = (unsigned char *)recv_buf;
		for (byte_idx = 0; byte_idx < msgsz; byte_idx++) {
			dst_bytes[byte_idx] = src_bytes[byte_idx];
		}

		/* Set message size in receiver's return value (x0) via saved register context */
		if (recv_task->wait_regs != NULL) {
			SVC_REGS *recv_regs = (SVC_REGS *)recv_task->wait_regs;
			recv_regs->x0 = msgsz;
		}

		/* Clear timeout since receiver is being woken up successfully */
		recv_task->wait_timeout = 0;

		/* Wake up receiver */
		recv_task->state = TS_READY;
		recv_task->wait_next = NULL;
		recv_task->wait_obj = NULL;
		recv_task->wait_regs = NULL;

		regs->x0 = E_OK;
		return E_OK;
	}

	/* Check if there's enough space in buffer */
	if (mbf->freesz < msg_total_sz) {
		/* Buffer full, add sender to wait queue */
		current = (TCB *)ctxtsk;
		if (current == NULL) {
			regs->x0 = -1;
			return -1;
		}

		/* Store message info in TCB for later */
		current->state = TS_WAIT;
		current->wait_next = NULL;
		current->wait_obj = (void *)msg;  /* Store message pointer */
		current->wait_flgptn = msgsz;     /* Store message size */

		/* Add to end of send queue */
		if (mbf->send_queue == NULL) {
			mbf->send_queue = current;
		} else {
			queue_ptr = &mbf->send_queue;
			while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
				queue_ptr = &((*queue_ptr)->wait_next);
			}
			(*queue_ptr)->wait_next = current;
		}

		/* Schedule next task since current task is now waiting */
		schedule();

		regs->x0 = E_OK;
		return E_OK;
	}

	/* Write message to buffer */
	dst_bytes = (unsigned char *)mbf->buffer;
	src_bytes = (unsigned char *)msg;

	/* Write size (2 bytes) */
	dst_bytes[(mbf->tail) % mbf->bufsz] = (unsigned char)(msgsz & 0xFF);
	dst_bytes[(mbf->tail + 1) % mbf->bufsz] = (unsigned char)((msgsz >> 8) & 0xFF);

	/* Write data */
	for (byte_idx = 0; byte_idx < msgsz; byte_idx++) {
		dst_bytes[(mbf->tail + 2 + byte_idx) % mbf->bufsz] = src_bytes[byte_idx];
	}

	mbf->tail = (mbf->tail + msg_total_sz) % mbf->bufsz;
	mbf->freesz -= msg_total_sz;

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Send message to buffer with timeout
 * Arguments: x0 = mbfid, x1 = msg buffer pointer, x2 = message size, x3 = timeout
 * Returns: E_OK, E_TMOUT, or error in x0
 */
static ER svc_snd_mbf_u(SVC_REGS *regs)
{
	ID mbfid = (ID)regs->x0;
	UW *msg = (UW *)regs->x1;
	UW msgsz = (UW)regs->x2;
	TMO tmo = (TMO)regs->x3;
	MBFCB *mbf = NULL;
	TCB *current;
	TCB *recv_task;
	TCB **queue_ptr;
	INT i;
	UW msg_total_sz;
	UW byte_idx;
	unsigned char *src_bytes;
	unsigned char *dst_bytes;

	/* Find message buffer */
	for (i = 0; i < MAX_MSGBUFFERS; i++) {
		if (msgbuffer_table[i].mbfid == mbfid) {
			mbf = &msgbuffer_table[i];
			break;
		}
	}

	if (mbf == NULL || msgsz == 0 || msgsz > mbf->maxmsz) {
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Message format: [2-byte size][data] */
	msg_total_sz = 2 + msgsz;

	/* Check if there's a task waiting to receive */
	if (mbf->recv_queue != NULL) {
		/* Direct transfer to waiting task */
		recv_task = mbf->recv_queue;
		mbf->recv_queue = recv_task->wait_next;

		/* Copy message to receiver's buffer (stored in wait_obj as UW*) */
		UW *recv_buf = (UW *)recv_task->wait_obj;
		src_bytes = (unsigned char *)msg;
		dst_bytes = (unsigned char *)recv_buf;
		for (byte_idx = 0; byte_idx < msgsz; byte_idx++) {
			dst_bytes[byte_idx] = src_bytes[byte_idx];
		}

		/* Set message size in receiver's return value (x0) via saved register context */
		if (recv_task->wait_regs != NULL) {
			SVC_REGS *recv_regs = (SVC_REGS *)recv_task->wait_regs;
			recv_regs->x0 = msgsz;
		}

		/* Clear timeout since receiver is being woken up successfully */
		recv_task->wait_timeout = 0;

		/* Wake up receiver */
		recv_task->state = TS_READY;
		recv_task->wait_next = NULL;
		recv_task->wait_obj = NULL;
		recv_task->wait_regs = NULL;

		regs->x0 = E_OK;
		return E_OK;
	}

	/* Handle polling case (TMO_POL = 0) */
	if (tmo == TMO_POL) {
		/* Check if there's enough space in buffer */
		if (mbf->freesz < msg_total_sz) {
			/* Buffer full, return timeout immediately */
			regs->x0 = E_TMOUT;
			return E_TMOUT;
		}
		/* Space available, fall through to write message */
	} else {
		/* Check if there's enough space in buffer */
		if (mbf->freesz < msg_total_sz) {
			/* Buffer full, add sender to wait queue with timeout */
			current = (TCB *)ctxtsk;
			if (current == NULL) {
				regs->x0 = E_SYS;
				return E_SYS;
			}

			/* Store message info in TCB for later */
			current->state = TS_WAIT;
			current->wait_next = NULL;
			current->wait_obj = (void *)msg;  /* Store message pointer */
			current->wait_flgptn = msgsz;     /* Store message size */
			current->wait_regs = regs;        /* Save register context for timeout handling */

			/* Set timeout: calculate absolute timeout time */
			if (tmo == TMO_FEVR) {
				current->wait_timeout = 0;  /* 0 means no timeout */
			} else {
				current->wait_timeout = timer_tick_count + tmo;
			}

			/* Add to end of send queue */
			if (mbf->send_queue == NULL) {
				mbf->send_queue = current;
			} else {
				queue_ptr = &mbf->send_queue;
				while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
					queue_ptr = &((*queue_ptr)->wait_next);
				}
				(*queue_ptr)->wait_next = current;
			}

			/* Schedule next task since current task is now waiting */
			schedule();

			/* Task will be woken up by rcv_mbf (E_OK) or timeout (E_TMOUT) */
			/* Note: The return value is set by either rcv_mbf or check_timeouts */
			regs->x0 = E_OK;
			return E_OK;
		}
		/* Space available, fall through to write message */
	}

	/* Write message to buffer */
	dst_bytes = (unsigned char *)mbf->buffer;
	src_bytes = (unsigned char *)msg;

	/* Write size (2 bytes) */
	dst_bytes[(mbf->tail) % mbf->bufsz] = (unsigned char)(msgsz & 0xFF);
	dst_bytes[(mbf->tail + 1) % mbf->bufsz] = (unsigned char)((msgsz >> 8) & 0xFF);

	/* Write data */
	for (byte_idx = 0; byte_idx < msgsz; byte_idx++) {
		dst_bytes[(mbf->tail + 2 + byte_idx) % mbf->bufsz] = src_bytes[byte_idx];
	}

	mbf->tail = (mbf->tail + msg_total_sz) % mbf->bufsz;
	mbf->freesz -= msg_total_sz;

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Receive message from buffer
 * Arguments: x0 = mbfid, x1 = msg buffer pointer
 * Returns: message size in x0 (or error)
 */
static ER svc_rcv_mbf(SVC_REGS *regs)
{
	ID mbfid = (ID)regs->x0;
	UW *msg = (UW *)regs->x1;
	MBFCB *mbf = NULL;
	TCB *current;
	TCB *send_task;
	TCB **queue_ptr;
	INT i;
	UW msgsz;
	UW msg_total_sz;
	UW byte_idx;
	unsigned char *src_bytes;
	unsigned char *dst_bytes;

	/* Find message buffer */
	for (i = 0; i < MAX_MSGBUFFERS; i++) {
		if (msgbuffer_table[i].mbfid == mbfid) {
			mbf = &msgbuffer_table[i];
			break;
		}
	}

	if (mbf == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Check if buffer has messages */
	if (mbf->freesz == mbf->bufsz) {
		/* Buffer empty, add to receive queue */
		current = (TCB *)ctxtsk;
		if (current == NULL) {
			regs->x0 = -1;
			return -1;
		}

		current->state = TS_WAIT;
		current->wait_next = NULL;
		current->wait_obj = (void *)msg;  /* Store receive buffer pointer */
		current->wait_regs = (void *)regs; /* Store register context for return value */

		/* Add to end of recv queue */
		if (mbf->recv_queue == NULL) {
			mbf->recv_queue = current;
		} else {
			queue_ptr = &mbf->recv_queue;
			while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
				queue_ptr = &((*queue_ptr)->wait_next);
			}
			(*queue_ptr)->wait_next = current;
		}
		/* Schedule next task since current task is now waiting */
		schedule();


		/* Don't set x0 here - it will be set when task is woken up */
		return E_OK;
	}

	/* Read message from buffer */
	src_bytes = (unsigned char *)mbf->buffer;
	dst_bytes = (unsigned char *)msg;

	/* Read size (2 bytes) */
	msgsz = src_bytes[mbf->head % mbf->bufsz];
	msgsz |= ((UW)src_bytes[(mbf->head + 1) % mbf->bufsz]) << 8;

	/* Read data */
	for (byte_idx = 0; byte_idx < msgsz; byte_idx++) {
		dst_bytes[byte_idx] = src_bytes[(mbf->head + 2 + byte_idx) % mbf->bufsz];
	}

	msg_total_sz = 2 + msgsz;
	mbf->head = (mbf->head + msg_total_sz) % mbf->bufsz;
	mbf->freesz += msg_total_sz;

	/* Check if any sender is waiting */
	if (mbf->send_queue != NULL) {
		send_task = mbf->send_queue;
		mbf->send_queue = send_task->wait_next;

		/* Get sender's message info */
		UW *send_msg = (UW *)send_task->wait_obj;
		UW send_msgsz = send_task->wait_flgptn;
		UW send_total_sz = 2 + send_msgsz;

		/* Check if there's now enough space */
		if (mbf->freesz >= send_total_sz) {
			/* Write sender's message to buffer */
			unsigned char *send_src = (unsigned char *)send_msg;

			/* Write size */
			src_bytes[(mbf->tail) % mbf->bufsz] = (unsigned char)(send_msgsz & 0xFF);
			src_bytes[(mbf->tail + 1) % mbf->bufsz] = (unsigned char)((send_msgsz >> 8) & 0xFF);

			/* Write data */
			for (byte_idx = 0; byte_idx < send_msgsz; byte_idx++) {
				src_bytes[(mbf->tail + 2 + byte_idx) % mbf->bufsz] = send_src[byte_idx];
			}

			mbf->tail = (mbf->tail + send_total_sz) % mbf->bufsz;
			mbf->freesz -= send_total_sz;

			/* Set return value to E_OK if sender has saved register context (timeout variant) */
			if (send_task->wait_regs != NULL) {
				((SVC_REGS *)send_task->wait_regs)->x0 = E_OK;
			}

			/* Clear timeout since sender is being woken up successfully */
			send_task->wait_timeout = 0;

			/* Wake up sender */
			send_task->state = TS_READY;
			send_task->wait_next = NULL;
			send_task->wait_obj = NULL;
			send_task->wait_flgptn = 0;
			send_task->wait_regs = NULL;
		} else {
			/* Still not enough space, put back in queue */
			send_task->wait_next = mbf->send_queue;
			mbf->send_queue = send_task;
		}
	}

	regs->x0 = msgsz;
	return E_OK;
}

/*
 * System call: Receive message from buffer with timeout
 * Parameters: x0 = mbfid, x1 = msg (pointer to receive buffer), x2 = tmo (timeout)
 * Returns: message size in x0 (or error)
 */
static ER svc_rcv_mbf_u(SVC_REGS *regs)
{
	ID mbfid = (ID)regs->x0;
	UW *msg = (UW *)regs->x1;
	TMO tmo = (TMO)regs->x2;
	MBFCB *mbf = NULL;
	TCB *current;
	TCB *send_task;
	TCB **queue_ptr;
	INT i;
	UW msgsz;
	UW msg_total_sz;
	UW byte_idx;
	unsigned char *src_bytes;
	unsigned char *dst_bytes;

	/* Find message buffer */
	for (i = 0; i < MAX_MSGBUFFERS; i++) {
		if (msgbuffer_table[i].mbfid == mbfid) {
			mbf = &msgbuffer_table[i];
			break;
		}
	}

	if (mbf == NULL) {
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Check if there's a sender waiting (direct transfer) */
	if (mbf->send_queue != NULL) {
		send_task = mbf->send_queue;
		mbf->send_queue = send_task->wait_next;

		/* Get sender's message info */
		UW *send_msg = (UW *)send_task->wait_obj;
		UW send_msgsz = send_task->wait_flgptn;

		/* Copy message directly to receiver's buffer */
		unsigned char *send_src = (unsigned char *)send_msg;
		dst_bytes = (unsigned char *)msg;
		for (byte_idx = 0; byte_idx < send_msgsz; byte_idx++) {
			dst_bytes[byte_idx] = send_src[byte_idx];
		}

		/* Set return value to E_OK if sender has saved register context (timeout variant) */
		if (send_task->wait_regs != NULL) {
			((SVC_REGS *)send_task->wait_regs)->x0 = E_OK;
		}

		/* Clear timeout since sender is being woken up successfully */
		send_task->wait_timeout = 0;

		/* Wake up sender */
		send_task->state = TS_READY;
		send_task->wait_next = NULL;
		send_task->wait_obj = NULL;
		send_task->wait_flgptn = 0;
		send_task->wait_regs = NULL;

		regs->x0 = send_msgsz;
		return E_OK;
	}

	/* Handle polling case (TMO_POL = 0) */
	if (tmo == TMO_POL) {
		if (mbf->freesz == mbf->bufsz) {
			/* Buffer empty */
			regs->x0 = E_TMOUT;
			return E_TMOUT;
		}
	} else {
		/* Check if buffer is empty */
		if (mbf->freesz == mbf->bufsz) {
			/* Buffer empty, add to receive queue with timeout */
			current = (TCB *)ctxtsk;
			if (current == NULL) {
				regs->x0 = E_SYS;
				return E_SYS;
			}

			current->state = TS_WAIT;
			current->wait_next = NULL;
			current->wait_obj = (void *)msg;  /* Store receive buffer pointer */
			current->wait_regs = regs;

			/* Set timeout */
			if (tmo == TMO_FEVR) {
				current->wait_timeout = 0;  /* 0 means no timeout */
			} else {
				current->wait_timeout = timer_tick_count + tmo;
			}

			/* Add to end of recv queue */
			if (mbf->recv_queue == NULL) {
				mbf->recv_queue = current;
			} else {
				queue_ptr = &mbf->recv_queue;
				while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
					queue_ptr = &((*queue_ptr)->wait_next);
				}
				(*queue_ptr)->wait_next = current;
			}

			schedule();

			regs->x0 = E_OK;
			return E_OK;
		}
	}

	/* Read message from buffer */
	src_bytes = (unsigned char *)mbf->buffer;
	dst_bytes = (unsigned char *)msg;

	/* Read size (2 bytes) */
	msgsz = src_bytes[mbf->head % mbf->bufsz];
	msgsz |= ((UW)src_bytes[(mbf->head + 1) % mbf->bufsz]) << 8;

	/* Read data */
	for (byte_idx = 0; byte_idx < msgsz; byte_idx++) {
		dst_bytes[byte_idx] = src_bytes[(mbf->head + 2 + byte_idx) % mbf->bufsz];
	}

	msg_total_sz = 2 + msgsz;
	mbf->head = (mbf->head + msg_total_sz) % mbf->bufsz;
	mbf->freesz += msg_total_sz;

	/* Check if any sender is waiting */
	if (mbf->send_queue != NULL) {
		send_task = mbf->send_queue;
		mbf->send_queue = send_task->wait_next;

		/* Get sender's message info */
		UW *send_msg = (UW *)send_task->wait_obj;
		UW send_msgsz = send_task->wait_flgptn;
		UW send_total_sz = 2 + send_msgsz;

		/* Check if there's now enough space */
		if (mbf->freesz >= send_total_sz) {
			/* Write sender's message to buffer */
			unsigned char *send_src = (unsigned char *)send_msg;

			/* Write size */
			src_bytes[(mbf->tail) % mbf->bufsz] = (unsigned char)(send_msgsz & 0xFF);
			src_bytes[(mbf->tail + 1) % mbf->bufsz] = (unsigned char)((send_msgsz >> 8) & 0xFF);

			/* Write data */
			for (byte_idx = 0; byte_idx < send_msgsz; byte_idx++) {
				src_bytes[(mbf->tail + 2 + byte_idx) % mbf->bufsz] = send_src[byte_idx];
			}

			mbf->tail = (mbf->tail + send_total_sz) % mbf->bufsz;
			mbf->freesz -= send_total_sz;

			/* Set return value to E_OK if sender has saved register context (timeout variant) */
			if (send_task->wait_regs != NULL) {
				((SVC_REGS *)send_task->wait_regs)->x0 = E_OK;
			}

			/* Clear timeout since sender is being woken up successfully */
			send_task->wait_timeout = 0;

			/* Wake up sender */
			send_task->state = TS_READY;
			send_task->wait_next = NULL;
			send_task->wait_obj = NULL;
			send_task->wait_flgptn = 0;
			send_task->wait_regs = NULL;
		} else {
			/* Still not enough space, put back in queue */
			send_task->wait_next = mbf->send_queue;
			mbf->send_queue = send_task;
		}
	}

	regs->x0 = msgsz;
	return E_OK;
}

/*
 * System call: Sleep task with timeout
 * Parameters: x0 = tmo (timeout)
 * Returns: E_OK or error in x0
 */
static ER svc_slp_tsk_u(SVC_REGS *regs)
{
	TMO tmo = (TMO)regs->x0;
	TCB *current;

	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = E_SYS;
		return E_SYS;
	}

	/* Handle polling case (TMO_POL = 0) */
	if (tmo == TMO_POL) {
		if (current->wakeup_count > 0) {
			/* Wakeup request is pending */
			current->wakeup_count--;
			regs->x0 = E_OK;
			return E_OK;
		} else {
			/* No wakeup pending */
			regs->x0 = E_TMOUT;
			return E_TMOUT;
		}
	}

	/* Check if there's already a pending wakeup */
	if (current->wakeup_count > 0) {
		current->wakeup_count--;
		regs->x0 = E_OK;
		return E_OK;
	}

	/* Put task to sleep */
	current->state = TS_WAIT;
	current->wait_obj = NULL;  /* NULL means not waiting on any object */
	current->wait_flgptn = 1;  /* Flag to indicate sleep (not delay) */
	current->wait_regs = regs;

	/* Set timeout */
	if (tmo == TMO_FEVR) {
		current->wait_timeout = 0;  /* 0 means no timeout */
	} else {
		current->wait_timeout = timer_tick_count + tmo;
	}

	schedule();

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Wakeup task
 * Parameters: x0 = tskid (task ID)
 * Returns: E_OK or error in x0
 */
static ER svc_wup_tsk(SVC_REGS *regs)
{
	ID tskid = (ID)regs->x0;
	TCB *target_task = NULL;
	INT i;

	/* Find target task */
	for (i = 0; i < MAX_TASKS; i++) {
		if (task_table[i].tskid == tskid) {
			target_task = &task_table[i];
			break;
		}
	}

	if (target_task == NULL) {
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Check task state */
	if (target_task->state == TS_WAIT && target_task->wait_obj == NULL) {
		/* Task is sleeping, wake it up */
		target_task->state = TS_READY;

		/* Set return value to E_OK if task has saved register context */
		if (target_task->wait_regs != NULL) {
			((SVC_REGS *)target_task->wait_regs)->x0 = E_OK;
		}

		/* Clear timeout since task is being woken up */
		target_task->wait_timeout = 0;
		target_task->wait_regs = NULL;

		regs->x0 = E_OK;
		return E_OK;
	} else if (target_task->state == TS_READY || target_task->state == TS_RUN) {
		/* Task is not sleeping, queue the wakeup request */
		target_task->wakeup_count++;
		regs->x0 = E_OK;
		return E_OK;
	} else {
		/* Task is waiting on something else (not sleeping) */
		regs->x0 = E_OBJ;  /* Object state error */
		return E_OBJ;
	}
}

/*
 * System call: Suspend task
 * Arguments: x0 = tskid
 * Returns: E_OK or error code
 */
static ER svc_sus_tsk(SVC_REGS *regs)
{
	ID tskid = (ID)regs->x0;
	TCB *target_task = NULL;
	TCB *current;
	INT i;

	/* Cannot suspend self */
	current = (TCB *)ctxtsk;
	if (current != NULL && current->tskid == tskid) {
		regs->x0 = E_OBJ;
		return E_OBJ;
	}

	/* Find target task */
	for (i = 0; i < MAX_TASKS; i++) {
		if (task_table[i].tskid == tskid) {
			target_task = &task_table[i];
			break;
		}
	}

	if (target_task == NULL) {
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Increment suspend count */
	target_task->suspend_count++;

	/* If this is the first suspend, change state */
	if (target_task->suspend_count == 1) {
		if (target_task->state == TS_READY || target_task->state == TS_RUN) {
			/* Change to suspended state */
			target_task->state = TS_SUSPEND;
			/* If suspending running task, trigger reschedule */
			if (target_task == current) {
				schedule();
			}
		} else if (target_task->state == TS_WAIT) {
			/* Change to waiting+suspended state */
			target_task->state = TS_WAITSUS;
		}
		/* If already SUSPEND or WAITSUS, count is already >0, no state change needed */
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Resume task
 * Arguments: x0 = tskid
 * Returns: E_OK or error code
 */
static ER svc_rsm_tsk(SVC_REGS *regs)
{
	ID tskid = (ID)regs->x0;
	TCB *target_task = NULL;
	INT i;

	/* Find target task */
	for (i = 0; i < MAX_TASKS; i++) {
		if (task_table[i].tskid == tskid) {
			target_task = &task_table[i];
			break;
		}
	}

	if (target_task == NULL) {
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Check if task is suspended */
	if (target_task->suspend_count == 0) {
		regs->x0 = E_OBJ;  /* Task is not suspended */
		return E_OBJ;
	}

	/* Decrement suspend count */
	target_task->suspend_count--;

	/* If count reaches 0, resume task */
	if (target_task->suspend_count == 0) {
		if (target_task->state == TS_SUSPEND) {
			/* Change to ready state */
			target_task->state = TS_READY;
		} else if (target_task->state == TS_WAITSUS) {
			/* Change back to waiting state */
			target_task->state = TS_WAIT;
		}
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Force resume task
 * Arguments: x0 = tskid
 * Returns: E_OK or error code
 */
static ER svc_frsm_tsk(SVC_REGS *regs)
{
	ID tskid = (ID)regs->x0;
	TCB *target_task = NULL;
	INT i;

	/* Find target task */
	for (i = 0; i < MAX_TASKS; i++) {
		if (task_table[i].tskid == tskid) {
			target_task = &task_table[i];
			break;
		}
	}

	if (target_task == NULL) {
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Check if task is suspended */
	if (target_task->suspend_count == 0) {
		regs->x0 = E_OBJ;  /* Task is not suspended */
		return E_OBJ;
	}

	/* Force suspend count to 0 */
	target_task->suspend_count = 0;

	/* Resume task regardless of how many times it was suspended */
	if (target_task->state == TS_SUSPEND) {
		/* Change to ready state */
		target_task->state = TS_READY;
	} else if (target_task->state == TS_WAITSUS) {
		/* Change back to waiting state */
		target_task->state = TS_WAIT;
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: tk_ext_tsk - Exit task (self termination)
 * No arguments
 * No return value (does not return to caller)
 */
static void svc_ext_tsk(SVC_REGS *regs)
{
	TCB *current = (TCB *)ctxtsk;
	INT i;

	(void)regs;  /* Unused */

	if (current == NULL) {
		return;  /* Should not happen */
	}

	/* Change state to DORMANT */
	current->state = TS_DORMANT;

	/* Reset suspend count */
	current->suspend_count = 0;

	/* Remove from ready queue if present */
	/* (Not needed as state change handles this) */

	/* Remove from waiting queues and release resources */
	/* Check semaphores */
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphore_table[i].semid != 0) {
			TCB **queue = &semaphore_table[i].wait_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == current) {
					/* Remove from wait queue */
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Check mutexes - if task holds any mutex, unlock it */
	for (i = 0; i < MAX_MUTEXES; i++) {
		if (mutex_table[i].mtxid != 0) {
			if (mutex_table[i].locked && mutex_table[i].owner == current) {
				/* Unlock the mutex */
				mutex_table[i].locked = false;
				mutex_table[i].owner = NULL;

				/* Wake up first waiting task */
				TCB *waiting = mutex_table[i].wait_queue;
				if (waiting != NULL) {
					mutex_table[i].wait_queue = waiting->wait_next;
					waiting->wait_next = NULL;
					mutex_table[i].locked = true;
					mutex_table[i].owner = waiting;
					waiting->state = TS_READY;
				}
			}

			/* Remove from wait queue if waiting */
			TCB **queue = &mutex_table[i].wait_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == current) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Check event flags */
	for (i = 0; i < MAX_EVENTFLAGS; i++) {
		if (eventflag_table[i].flgid != 0) {
			TCB **queue = &eventflag_table[i].wait_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == current) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Check message buffers */
	for (i = 0; i < MAX_MSGBUFFERS; i++) {
		if (msgbuffer_table[i].mbfid != 0) {
			TCB **queue = &msgbuffer_table[i].recv_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == current) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Check mailboxes */
	for (i = 0; i < MAX_MAILBOXES; i++) {
		if (mailbox_table[i].mbxid != 0) {
			TCB **queue = &mailbox_table[i].recv_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == current) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Cancel timeout if active */
	current->wait_timeout = 0;

	/* Set ctxtsk to NULL since current task is terminated */
	ctxtsk = NULL;

	/* Reschedule - will not return */
	schedule();
}

/*
 * System call: tk_ter_tsk - Terminate task
 * Arguments: x0 = task ID
 * Returns: E_OK, E_NOEXS, or E_OBJ in x0
 */
static ER svc_ter_tsk(SVC_REGS *regs)
{
	ID tskid = (ID)regs->x0;
	TCB *current = (TCB *)ctxtsk;
	TCB *target_task = NULL;
	INT i;

	/* Cannot terminate self (use tk_ext_tsk instead) */
	if (current != NULL && current->tskid == tskid) {
		regs->x0 = E_OBJ;
		return E_OBJ;
	}

	/* Find target task */
	for (i = 0; i < MAX_TASKS; i++) {
		if (task_table[i].tskid == tskid) {
			target_task = &task_table[i];
			break;
		}
	}

	if (target_task == NULL) {
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Cannot terminate task in DORMANT or NONEXIST state */
	if (target_task->state == TS_DORMANT || target_task->state == TS_NONEXIST) {
		regs->x0 = E_OBJ;
		return E_OBJ;
	}

	/* Change state to DORMANT */
	target_task->state = TS_DORMANT;

	/* Reset suspend count */
	target_task->suspend_count = 0;

	/* Remove from waiting queues and release resources */
	/* Check semaphores */
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphore_table[i].semid != 0) {
			TCB **queue = &semaphore_table[i].wait_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == target_task) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Check mutexes - if task holds any mutex, unlock it */
	for (i = 0; i < MAX_MUTEXES; i++) {
		if (mutex_table[i].mtxid != 0) {
			if (mutex_table[i].locked && mutex_table[i].owner == target_task) {
				/* Unlock the mutex */
				mutex_table[i].locked = false;
				mutex_table[i].owner = NULL;

				/* Wake up first waiting task */
				TCB *waiting = mutex_table[i].wait_queue;
				if (waiting != NULL) {
					mutex_table[i].wait_queue = waiting->wait_next;
					waiting->wait_next = NULL;
					mutex_table[i].locked = true;
					mutex_table[i].owner = waiting;
					waiting->state = TS_READY;
				}
			}

			/* Remove from wait queue if waiting */
			TCB **queue = &mutex_table[i].wait_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == target_task) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Check event flags */
	for (i = 0; i < MAX_EVENTFLAGS; i++) {
		if (eventflag_table[i].flgid != 0) {
			TCB **queue = &eventflag_table[i].wait_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == target_task) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Check message buffers */
	for (i = 0; i < MAX_MSGBUFFERS; i++) {
		if (msgbuffer_table[i].mbfid != 0) {
			TCB **queue = &msgbuffer_table[i].recv_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == target_task) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Check mailboxes */
	for (i = 0; i < MAX_MAILBOXES; i++) {
		if (mailbox_table[i].mbxid != 0) {
			TCB **queue = &mailbox_table[i].recv_queue;
			TCB *prev = NULL;
			TCB *task = *queue;

			while (task != NULL) {
				if (task == target_task) {
					if (prev == NULL) {
						*queue = task->wait_next;
					} else {
						prev->wait_next = task->wait_next;
					}
					task->wait_next = NULL;
					break;
				}
				prev = task;
				task = task->wait_next;
			}
		}
	}

	/* Cancel timeout if active */
	target_task->wait_timeout = 0;

	/* No need to reschedule unless target was running */
	/* (Scheduler will skip DORMANT tasks naturally) */

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: tk_chg_pri - Change task priority
 * Arguments: x0 = task ID, x1 = new priority
 * Returns: E_OK, E_NOEXS, E_OBJ, or E_PAR in x0
 *
 * Priority: Lower number = higher priority (T-Kernel convention)
 * Special values:
 *   - tskid == 0: change current task's priority
 *   - priority == 0: invalid (reserved)
 */
static ER svc_chg_pri(SVC_REGS *regs)
{
	ID tskid = (ID)regs->x0;
	INT tskpri = (INT)regs->x1;
	TCB *target_task = NULL;
	TCB *current = (TCB *)ctxtsk;
	INT i;

	/* tskid == 0 means change current task's priority */
	if (tskid == 0) {
		if (current == NULL) {
			regs->x0 = E_OBJ;
			return E_OBJ;
		}
		target_task = current;
	} else {
		/* Find target task */
		for (i = 0; i < MAX_TASKS; i++) {
			if (task_table[i].tskid == tskid) {
				target_task = &task_table[i];
				break;
			}
		}

		if (target_task == NULL) {
			regs->x0 = E_NOEXS;
			return E_NOEXS;
		}
	}

	/* Check if task is in valid state */
	if (target_task->state == TS_NONEXIST || target_task->state == TS_DORMANT) {
		regs->x0 = E_OBJ;
		return E_OBJ;
	}

	/* Validate priority (0 is reserved, must be > 0) */
	if (tskpri <= 0 || tskpri > 140) {  /* T-Kernel typical range: 1-140 */
		regs->x0 = E_PAR;
		return E_PAR;
	}

	/* Change priority */
	target_task->priority = tskpri;

	/* If target task is READY and priority changed, may need to reschedule */
	if (target_task->state == TS_READY || target_task->state == TS_RUN) {
		/* Simple approach: trigger reschedule to re-evaluate priorities */
		/* The scheduler will pick the highest priority task */
		schedule();
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: tk_rot_rdq - Rotate ready queue
 * Arguments: x0 = priority level (0 = current priority)
 * Returns: E_OK or E_PAR in x0
 *
 * Rotates the ready queue at the specified priority level.
 * The current running task moves to the end of its priority level.
 * This implements round-robin scheduling within same priority.
 */
static ER svc_rot_rdq(SVC_REGS *regs)
{
	INT tskpri = (INT)regs->x0;
	TCB *current = (TCB *)ctxtsk;

	/* tskpri == 0 means use current task's priority */
	if (tskpri == 0) {
		if (current == NULL) {
			regs->x0 = E_OK;  /* No current task, nothing to rotate */
			return E_OK;
		}
		tskpri = current->priority;
	}

	/* Validate priority */
	if (tskpri < 0 || tskpri > 140) {
		regs->x0 = E_PAR;
		return E_PAR;
	}

	/* For now, just trigger reschedule to give other tasks a chance */
	/* A full implementation would maintain separate queues per priority level */
	/* and rotate within that level. For simplicity, we just reschedule. */
	schedule();

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Create mailbox
 * Returns: mailbox ID in x0 (or error)
 */
static ER svc_cre_mbx(SVC_REGS *regs)
{
	MBXCB *mbx = NULL;
	INT i;

	/* Find free mailbox */
	for (i = 0; i < MAX_MAILBOXES; i++) {
		if (mailbox_table[i].mbxid == 0) {
			mbx = &mailbox_table[i];
			break;
		}
	}

	if (mbx == NULL) {
		regs->x0 = -1;  /* No free mailbox */
		return -1;
	}

	/* Initialize mailbox */
	mbx->mbxid = next_mbxid++;
	mbx->msg_queue = NULL;
	mbx->recv_queue = NULL;

	regs->x0 = mbx->mbxid;
	return E_OK;
}

/*
 * System call: Send message to mailbox
 * Arguments: x0 = mbxid, x1 = message pointer (T_MSG*)
 * Returns: E_OK or error in x0
 */
static ER svc_snd_mbx(SVC_REGS *regs)
{
	ID mbxid = (ID)regs->x0;
	T_MSG *msg = (T_MSG *)regs->x1;
	MBXCB *mbx = NULL;
	TCB *recv_task;
	T_MSG **msg_ptr;
	INT i;

	/* Find mailbox */
	for (i = 0; i < MAX_MAILBOXES; i++) {
		if (mailbox_table[i].mbxid == mbxid) {
			mbx = &mailbox_table[i];
			break;
		}
	}

	if (mbx == NULL || msg == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Check if there's a task waiting to receive */
	if (mbx->recv_queue != NULL) {
		/* Direct transfer to waiting task */
		recv_task = mbx->recv_queue;
		mbx->recv_queue = recv_task->wait_next;

		/* Set message pointer in receiver's return value via saved register context */
		if (recv_task->wait_regs != NULL) {
			SVC_REGS *recv_regs = (SVC_REGS *)recv_task->wait_regs;

			/* Check if this is svc_rcv_mbx_u (has ppk_msg parameter in x2) */
			/* For svc_rcv_mbx_u: x0=mbxid, x1=tmo, x2=ppk_msg */
			/* For svc_rcv_mbx: x0=mbxid, x1/x2 undefined */
			/* We detect _u variant by checking if x2 looks like a valid stack pointer */
			T_MSG **ppk_msg = (T_MSG **)recv_regs->x2;

			if (ppk_msg != NULL && (UW64)ppk_msg >= 0x40000000 && (UW64)ppk_msg < 0x80000000) {
				/* This is svc_rcv_mbx_u - return via output parameter */
				*ppk_msg = msg;
				recv_regs->x0 = E_OK;
			} else {
				/* This is svc_rcv_mbx - return message pointer in x0 */
				recv_regs->x0 = (UW64)msg;
			}
		}

		/* Clear timeout since task is being woken up successfully */
		recv_task->wait_timeout = 0;

		/* Wake up receiver */
		recv_task->state = TS_READY;
		recv_task->wait_next = NULL;
		recv_task->wait_obj = NULL;
		recv_task->wait_regs = NULL;

		regs->x0 = E_OK;
		return E_OK;
	}

	/* Add message to queue in priority order (higher priority first) */
	msg->next = NULL;

	if (mbx->msg_queue == NULL) {
		/* Queue is empty, add as first message */
		mbx->msg_queue = msg;
	} else {
		/* Find insertion point based on priority */
		msg_ptr = &mbx->msg_queue;
		while (*msg_ptr != NULL && (*msg_ptr)->msgpri >= msg->msgpri) {
			msg_ptr = &((*msg_ptr)->next);
		}
		/* Insert message */
		msg->next = *msg_ptr;
		*msg_ptr = msg;
	}

	regs->x0 = E_OK;
	return E_OK;
}

/*
 * System call: Receive message from mailbox
 * Arguments: x0 = mbxid, x1 = pointer to receive message pointer (T_MSG**)
 * Returns: message pointer in x0 (or error)
 */
static ER svc_rcv_mbx(SVC_REGS *regs)
{
	ID mbxid = (ID)regs->x0;
	MBXCB *mbx = NULL;
	TCB *current;
	TCB **queue_ptr;
	T_MSG *msg;
	INT i;

	/* Find mailbox */
	for (i = 0; i < MAX_MAILBOXES; i++) {
		if (mailbox_table[i].mbxid == mbxid) {
			mbx = &mailbox_table[i];
			break;
		}
	}

	if (mbx == NULL) {
		regs->x0 = -1;
		return -1;
	}

	/* Check if mailbox has messages */
	if (mbx->msg_queue == NULL) {
		/* Mailbox empty, add to receive queue */
		current = (TCB *)ctxtsk;
		if (current == NULL) {
			regs->x0 = -1;
			return -1;
		}

		current->state = TS_WAIT;
		current->wait_next = NULL;
		current->wait_obj = (void *)mbx;
		current->wait_regs = (void *)regs; /* Store register context for return value */

		/* Add to end of recv queue */
		if (mbx->recv_queue == NULL) {
			mbx->recv_queue = current;
		} else {
			queue_ptr = &mbx->recv_queue;
			while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
				queue_ptr = &((*queue_ptr)->wait_next);
			}
			(*queue_ptr)->wait_next = current;
		}

		/* Schedule next task since current task is now waiting */
		schedule();

		/* Don't set x0 here - it will be set when task is woken up by snd_mbx() */
		return E_OK;
	}

	/* Get message from queue (highest priority first) */
	msg = mbx->msg_queue;

	/* Validate message pointer is within msg_storage array */
	if (msg < (T_MSG*)msg_storage || msg >= (T_MSG*)(msg_storage + MSG_POOL_SIZE)) {
		/* Invalid message pointer - mailbox is corrupted */
		uart_puts("ERROR: Invalid message pointer in mailbox queue: ");
		uart_puthex((UW64)msg);
		uart_puts("\n");
		/* Reset mailbox to prevent further corruption */
		mbx->msg_queue = NULL;
		regs->x0 = -1;
		return -1;
	}

	/* Validate next pointer before using it */
	if (msg->next != NULL &&
	    (msg->next < (T_MSG*)msg_storage || msg->next >= (T_MSG*)(msg_storage + MSG_POOL_SIZE))) {
		/* Invalid next pointer - set to NULL to prevent propagation */
		uart_puts("WARNING: Invalid next pointer in message, setting to NULL: ");
		uart_puthex((UW64)msg->next);
		uart_puts("\n");
		msg->next = NULL;
	}

	mbx->msg_queue = msg->next;
	msg->next = NULL;

	/* Return message pointer */
	regs->x0 = (UW64)msg;
	return E_OK;
}

/*
 * System call: Receive message from mailbox with timeout
 * Input: x0 = mailbox ID, x1 = timeout (TMO), x2 = pointer to message pointer
 * Output: x0 = E_OK, E_TMOUT, or error
 *         *ppk_msg = received message pointer (on success)
 */
static ER svc_rcv_mbx_u(SVC_REGS *regs)
{
	ID mbxid = (ID)regs->x0;
	TMO tmo = (TMO)regs->x1;
	T_MSG **ppk_msg = (T_MSG **)regs->x2;
	MBXCB *mbx = NULL;
	TCB *current;
	TCB **queue_ptr;
	T_MSG *msg;
	INT i;

	/* Find mailbox */
	for (i = 0; i < MAX_MAILBOXES; i++) {
		if (mailbox_table[i].mbxid == mbxid) {
			mbx = &mailbox_table[i];
			break;
		}
	}

	if (mbx == NULL) {
		regs->x0 = E_NOEXS;
		return E_NOEXS;
	}

	/* Handle polling case (TMO_POL = 0) */
	if (tmo == TMO_POL) {
		if (mbx->msg_queue == NULL) {
			/* Mailbox empty */
			regs->x0 = E_TMOUT;
			return E_TMOUT;
		} else {
			/* Get message from queue */
			msg = mbx->msg_queue;

			/* Validate message pointer */
			if (msg < (T_MSG*)msg_storage || msg >= (T_MSG*)(msg_storage + MSG_POOL_SIZE)) {
				mbx->msg_queue = NULL;
				regs->x0 = E_SYS;
				return E_SYS;
			}

			/* Validate next pointer */
			if (msg->next != NULL &&
			    (msg->next < (T_MSG*)msg_storage || msg->next >= (T_MSG*)(msg_storage + MSG_POOL_SIZE))) {
				msg->next = NULL;
			}

			mbx->msg_queue = msg->next;
			msg->next = NULL;

			/* Return message pointer via output parameter */
			*ppk_msg = msg;
			regs->x0 = E_OK;
			return E_OK;
		}
	}

	/* Check if mailbox has messages */
	if (mbx->msg_queue != NULL) {
		/* Get message from queue (highest priority first) */
		msg = mbx->msg_queue;

		/* Validate message pointer */
		if (msg < (T_MSG*)msg_storage || msg >= (T_MSG*)(msg_storage + MSG_POOL_SIZE)) {
			uart_puts("ERROR: Invalid message pointer in mailbox queue: ");
			uart_puthex((UW64)msg);
			uart_puts("\n");
			mbx->msg_queue = NULL;
			regs->x0 = E_SYS;
			return E_SYS;
		}

		/* Validate next pointer */
		if (msg->next != NULL &&
		    (msg->next < (T_MSG*)msg_storage || msg->next >= (T_MSG*)(msg_storage + MSG_POOL_SIZE))) {
			uart_puts("WARNING: Invalid next pointer in message, setting to NULL: ");
			uart_puthex((UW64)msg->next);
			uart_puts("\n");
			msg->next = NULL;
		}

		mbx->msg_queue = msg->next;
		msg->next = NULL;

		/* Return message pointer via output parameter */
		*ppk_msg = msg;
		regs->x0 = E_OK;
		return E_OK;
	}

	/* Mailbox empty, add to receive queue with timeout */
	current = (TCB *)ctxtsk;
	if (current == NULL) {
		regs->x0 = E_SYS;
		return E_SYS;
	}

	current->state = TS_WAIT;
	current->wait_next = NULL;
	current->wait_obj = (void *)mbx;
	current->wait_regs = (void *)regs; /* Store register context for return value */

	/* Set timeout: calculate absolute timeout time */
	if (tmo == TMO_FEVR) {
		current->wait_timeout = 0;  /* 0 means no timeout */
	} else {
		current->wait_timeout = timer_tick_count + tmo;
	}

	/* Add to end of recv queue */
	if (mbx->recv_queue == NULL) {
		mbx->recv_queue = current;
	} else {
		queue_ptr = &mbx->recv_queue;
		while (*queue_ptr != NULL && (*queue_ptr)->wait_next != NULL) {
			queue_ptr = &((*queue_ptr)->wait_next);
		}
		(*queue_ptr)->wait_next = current;
	}

	/* Schedule next task since current task is now waiting */
	schedule();

	/* Task will be woken up by snd_mbx (E_OK) or timeout (E_TMOUT) */
	/* Note: The return value is set by either snd_mbx or check_timeouts */
	/* For now, return E_OK - will be overwritten on timeout */
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
	case SVC_CRE_MBF:
		svc_cre_mbf(regs);
		break;
	case SVC_SND_MBF:
		svc_snd_mbf(regs);
		break;
	case SVC_RCV_MBF:
		svc_rcv_mbf(regs);
		break;
	case SVC_CRE_MBX:
		svc_cre_mbx(regs);
		break;
	case SVC_SND_MBX:
		svc_snd_mbx(regs);
		break;
	case SVC_RCV_MBX:
		svc_rcv_mbx(regs);
		break;
	case SVC_WAI_SEM_U:
		svc_wai_sem_u(regs);
		break;
	case SVC_LOC_MTX_U:
		svc_loc_mtx_u(regs);
		break;
	case SVC_RCV_MBX_U:
		svc_rcv_mbx_u(regs);
		break;
	case SVC_WAI_FLG_U:
		svc_wai_flg_u(regs);
		break;
	case SVC_SND_MBF_U:
		svc_snd_mbf_u(regs);
		break;
	case SVC_RCV_MBF_U:
		svc_rcv_mbf_u(regs);
		break;
	case SVC_SLP_TSK_U:
		svc_slp_tsk_u(regs);
		break;
	case SVC_WUP_TSK:
		svc_wup_tsk(regs);
		break;
	case SVC_SUS_TSK:
		svc_sus_tsk(regs);
		break;
	case SVC_RSM_TSK:
		svc_rsm_tsk(regs);
		break;
	case SVC_FRSM_TSK:
		svc_frsm_tsk(regs);
		break;
	case SVC_EXT_TSK:
		svc_ext_tsk(regs);
		break;
	case SVC_TER_TSK:
		svc_ter_tsk(regs);
		break;
	case SVC_CHG_PRI:
		svc_chg_pri(regs);
		break;
	case SVC_ROT_RDQ:
		svc_rot_rdq(regs);
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
 * Kernel-space direct creation functions
 * These can be called from kernel_main() without using SVC
 */

static ID direct_cre_sem(INT isemcnt, INT maxsem)
{
	INT i;
	for (i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphore_table[i].semid == 0) {
			semaphore_table[i].semid = next_semid++;
			semaphore_table[i].semcnt = isemcnt;
			semaphore_table[i].maxsem = maxsem;
			semaphore_table[i].wait_queue = NULL;
			return semaphore_table[i].semid;
		}
	}
	return -1;
}

static ID direct_cre_mtx(void)
{
	INT i;
	for (i = 0; i < MAX_MUTEXES; i++) {
		if (mutex_table[i].mtxid == 0) {
			mutex_table[i].mtxid = next_mtxid++;
			mutex_table[i].locked = 0;
			mutex_table[i].owner = NULL;
			mutex_table[i].wait_queue = NULL;
			return mutex_table[i].mtxid;
		}
	}
	return -1;
}

static ID direct_cre_flg(UW iflgptn)
{
	INT i;
	for (i = 0; i < MAX_EVENTFLAGS; i++) {
		if (eventflag_table[i].flgid == 0) {
			eventflag_table[i].flgid = next_flgid++;
			eventflag_table[i].flgptn = iflgptn;
			eventflag_table[i].wait_queue = NULL;
			return eventflag_table[i].flgid;
		}
	}
	return -1;
}

static ID direct_cre_mbf(UW bufsz, UW maxmsz)
{
	INT i;
	for (i = 0; i < MAX_MSGBUFFERS; i++) {
		if (msgbuffer_table[i].mbfid == 0) {
			static UW mbf_buffer[512];  /* Static buffer for demo */
			msgbuffer_table[i].mbfid = next_mbfid++;
			msgbuffer_table[i].bufsz = bufsz;
			msgbuffer_table[i].maxmsz = maxmsz;
			msgbuffer_table[i].buffer = mbf_buffer;
			msgbuffer_table[i].head = 0;
			msgbuffer_table[i].tail = 0;
			msgbuffer_table[i].freesz = bufsz;
			msgbuffer_table[i].send_queue = NULL;
			msgbuffer_table[i].recv_queue = NULL;
			return msgbuffer_table[i].mbfid;
		}
	}
	return -1;
}

static ID direct_cre_mbx(void)
{
	INT i;
	for (i = 0; i < MAX_MAILBOXES; i++) {
		if (mailbox_table[i].mbxid == 0) {
			mailbox_table[i].mbxid = next_mbxid++;
			mailbox_table[i].msg_queue = NULL;
			mailbox_table[i].recv_queue = NULL;
			return mailbox_table[i].mbxid;
		}
	}
	return -1;
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
	register UW64 delay __asm__("x0") = dlytim;
	__asm__ volatile(
		"svc %1"
		: "+r"(delay)
		: "i"(SVC_DLY_TSK)
		: "memory"
	);
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

/* Wait semaphore with timeout */
static inline ER tk_wai_sem_u(ID semid, TMO tmo)
{
	register UW64 r0 __asm__("x0") = semid;
	register UW64 r1 __asm__("x1") = tmo;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_WAI_SEM_U), "r"(r1)
		: "memory"
	);
	return (ER)r0;
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

/* Lock mutex with timeout */
static inline ER tk_loc_mtx_u(ID mtxid, TMO tmo)
{
	register UW64 r0 __asm__("x0") = mtxid;
	register UW64 r1 __asm__("x1") = tmo;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_LOC_MTX_U), "r"(r1)
		: "memory"
	);
	return (ER)r0;
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

/* Wait event flag with timeout */
static inline ER tk_wai_flg_u(ID flgid, UW waiptn, UW wfmode, TMO tmo)
{
	register UW64 r0 __asm__("x0") = flgid;
	register UW64 r1 __asm__("x1") = waiptn;
	register UW64 r2 __asm__("x2") = wfmode;
	register UW64 r3 __asm__("x3") = tmo;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_WAI_FLG_U), "r"(r1), "r"(r2), "r"(r3)
		: "memory"
	);
	return (ER)r0;
}

/* Create message buffer */
static inline ID tk_cre_mbf(UW bufsz, UW maxmsz)
{
	register UW64 ret __asm__("x0") = bufsz;
	register UW64 max __asm__("x1") = maxmsz;
	__asm__ volatile(
		"svc %2"
		: "+r"(ret)
		: "r"(max), "i"(SVC_CRE_MBF)
		: "memory"
	);
	return (ID)ret;
}

/* Send message to buffer */
static inline ER tk_snd_mbf(ID mbfid, void *msg, UW msgsz)
{
	register UW64 ret __asm__("x0") = mbfid;
	register UW64 msgptr __asm__("x1") = (UW64)msg;
	register UW64 size __asm__("x2") = msgsz;
	__asm__ volatile(
		"svc %3"
		: "+r"(ret)
		: "r"(msgptr), "r"(size), "i"(SVC_SND_MBF)
		: "memory"
	);
	return (ER)ret;
}

/* Send message to buffer with timeout */
static inline ER tk_snd_mbf_u(ID mbfid, void *msg, UW msgsz, TMO tmo)
{
	register UW64 r0 __asm__("x0") = mbfid;
	register UW64 r1 __asm__("x1") = (UW64)msg;
	register UW64 r2 __asm__("x2") = msgsz;
	register UW64 r3 __asm__("x3") = tmo;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_SND_MBF_U), "r"(r1), "r"(r2), "r"(r3)
		: "memory"
	);
	return (ER)r0;
}

/* Receive message from buffer with timeout */
static inline INT tk_rcv_mbf_u(ID mbfid, void *msg, TMO tmo)
{
	register UW64 r0 __asm__("x0") = mbfid;
	register UW64 r1 __asm__("x1") = (UW64)msg;
	register UW64 r2 __asm__("x2") = tmo;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_RCV_MBF_U), "r"(r1), "r"(r2)
		: "memory"
	);
	return (INT)r0;
}

/* Sleep task with timeout */
static inline ER tk_slp_tsk_u(TMO tmo)
{
	register UW64 r0 __asm__("x0") = tmo;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_SLP_TSK_U)
		: "memory"
	);
	return (ER)r0;
}

/* Wakeup task */
static inline ER tk_wup_tsk(ID tskid)
{
	register UW64 r0 __asm__("x0") = tskid;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_WUP_TSK)
		: "memory"
	);
	return (ER)r0;
}

/* Suspend task */
static inline ER tk_sus_tsk(ID tskid)
{
	register UW64 r0 __asm__("x0") = tskid;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_SUS_TSK)
		: "memory"
	);
	return (ER)r0;
}

/* Resume task */
static inline ER tk_rsm_tsk(ID tskid)
{
	register UW64 r0 __asm__("x0") = tskid;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_RSM_TSK)
		: "memory"
	);
	return (ER)r0;
}

/* Force resume task */
static inline ER tk_frsm_tsk(ID tskid)
{
	register UW64 r0 __asm__("x0") = tskid;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_FRSM_TSK)
		: "memory"
	);
	return (ER)r0;
}

/* Exit task (self termination) */
static inline void tk_ext_tsk(void)
{
	__asm__ volatile(
		"svc %0"
		:
		: "i"(SVC_EXT_TSK)
		: "memory"
	);
	/* Does not return */
}

/* Terminate task */
static inline ER tk_ter_tsk(ID tskid)
{
	register UW64 r0 __asm__("x0") = tskid;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_TER_TSK)
		: "memory"
	);
	return (ER)r0;
}

/* Change task priority */
static inline ER tk_chg_pri(ID tskid, INT tskpri)
{
	register UW64 r0 __asm__("x0") = tskid;
	register UW64 r1 __asm__("x1") = tskpri;
	__asm__ volatile(
		"svc %2"
		: "+r"(r0)
		: "r"(r1), "i"(SVC_CHG_PRI)
		: "memory"
	);
	return (ER)r0;
}

/* Rotate ready queue */
static inline ER tk_rot_rdq(INT tskpri)
{
	register UW64 r0 __asm__("x0") = tskpri;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_ROT_RDQ)
		: "memory"
	);
	return (ER)r0;
}

/* Receive message from buffer */
static inline INT tk_rcv_mbf(ID mbfid, void *msg)
{
	register UW64 ret __asm__("x0") = mbfid;
	register UW64 msgptr __asm__("x1") = (UW64)msg;
	__asm__ volatile(
		"svc %2"
		: "+r"(ret)
		: "r"(msgptr), "i"(SVC_RCV_MBF)
		: "memory"
	);
	return (INT)ret;
}

/* Create mailbox */
static inline ID tk_cre_mbx(void)
{
	register UW64 ret __asm__("x0");
	__asm__ volatile(
		"svc %1"
		: "=r"(ret)
		: "i"(SVC_CRE_MBX)
		: "memory"
	);
	return (ID)ret;
}

/* Send message to mailbox */
static inline ER tk_snd_mbx(ID mbxid, T_MSG *msg)
{
	register UW64 ret __asm__("x0") = mbxid;
	register UW64 msgptr __asm__("x1") = (UW64)msg;
	__asm__ volatile(
		"svc %2"
		: "+r"(ret)
		: "r"(msgptr), "i"(SVC_SND_MBX)
		: "memory"
	);
	return (ER)ret;
}

/* Receive message from mailbox */
static inline T_MSG* tk_rcv_mbx(ID mbxid)
{
	register UW64 ret __asm__("x0") = mbxid;
	__asm__ volatile(
		"svc %1"
		: "+r"(ret)
		: "i"(SVC_RCV_MBX)
		: "memory"
	);
	return (T_MSG*)ret;
}

/* Receive message from mailbox with timeout */
static inline ER tk_rcv_mbx_u(ID mbxid, TMO tmo, T_MSG **ppk_msg)
{
	register UW64 r0 __asm__("x0") = mbxid;
	register UW64 r1 __asm__("x1") = tmo;
	register UW64 r2 __asm__("x2") = (UW64)ppk_msg;
	__asm__ volatile(
		"svc %1"
		: "+r"(r0)
		: "i"(SVC_RCV_MBX_U), "r"(r1), "r"(r2)
		: "memory"
	);
	return (ER)r0;
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
 * Demo task functions - Mailbox demonstration with priority messages
 */

/* Allocate a message from the pool */
static MY_MSG* alloc_message(void)
{
	INT i;
	for (i = 0; i < MSG_POOL_SIZE; i++) {
		if (!msg_in_use[i]) {
			msg_in_use[i] = true;

			/* Zero-clear the entire message structure */
			MY_MSG *msg = &msg_storage[i];
			for (UW j = 0; j < sizeof(MY_MSG); j++) {
				((unsigned char*)msg)[j] = 0;
			}

			/* Explicitly set critical fields to ensure proper initialization */
			msg->header.next = NULL;
			msg->header.msgpri = 0;
			msg->pool_index = i;
			return msg;
		}
	}
	return NULL;  /* Pool exhausted */
}

/* Free a message back to the pool */
static void free_message(MY_MSG *msg)
{
	if (msg != NULL && msg->pool_index >= 0 && msg->pool_index < MSG_POOL_SIZE) {
		msg_in_use[msg->pool_index] = false;
	}
}

/* Global test coordination variables */
static ID task2_sem = 0;                    /* Task2制御用セマフォ */
static ID task3_sem = 0;                    /* Task3制御用セマフォ */
static volatile INT task2_run_count = 0;    /* Task2実行回数 */
static volatile INT task3_run_count = 0;    /* Task3実行回数 */
static volatile BOOL task2_done = FALSE;    /* Task2動作完了フラグ */
static volatile BOOL task3_done = FALSE;    /* Task3動作完了フラグ */

static void task1_main(INT stacd, void *exinf)
{
	ID tid, task2_id, task3_id;
	ER err;
	INT test_num = 0;
	TCB *task2_tcb = NULL;
	TCB *task3_tcb = NULL;

	(void)stacd;
	(void)exinf;

	/* Get task ID using system call */
	tid = tk_get_tid();
	uart_puts("\n");
	uart_puts("========================================\n");
	uart_puts("  Task Priority Change Test Suite\n");
	uart_puts("========================================\n\n");

	uart_puts("[Task1] Starting tests...\n");
	uart_puts("[Task1] Task ID: ");
	uart_puthex((UW)tid);
	uart_puts(", Initial Priority: 10\n\n");

	/* Wait for all tasks to start and enter semaphore wait */
	/* タイマー間隔100ms × 3回 = 300ms待機して、Task2とTask3が確実にセマフォ待ちに入る */
	tk_dly_tsk(300);

	/* Get Task2 and Task3 IDs */
	task2_id = tid + 1;
	task3_id = tid + 2;

	/* Get TCB pointers for state/priority checking */
	for (INT i = 0; i < MAX_TASKS; i++) {
		if (task_table[i].tskid == task2_id) {
			task2_tcb = &task_table[i];
		}
		if (task_table[i].tskid == task3_id) {
			task3_tcb = &task_table[i];
		}
	}

	/* ===== Test 1: Basic priority change ===== */
	test_num++;
	uart_puts("[Test ");
	uart_puthex(test_num);
	uart_puts("] tk_chg_pri - Basic priority change\n");
	uart_puts("  Initial: Task1(10), Task2(20), Task3(15)\n");
	uart_puts("  Change Task2 priority: 20 -> 5 (higher than Task1)\n\n");

	/* Step 1: 現在の優先度を確認 */
	if (task2_tcb) {
		uart_puts("  Task2 current priority: ");
		uart_puthex(task2_tcb->priority);
		uart_puts("\n");
	}

	/* Step 2: 優先度を変更 */
	err = tk_chg_pri(task2_id, 5);
	uart_puts("  tk_chg_pri(task2, 5) result: ");
	uart_puthex(err);
	if (err == E_OK) {
		uart_puts(" (E_OK)\n");
	} else {
		uart_puts(" ✗ FAIL\n\n");
		goto test2;
	}

	/* Step 3: 優先度が変更されたことを確認 */
	if (task2_tcb && task2_tcb->priority == 5) {
		uart_puts("  Task2 new priority: ");
		uart_puthex(task2_tcb->priority);
		uart_puts(" ✓ Priority changed\n");
	} else {
		uart_puts("  ✗ FAIL - Priority not changed\n\n");
		goto test2;
	}

	/* Step 4: Task2を起動（セマフォシグナル） */
	task2_run_count = 0;
	task2_done = FALSE;
	err = tk_sig_sem(task2_sem);
	if (err != E_OK) {
		uart_puts("  ✗ FAIL - tk_sig_sem failed\n\n");
		goto test2;
	}

	/* Step 5: Task2が実行されるまで待つ */
	/* Task2は優先度5なのでTask1(10)より優先される */
	tk_dly_tsk(100);

	/* Step 6: Task2が実行されたことを確認 */
	uart_puts("  Task2 run count: ");
	uart_puthex(task2_run_count);
	uart_puts(", done flag: ");
	uart_puthex(task2_done ? 1 : 0);
	uart_puts("\n");

	if (task2_done && task2_run_count > 0) {
		uart_puts("  ✓ PASS - Task2 ran with higher priority\n\n");
	} else {
		uart_puts("  ✗ FAIL - Task2 did not run\n\n");
	}

	/* Step 7: Task2の優先度を元に戻す */
	tk_chg_pri(task2_id, 20);

test2:
	/* ===== Test 2: Change current task priority (tskid=0) ===== */
	test_num++;
	uart_puts("[Test ");
	uart_puthex(test_num);
	uart_puts("] tk_chg_pri - Change current task priority (tskid=0)\n");
	uart_puts("  Task1 changes its own priority: 10 -> 30\n\n");

	err = tk_chg_pri(0, 30);
	uart_puts("  tk_chg_pri(0, 30) result: ");
	uart_puthex(err);
	if (err == E_OK) {
		uart_puts(" (E_OK)\n");
		TCB *my_tcb = NULL;
		for (INT i = 0; i < MAX_TASKS; i++) {
			if (task_table[i].tskid == tid) {
				my_tcb = &task_table[i];
				break;
			}
		}
		if (my_tcb && my_tcb->priority == 30) {
			uart_puts("  Task1 new priority: ");
			uart_puthex(my_tcb->priority);
			uart_puts(" ✓ PASS\n\n");
		} else {
			uart_puts("  ✗ FAIL - Priority not changed\n\n");
		}
	} else {
		uart_puts(" ✗ FAIL\n\n");
	}

	/* Restore Task1 priority */
	tk_chg_pri(0, 10);

	/* ===== Test 3: Priority ordering verification ===== */
	test_num++;
	uart_puts("[Test ");
	uart_puthex(test_num);
	uart_puts("] Priority ordering test\n");
	uart_puts("  Task1(10) < Task3(15) < Task2(20)\n");
	uart_puts("  Lower number = higher priority\n\n");

	/* リセット */
	task2_run_count = 0;
	task3_run_count = 0;
	task2_done = FALSE;
	task3_done = FALSE;

	/* Task3とTask2の状態を確認 */
	uart_puts("  Task2 state before signal: ");
	uart_puthex(task2_tcb ? task2_tcb->state : 0xFF);
	uart_puts(", Task3 state: ");
	uart_puthex(task3_tcb ? task3_tcb->state : 0xFF);
	uart_puts("\n");

	/* Task3とTask2を起動 */
	uart_puts("  Signaling Task3 (priority 15)...\n");
	err = tk_sig_sem(task3_sem);
	uart_puts("  tk_sig_sem(task3_sem) result: ");
	uart_puthex(err);
	uart_puts("\n");
	tk_dly_tsk(150);  /* Task3実行を待つ（タイマー割り込み待ち） */

	uart_puts("  Signaling Task2 (priority 20)...\n");
	err = tk_sig_sem(task2_sem);
	uart_puts("  tk_sig_sem(task2_sem) result: ");
	uart_puthex(err);
	uart_puts("\n");
	tk_dly_tsk(150);  /* Task2実行を待つ（タイマー割り込み待ち） */

	uart_puts("  Task2 (pri=20) run count: ");
	uart_puthex(task2_run_count);
	uart_puts(", done: ");
	uart_puthex(task2_done ? 1 : 0);
	uart_puts("\n");

	uart_puts("  Task3 (pri=15) run count: ");
	uart_puthex(task3_run_count);
	uart_puts(", done: ");
	uart_puthex(task3_done ? 1 : 0);
	uart_puts("\n");

	if (task2_run_count > 0 && task3_run_count > 0) {
		uart_puts("  ✓ PASS - All tasks ran according to priority\n\n");
	} else {
		uart_puts("  ✗ FAIL - Task execution issue\n\n");
	}

	/* ===== Test 4: tk_rot_rdq ===== */
	test_num++;
	uart_puts("[Test ");
	uart_puthex(test_num);
	uart_puts("] tk_rot_rdq - Rotate ready queue\n");
	uart_puts("  Rotate queue at priority 0 (current priority)\n\n");

	err = tk_rot_rdq(0);
	uart_puts("  tk_rot_rdq(0) result: ");
	uart_puthex(err);
	if (err == E_OK) {
		uart_puts(" (E_OK) ✓ PASS\n\n");
	} else {
		uart_puts(" ✗ FAIL\n\n");
	}

	/* ===== Test 5: Error - Non-existent task ===== */
	test_num++;
	uart_puts("[Test ");
	uart_puthex(test_num);
	uart_puts("] tk_chg_pri - Non-existent task\n");
	uart_puts("  Expected: E_NOEXS\n\n");

	err = tk_chg_pri(999, 10);
	uart_puts("  tk_chg_pri(999, 10) result: ");
	uart_puthex(err);
	if (err == E_NOEXS) {
		uart_puts(" (E_NOEXS) ✓ PASS\n\n");
	} else {
		uart_puts(" ✗ FAIL\n\n");
	}

	/* ===== Test 6: Error - Invalid priority (0) ===== */
	test_num++;
	uart_puts("[Test ");
	uart_puthex(test_num);
	uart_puts("] tk_chg_pri - Invalid priority (0)\n");
	uart_puts("  Expected: E_PAR\n\n");

	err = tk_chg_pri(task2_id, 0);
	uart_puts("  tk_chg_pri(task2, 0) result: ");
	uart_puthex(err);
	if (err == E_PAR) {
		uart_puts(" (E_PAR) ✓ PASS\n\n");
	} else {
		uart_puts(" ✗ FAIL\n\n");
	}

	/* ===== Test 7: Error - Invalid priority (>140) ===== */
	test_num++;
	uart_puts("[Test ");
	uart_puthex(test_num);
	uart_puts("] tk_chg_pri - Invalid priority (141)\n");
	uart_puts("  Expected: E_PAR\n\n");

	err = tk_chg_pri(task2_id, 141);
	uart_puts("  tk_chg_pri(task2, 141) result: ");
	uart_puthex(err);
	if (err == E_PAR) {
		uart_puts(" (E_PAR) ✓ PASS\n\n");
	} else {
		uart_puts(" ✗ FAIL\n\n");
	}

	/* ===== Test 8: Error - DORMANT task ===== */
	test_num++;
	uart_puts("[Test ");
	uart_puthex(test_num);
	uart_puts("] tk_chg_pri - DORMANT task\n");
	uart_puts("  Expected: E_OBJ\n\n");

	/* Terminate Task2 first */
	tk_ter_tsk(task2_id);
	tk_dly_tsk(50);

	err = tk_chg_pri(task2_id, 15);
	uart_puts("  tk_chg_pri(dormant_task, 15) result: ");
	uart_puthex(err);
	if (err == E_OBJ) {
		uart_puts(" (E_OBJ) ✓ PASS\n\n");
	} else {
		uart_puts(" ✗ FAIL\n\n");
	}

	uart_puts("========================================\n");
	uart_puts("  All Tests Complete!\n");
	uart_puts("========================================\n\n");

	/* Done testing */
	while (1) {
		for (volatile int i = 0; i < 10000000; i++);
	}
}

static void task2_main(INT stacd, void *exinf)
{
	ER err;

	(void)stacd;
	(void)exinf;

	/* セマフォ制御によるテストヘルパータスク */
	while (1) {
		/* Task1からの起動信号を待つ（無限待ち） */
		err = tk_wai_sem_u(task2_sem, TMO_FEVR);
		if (err != E_OK) {
			uart_puts("[Task2] ERROR: tk_wai_sem_u failed\n");
			continue;
		}

		/* 動作実行 */
		task2_run_count++;
		task2_done = TRUE;
	}
}

static void task3_main(INT stacd, void *exinf)
{
	ER err;

	(void)stacd;
	(void)exinf;

	/* セマフォ制御によるテストヘルパータスク */
	while (1) {
		/* Task1からの起動信号を待つ（無限待ち） */
		err = tk_wai_sem_u(task3_sem, TMO_FEVR);
		if (err != E_OK) {
			uart_puts("[Task3] ERROR: tk_wai_sem_u failed\n");
			continue;
		}

		/* 動作実行 */
		task3_run_count++;
		task3_done = TRUE;
	}
}

static void idle_task_main(INT stacd, void *exinf)
{
	static volatile INT idle_count = 0;

	(void)stacd;
	(void)exinf;

	while (1) {
		/* Only print occasionally to reduce output spam */
		if (++idle_count >= 10000) {
			idle_count = 0;
			/* Idle task is running - system is waiting for events */
		}

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

	/* Initialize semaphores */
	uart_puts("Initializing semaphores...\n");
	for (INT i = 0; i < MAX_SEMAPHORES; i++) {
		semaphore_table[i].semid = 0;
	}
	uart_puts("Semaphore table initialized.\n");

	/* Create demo semaphore (initial count=1, max=10) - for timeout test */
	uart_puts("Creating demo semaphore...\n");
	demo_sem = direct_cre_sem(1, 10);
	uart_puts("Demo semaphore created: ID=");
	uart_puthex((UW)demo_sem);
	uart_puts("\n");

	/* Create test control semaphores (initial count=0) */
	uart_puts("Creating test control semaphores...\n");
	task2_sem = direct_cre_sem(0, 10);  /* Task2制御用 */
	task3_sem = direct_cre_sem(0, 10);  /* Task3制御用 */

	if (task2_sem <= 0 || task3_sem <= 0) {
		uart_puts("ERROR: Failed to create test semaphores\n");
		for (;;);
	}

	uart_puts("Test semaphores created:\n");
	uart_puts("  task2_sem: ");
	uart_puthex((UW)task2_sem);
	uart_puts("\n");
	uart_puts("  task3_sem: ");
	uart_puthex((UW)task3_sem);
	uart_puts("\n\n");

	/* Initialize mutexes */
	uart_puts("Initializing mutexes...\n");
	for (INT i = 0; i < MAX_MUTEXES; i++) {
		mutex_table[i].mtxid = 0;
	}

	/* Create demo mutex */
	demo_mtx = direct_cre_mtx();
	uart_puts("Demo mutex created: ID=");
	uart_puthex((UW)demo_mtx);
	uart_puts("\n");

	/* Initialize event flags */
	uart_puts("Initializing event flags...\n");
	for (INT i = 0; i < MAX_EVENTFLAGS; i++) {
		eventflag_table[i].flgid = 0;
	}

	/* Create demo event flag (initial pattern = 0) */
	demo_flg = direct_cre_flg(0);
	uart_puts("Demo event flag created: ID=");
	uart_puthex((UW)demo_flg);
	uart_puts("\n");

	/* Initialize message buffers */
	uart_puts("Initializing message buffers...\n");
	for (INT i = 0; i < MAX_MSGBUFFERS; i++) {
		msgbuffer_table[i].mbfid = 0;
	}

	/* Create demo message buffer (128 bytes, max message 64 bytes) */
	demo_mbf = direct_cre_mbf(128, 64);
	uart_puts("Demo message buffer created: ID=");
	uart_puthex((UW)demo_mbf);
	uart_puts("\n");

	/* Initialize mailboxes */
	uart_puts("Initializing mailboxes...\n");
	for (INT i = 0; i < MAX_MAILBOXES; i++) {
		mailbox_table[i].mbxid = 0;
		mailbox_table[i].msg_queue = NULL;
		mailbox_table[i].recv_queue = NULL;
	}

	/* Create demo mailbox */
	demo_mbx_comm = direct_cre_mbx();
	uart_puts("Demo mailbox created: ID=");
	uart_puthex((UW)demo_mbx_comm);
	uart_puts("\n\n");

	/* Initialize message pool */
	uart_puts("Initializing message pool...\n");

	/* Zero-clear entire message storage for clean debugging */
	for (UW i = 0; i < sizeof(msg_storage); i++) {
		((unsigned char*)msg_storage)[i] = 0;
	}

	/* Initialize allocation tracking */
	for (INT i = 0; i < MSG_POOL_SIZE; i++) {
		msg_in_use[i] = false;
	}

	uart_puts("Message pool initialized: ");
	uart_puthex(MSG_POOL_SIZE);
	uart_puts(" messages\n\n");

	/* Initialize tasks */
	uart_puts("Creating tasks...\n");
	create_task(0, 1, task1_main, task1_stack, TASK_STACK_SIZE, 10);   /* Task1: priority 10 */
	create_task(1, 2, task2_main, task2_stack, TASK_STACK_SIZE, 20);   /* Task2: priority 20 */
	create_task(2, 3, task3_main, task3_stack, TASK_STACK_SIZE, 15);   /* Task3: priority 15 */
	create_task(3, 4, idle_task_main, idle_stack, TASK_STACK_SIZE, 140); /* Idle: lowest priority */

	uart_puts("Starting tasks...\n");
	start_task(0);
	start_task(1);
	start_task(2);
	start_task(3);

	uart_puts("Tasks created and started.\n");
	uart_puts("Task switching interval: ");
	uart_puthex(task_switch_interval);
	uart_puts(" ms\n");
	uart_puts("\n");

	/* Enable IRQ interrupts now that initialization is complete */
	uart_puts("Enabling IRQ interrupts...\n");
	__asm__ volatile("msr daifclr, #2");  // Clear IRQ mask (bit 1)
	uart_puts("IRQ enabled.\n");
	uart_puts("\n");

	/* Start timer interrupts (after IRQs are enabled to avoid pending interrupt flood) */
	uart_puts("Starting timer interrupts (1ms period)...\n");
	start_hw_timer();
	uart_puts("Timer started.\n");
	uart_puts("\n");

	/* Start multitasking by dispatching to first task */
	uart_puts("Starting multitasking...\n");
	uart_puts("\n");

	/* Run scheduler to select highest priority task */
	schedule();

	/* Debug: Check if schedtsk is set */
	if (schedtsk == NULL) {
		uart_puts("ERROR: schedtsk is NULL!\n");
		for (;;);
	}
	TCB *sched = (TCB *)schedtsk;
	uart_puts("Dispatching to task ID: ");
	uart_puthex((UW)sched->tskid);
	uart_puts(", priority: ");
	uart_puthex((UW)sched->priority);
	uart_puts("\n\n");

	/* This will switch to the first ready task and never return */
	dispatch_to_schedtsk();

	/* Should never reach here */
	uart_puts("ERROR: Returned from dispatcher!\n");
	for (;;) {
		__asm__ volatile("wfi");
	}
}

/*
 * Check for timed-out tasks
 * Called from timer interrupt handler
 */
static void check_timeouts(void)
{
	INT i;
	TCB *task;
	TCB **queue_ptr;
	SEMCB *sem;
	MTXCB *mtx;
	MBXCB *mbx;
	FLGCB *flg;
	MBFCB *mbf;
	void *obj_start, *obj_end;

	/* Check all tasks for timeout */
	for (i = 0; i < MAX_TASKS; i++) {
		task = &task_table[i];

		/* Skip if task not waiting or no timeout set */
		if (task->state != TS_WAIT || task->wait_timeout == 0) {
			continue;
		}

		/* Check if timeout has occurred */
		if ((SYSTIM)timer_tick_count >= task->wait_timeout) {
			/* Timeout occurred */

			if (task->wait_obj != NULL) {
				/* Waiting on an object (semaphore, mutex, mailbox, etc.) - remove from wait queue */

				/* Determine object type by checking which table it belongs to */
				obj_start = (void *)semaphore_table;
				obj_end = (void *)(&semaphore_table[MAX_SEMAPHORES]);

				if (task->wait_obj >= obj_start && task->wait_obj < obj_end) {
					/* Semaphore wait */
					sem = (SEMCB *)task->wait_obj;

					/* Remove from semaphore wait queue */
					if (sem->wait_queue == task) {
						sem->wait_queue = task->wait_next;
					} else {
						queue_ptr = &sem->wait_queue;
						while (*queue_ptr != NULL) {
							if ((*queue_ptr)->wait_next == task) {
								(*queue_ptr)->wait_next = task->wait_next;
								break;
							}
							queue_ptr = &((*queue_ptr)->wait_next);
						}
					}
				} else {
					/* Check if it's a mutex */
					obj_start = (void *)mutex_table;
					obj_end = (void *)(&mutex_table[MAX_MUTEXES]);

					if (task->wait_obj >= obj_start && task->wait_obj < obj_end) {
						/* Mutex wait */
						mtx = (MTXCB *)task->wait_obj;

						/* Remove from mutex wait queue */
						if (mtx->wait_queue == task) {
							mtx->wait_queue = task->wait_next;
						} else {
							queue_ptr = &mtx->wait_queue;
							while (*queue_ptr != NULL) {
								if ((*queue_ptr)->wait_next == task) {
									(*queue_ptr)->wait_next = task->wait_next;
									break;
								}
								queue_ptr = &((*queue_ptr)->wait_next);
							}
						}
					} else {
						/* Check if it's a mailbox */
						obj_start = (void *)mailbox_table;
						obj_end = (void *)(&mailbox_table[MAX_MAILBOXES]);

						if (task->wait_obj >= obj_start && task->wait_obj < obj_end) {
							/* Mailbox receive wait */
							mbx = (MBXCB *)task->wait_obj;

							/* Remove from mailbox recv queue */
							if (mbx->recv_queue == task) {
								mbx->recv_queue = task->wait_next;
							} else {
								queue_ptr = &mbx->recv_queue;
								while (*queue_ptr != NULL) {
									if ((*queue_ptr)->wait_next == task) {
										(*queue_ptr)->wait_next = task->wait_next;
										break;
									}
									queue_ptr = &((*queue_ptr)->wait_next);
								}
							}
						} else {
							/* Check if it's an event flag */
							obj_start = (void *)eventflag_table;
							obj_end = (void *)(&eventflag_table[MAX_EVENTFLAGS]);

							if (task->wait_obj >= obj_start && task->wait_obj < obj_end) {
								/* Event flag wait */
								flg = (FLGCB *)task->wait_obj;

								/* Remove from event flag wait queue */
								if (flg->wait_queue == task) {
									flg->wait_queue = task->wait_next;
								} else {
									queue_ptr = &flg->wait_queue;
									while (*queue_ptr != NULL) {
										if ((*queue_ptr)->wait_next == task) {
											(*queue_ptr)->wait_next = task->wait_next;
											break;
										}
										queue_ptr = &((*queue_ptr)->wait_next);
									}
								}
							} else {
								/* Could be a message buffer send or receive wait. For message buffer,
								 * wait_obj contains the message/buffer pointer, not the MBFCB.
								 * So we search all message buffers for this task. */
								INT j;
								int found = 0;
								for (j = 0; j < MAX_MSGBUFFERS; j++) {
									mbf = &msgbuffer_table[j];
									if (mbf->mbfid == 0) continue;

									/* Check if task is in this buffer's send queue */
									if (mbf->send_queue == task) {
										mbf->send_queue = task->wait_next;
										found = 1;
										break;
									} else {
										TCB *prev = mbf->send_queue;
										while (prev != NULL && prev->wait_next != NULL) {
											if (prev->wait_next == task) {
												prev->wait_next = task->wait_next;
												found = 1;
												break;
											}
											prev = prev->wait_next;
										}
										if (found) break;
									}

									/* Check if task is in this buffer's recv queue */
									if (mbf->recv_queue == task) {
										mbf->recv_queue = task->wait_next;
										found = 1;
										break;
									} else {
										TCB *prev = mbf->recv_queue;
										while (prev != NULL && prev->wait_next != NULL) {
											if (prev->wait_next == task) {
												prev->wait_next = task->wait_next;
												found = 1;
												break;
											}
											prev = prev->wait_next;
										}
										if (found) break;
									}
								}
								/* If not semaphore, mutex, mailbox, event flag, or message buffer, could be other objects - add handling as needed */
							}
						}
					}
				}

				/* Set return value to E_TMOUT (timeout error) */
				if (task->wait_regs != NULL) {
					((SVC_REGS *)task->wait_regs)->x0 = E_TMOUT;
				}
			} else {
				/* wait_obj is NULL - either task delay or task sleep */
				if (task->wait_flgptn == 1) {
					/* Task sleep (tk_slp_tsk_u) - timeout is an error */
					if (task->wait_regs != NULL) {
						((SVC_REGS *)task->wait_regs)->x0 = E_TMOUT;
					}
				} else {
					/* Task delay (tk_dly_tsk) - timeout is normal completion */
					if (task->wait_regs != NULL) {
						((SVC_REGS *)task->wait_regs)->x0 = E_OK;
					}
				}
			}

			/* Make task ready */
			task->state = TS_READY;
			task->wait_timeout = 0;
			task->wait_obj = NULL;
			task->wait_next = NULL;
			task->wait_flgptn = 0;
			task->wait_regs = NULL;

			/* Task may now be runnable - scheduler will handle it */
		}
	}
}

/*
 * Timer interrupt handler
 */
void timer_handler(void)
{
	/* Increment tick counter */
	timer_tick_count++;

	/* Check for timed-out tasks */
	check_timeouts();

	/* Always schedule after checking timeouts to ensure tasks made READY
	 * by timeout expiration run immediately */
	schedule();

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
