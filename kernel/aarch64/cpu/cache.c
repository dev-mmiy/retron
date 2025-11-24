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
 *	cache.c (AArch64)
 *	Cache Operation
 */

#include <basic.h>
#include <tk/tkernel.h>
#include <tk/sysdef.h>
#include "cpu_insn.h"

#define	CacheLineSZ	64	/* Typical AArch64 cache line size */

/*
 * Obtain cache line size
 */
EXPORT INT GetCacheLineSize( void )
{
	return CacheLineSZ;
}

/*
 * Flush cache
 *      Flush cache for an area that starts at laddr for len bytes.
 *      cache is written back and invalidated.
 *
 *      mode := [TCM_ICACHE] | [TCM_DCACHE]
 */
EXPORT void FlushCacheM( CONST void *laddr, INT len, UINT mode )
{
	CONST VB	*p, *ep;

	ep = (VB*)laddr + len;

	if ( (mode & TCM_DCACHE) != 0 ) {
		p = (VB*)((UW)laddr & ~(CacheLineSZ - 1));
		while ( p < ep ) {
			/* Clean and Invalidate data cache by VA to PoC */
			Asm("dc civac, %0":: "r"(p));
			p += CacheLineSZ;
		}
	}
	if ( (mode & TCM_ICACHE) != 0 ) {
		p = (VB*)((UW)laddr & ~(CacheLineSZ - 1));
		while ( p < ep ) {
			/* Invalidate instruction cache by VA to PoU */
			Asm("ic ivau, %0":: "r"(p));
			p += CacheLineSZ;
		}
	}
	DSB();
	ISB();
}

EXPORT void FlushCache( CONST void *laddr, INT len )
{
	FlushCacheM(laddr, len, TCM_ICACHE|TCM_DCACHE);
}

/*
 * Control cache
 *	mode := [CC_FLUSH] | [CC_INVALIDATE]
 */
EXPORT ER ControlCacheM( void *laddr, INT len, UINT mode )
{
	VB	*p, *ep;

	if ( (mode & ~(CC_FLUSH|CC_INVALIDATE)) != 0 ) return E_PAR;

	ep = (VB*)laddr + len;

	p = (VB*)((UW)laddr & ~(CacheLineSZ - 1));
	while ( p < ep ) {
		switch ( mode ) {
		  case CC_FLUSH:
			/* Clean data cache by VA to PoC */
			Asm("dc cvac, %0":: "r"(p));
			break;
		  case CC_INVALIDATE:
			/* Invalidate data cache by VA to PoC */
			Asm("dc ivac, %0":: "r"(p));
			break;
		  default:
			/* Clean and Invalidate data cache by VA to PoC */
			Asm("dc civac, %0":: "r"(p));
		}

		/* Invalidate instruction cache by VA to PoU */
		Asm("ic ivau, %0":: "r"(p));

		p += CacheLineSZ;
	}
	DSB();
	ISB();

	return E_OK;
}

/*
 * Invalidate entire instruction cache
 */
EXPORT void InvalidateICache( void )
{
	Asm("ic iallu":::);
	DSB();
	ISB();
}

/*
 * Clean entire data cache
 */
EXPORT void CleanDCache( void )
{
	/* Clean all data cache by set/way - implementation specific */
	/* For simplicity, we use the entire cache clean instruction if available */
	/* Note: This is a simplified implementation */
	DSB();
}

/*
 * Clean and invalidate entire data cache
 */
EXPORT void CleanInvalidateDCache( void )
{
	/* Clean and invalidate all data cache by set/way */
	/* Note: This is a simplified implementation */
	DSB();
	ISB();
}
