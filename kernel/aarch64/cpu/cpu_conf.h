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
 *	cpu_conf.h (AArch64)
 *	CPU-Dependent OS Configuration Information
 */

#ifndef _CPU_CONF_
#define _CPU_CONF_

/*
 * Define 1 when using MMU
 *   0: not using MMU
 *   1: using MMU
 */
#define USE_MMU			0

/*
 * Definition of minimum system stack size
 *	Minimum system stack size when setting the system stack size
 *	per task by 'tk_cre_tsk().'
 *	AArch64 requires larger stack due to 64-bit registers
 */
#define MIN_SYS_STACK_SIZE	2048

/*
 * Maximum logical space ID
 */
#define MAX_LSID		255

/*
 * AArch64 specific configurations
 */

/* Number of exception levels (EL0-EL3) */
#define NUM_EL			4

/* Current exception level for kernel (typically EL1) */
#define KERNEL_EL		1

/* Stack alignment requirement (16 bytes for AArch64) */
#define STACK_ALIGN		16

/* Cache line size (typically 64 bytes for Cortex-A53) */
#define CACHE_LINE_SIZE		64

#endif /* _CPU_CONF_ */
