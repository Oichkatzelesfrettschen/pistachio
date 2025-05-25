/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created from i386 BSDSS.
 *
 * $Log:	param.h,v $
 */

#ifndef _NS532_PARAM_H_
#define _NS532_PARAM_H_

/*
 * Machine dependent constants for the 32532
 */
/*
 * Base address for U*X system call emulator.
 */
#define	EMULATOR_BASE	0x13000000
#define	EMULATOR_END	0x13040000
				/* 256 Kbytes */

/*
 * Define base for user mapped files
 */

#define MAP_FILE_BASE	0x11000000

/*
 * stack sits above emulator.
 */
#define	EMULATOR_ABOVE_STACK	0

#define USRTEXT		0x10000

#define	USRSTACK	0x18000000
#define TRAMPOLINE_MAX_SIZE	0x100

/*
 * Virtual memory related constants, all in bytes
 */
#ifndef	MAXTSIZ
#define MAXTSIZ		(6*1024*1024)		/* max text size */
#endif
#ifndef	DFLDSIZ
#define DFLDSIZ		(6*1024*1024)		/* initial data size limit */
#endif
#ifndef	MAXDSIZ
#define MAXDSIZ		(16*1024*1024)		/* max data size */
#endif
#ifndef	DFLSSIZ
#define DFLSSIZ		(512*1024)		/* initial stack size limit */
#endif
#ifndef	MAXSSIZ
#define MAXSSIZ		MAXDSIZ			/* max stack size */
#endif

#define	MAXPHYS		(64 * 1024)	/* max raw I/O transfer size */
#define	CLSIZE		1
#define	CLSIZELOG2	0

#define	NBPG		4096		/* bytes/page */
#define	PGSHIFT		12		/* LOG2(NBPG) */

#define	MSIZE		128		/* size of an mbuf */
#define	MCLBYTES	1024
#define	MCLSHIFT	10
#define	MCLOFSET	(MCLBYTES - 1)
#define UPAGES		2

#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */

#ifndef NMBCLUSTERS
#ifdef GATEWAY
#define	NMBCLUSTERS	512		/* map size, max cluster allocation */
#else
#define	NMBCLUSTERS	256		/* map size, max cluster allocation */
#endif
#endif

/*
 * Some macros for units conversion
 */
/* Core clicks (4096 bytes) to segments and vice versa */
#define	ctos(x)	(x)
#define	stoc(x)	(x)

/* Core clicks (4096 bytes) to disk blocks */
#define	ctod(x)	((x)<<(PGSHIFT-DEV_BSHIFT))
#define	dtoc(x)	((x)>>(PGSHIFT-DEV_BSHIFT))
#define	dtob(x)	((x)<<DEV_BSHIFT)

/* clicks to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to clicks */
#define	btoc(x)	(((unsigned)(x)+(NBPG-1))>>PGSHIFT)

#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned)(db) << DEV_BSHIFT)

#include <mach/ns532/vm_param.h>

#endif /* _NS532_PARAM_H_ */
