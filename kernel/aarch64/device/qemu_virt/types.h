/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package - Minimal Types
 *
 *    Modified for AArch64 QEMU virt (ReTron OS) at 2024.
 *
 *----------------------------------------------------------------------
 */

/*
 *	types.h (AArch64)
 *	Minimal type definitions for standalone build
 */

#ifndef _TYPES_H_
#define _TYPES_H_

/* Basic types */
typedef signed char		B;
typedef signed short		H;
typedef signed int		W;
typedef signed long long	D;
typedef unsigned char		UB;
typedef unsigned short		UH;
typedef unsigned int		UW;
typedef unsigned long long	UD;

typedef signed char		VB;
typedef signed short		VH;
typedef signed int		VW;
typedef signed long long	VD;

typedef volatile B		_B;
typedef volatile H		_H;
typedef volatile W		_W;
typedef volatile D		_D;
typedef volatile UB		_UB;
typedef volatile UH		_UH;
typedef volatile UW		_UW;
typedef volatile UD		_UD;

typedef int			INT;
typedef unsigned int		UINT;

typedef int			BOOL;
#define TRUE			1
#define FALSE			0

typedef void			(*FP)(void);
typedef void			*VP;

/* T-Kernel types */
typedef W			ER;		/* Error code */
typedef INT			ID;		/* Object ID */
typedef UINT			ATR;		/* Object attribute */
typedef D			SYSTIM;		/* System time (64-bit, milliseconds) */
typedef W			TMO;		/* Timeout value (milliseconds) */

/* Timeout special values */
#define TMO_POL			0		/* Polling (no wait) */
#define TMO_FEVR		(-1)		/* Wait forever */

/* Error codes */
#define E_OK			0		/* Normal completion */
#define E_SYS			(-5)		/* System error */
#define E_NOSPT			(-9)		/* Unsupported function */
#define E_RSFN			(-10)		/* Reserved function code */
#define E_TMOUT			(-11)		/* Timeout */
#define E_PAR			(-17)		/* Parameter error */
#define E_OBJ			(-41)		/* Invalid object state */
#define E_NOEXS			(-42)		/* Object does not exist */
#define E_OACV			(-57)		/* Object access violation */

/* NULL pointer */
#ifndef NULL
#define NULL			((void *)0)
#endif

/* Export/Local */
#define EXPORT
#define LOCAL			static

/* Inline assembly */
#define Asm			__asm__ __volatile__

/* Inline function */
#define Inline			static inline

#endif /* _TYPES_H_ */
