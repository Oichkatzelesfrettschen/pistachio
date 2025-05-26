/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * Copyright (c) 1992 Helsinki University of Technology
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND HELSINKI UNIVERSITY OF TECHNOLOGY ALLOW FREE USE
 * OF THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * HELSINKI UNIVERSITY OF TECHNOLOGY DISCLAIM ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * 11-May-92  Tero Kivinen (kivinen) at Helsinki University of Technology
 * 	Created.
 * 
 * $Log:$
 */
/*
 * 	File: threads/ns532/lock.s
 *	Author: Tero Kivinen, Helsinki University of Technology 1992.
 *
 *	Locking primitives for the 32532.
 */

/*
 * To make these routines faster:
 * - remove frames
 * - don't use interlocked set bit on uniprocessor
 * - inline
 */

#include <ns532/asm.h>

/*
 * boolean_t spin_try_lock(int *m)
 */
ENTRY(spin_try_lock)
	FRAME
	movd	B_ARG0,r0
	sbitib	0,0(r0) /* This needs checking in trap.c for sbiti. 
			   sbitb works only for uniprocessors, 
			   (see cpu bug list for sbitib) */
	bfs	spin_try_lock_fail
	movqd	1,r0
	EMARF
	ret	0
spin_try_lock_fail:
	movqd	0,r0
	EMARF
	ret	0

/*
 * void spin_unlock(int *m)
 */
ENTRY(spin_unlock)
	FRAME
	movd	B_ARG0,r0
	movd	0,0(r0)
	EMARF
	ret	0
