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
 * 	Created.
 * 
 * $Log:$
 */

#ifndef _MACHINE_CTHREADS_H_
#define _MACHINE_CTHREADS_H_

#ifdef __GNUC__

#define cthread_sp() \
  ({ int __c_sp; \
       asm ("sprd sp,%0": "=g" (__c_sp): /* No inputs */); \
	 __c_sp; })

#else
#define cthread_sp() __cthread_sp()
#endif /*__GNUC__*/

#ifdef DEBUG
#define	ASSERT(p)	do { int ___foo___; \
			  if (!(p)) { \
			     printf( \
				"File %s, line %d: assertion p failed.\n", \
				__FILE__, __LINE__); \
			     for(___foo___=0;___foo___<500000;___foo___++) \
			       ; \
			     asm("bpt"); \
			} } while(0)
#else /*DEBUG*/
#define ASSERT(p)
#endif /*DEBUG*/

typedef int spin_lock_t;
#define SPIN_LOCK_INITIALIZER 0
#define spin_lock_init(s) *(s)=0
#define spin_lock_locked(s) (*(s) != 0)

#endif _MACHINE_CTHREADS_H_
