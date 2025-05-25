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
 * $Log:	import_mach.h,v $
 * Revision 2.2  92/04/22  14:01:13  rwd
 * 	Undef DEBUG here since cthreads.h uses DEBUG.  This should go
 * 	away when cthreads.h is fixed.
 * 	[92/04/22            rwd]
 * 
 * Revision 2.1  92/04/21  17:10:57  rwd
 * BSDSS
 * 
 *
 */

/*
 * MACH interface definitions and data for UX out-of-kernel kernel.
 */

/*
 * <mach/mach.h> must be included with 'KERNEL' off
 */
#ifdef	KERNEL
#define	KERNEL__
#undef	KERNEL
#endif	KERNEL

#ifdef DEBUG
#define DEBUG__ DEBUG
#undef DEBUG
#endif

#include <mach.h>
#include <mach/message.h>
#include <mach/notify.h>
#include <mach/mig_errors.h>
#include <cthreads.h>

#ifdef DEBUG__
#define DEBUG DEBUG__
#undef DEBUG__
#endif

#ifdef	KERNEL__
#undef	KERNEL__
#define	KERNEL	1
#endif	KERNEL__
