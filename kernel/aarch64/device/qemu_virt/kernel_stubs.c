/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Modified for AArch64 QEMU virt (ReTron OS) at 2024.
 *
 *----------------------------------------------------------------------
 */

/*
 *	kernel_stubs.c (AArch64)
 *	Stub definitions for standalone test kernel
 */

#include "types.h"

/*
 * Kernel global variables (stubs)
 */

/* Current task control block pointer */
void *ctxtsk = (void *)0;

/* Next scheduled task */
void *schedtsk = (void *)0;

/* Low power mode disable count */
UW lowpow_discnt = 1;  /* Disable low power mode */

/* Note: dispatch_disabled is defined in cpu_support.S */

/*
 * Timer handler stub
 */
void timer_handler(void)
{
    /* Timer handling would go here */
    /* For now, just return */
}

/*
 * Low power mode stub
 */
void low_pow(void)
{
    /* Enter WFI (Wait For Interrupt) */
    __asm__ volatile("wfi");
}
