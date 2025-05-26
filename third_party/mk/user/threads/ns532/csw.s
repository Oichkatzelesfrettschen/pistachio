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
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log:$
 */
/*
 * 	File: threads/ns532/csw.s
 *	Author: Jukka Virtanen, Helsinki University of Technology 1992.
 * 
 *	CThreads user level context switch routines.
 */

#include <ns532/asm.h>

/*
 * Suspend the current thread and resume the next one.
 *
 * void cproc_switch(int *cur, int *next, int *lock)
 */
/*
 * XXXX What about floats? Do we need to cswitch them too?
 */
ENTRY(cproc_switch)
	enter	[r3,r4,r5,r6,r7],0
	movd	B_ARG0,r0
	movd	B_ARG1,r1
	movd	B_ARG2,r2
	sprd	sp,0(r0)
	lprd	sp,0(r1)
	movqd	0,0(r2)
	restore	[r3,r4,r5,r6,r7] /* don't use exit: fp points to old stack */
	lprd	fp,tos		
	ret	0

/*
 * Create a new stack frame for a 'waiting' thread,
 * save current thread's frame, and switch to waiting thread.
 *
 * void cproc_start_wait(int *cur,
 *			 cproc_t child,
 *			 int stackp,
 *			 int *lock)
 */
ENTRY(cproc_start_wait)
	enter	[r3,r4,r5,r6,r7],0
	movd	B_ARG0,r0		/* get cur */
	sprd	sp,0(r0)		/* save current sp */
	movd	B_ARG1,r0		/* get child thread */
	movd	B_ARG3,r1		/* get address of lock */
        lprd	sp,B_ARG2		/* childs' stack */
	movd	r0,tos			/* push child thread */
	lprd	fp,0			/* CLEAR frame pointer */
	movqd	0,0(r1)			/* unlock */
	bsr	EX(cproc_waiting)	/* call cproc_waiting for our child */
	/*NOTREACHED*/

/*
 * Set up a thread's stack so that when cproc_switch switches to
 * it, it will start up as if it called
 * cproc_body(child)
 *
 * void cproc_prepare(cproc_t child, int *context, int stack)
 */
ENTRY(cproc_prepare)
	enter	[],0		/* push fp, load fp from sp */
	movd	B_ARG2,r0	/* get child threads' stack */

        subd	36,r0
       				/* Make room for the context:
				   (Check enter[] register push order!!)
				   0	saved R7
				   4	saved R6
				   8	saved R5
				  12	saved R4
				  16	saved R3
				  20	saved FP
				  24	return PC from cproc_switch
				  28	return PC from cthread_body
				  32	argument to cthread_body
				 */

	movqd	0,20(r0)	/* CLEAR FP */

	/* fake return address from cthread_body */
	movd	EX(cthread_body),24(r0) 

	movd	B_ARG0,32(r0)	/* stuff child argument pointer */

	movd	B_ARG1,r1	/* get pointer to context */
	movd	r0,0(r1)	/* save context */

	exit	[]
	ret	0
