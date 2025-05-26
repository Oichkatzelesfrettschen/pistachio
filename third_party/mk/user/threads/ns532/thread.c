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
 * 28-May-93  Johannes Helander (jvh) at Helsinki University of Technology
 *	Include mach.h instead of mach/mach.h.
 *
 * 14-Apr-93  Ian Dall (ian) at University of Adelaide
 *	Changed types of arguments tp cproc_setup to match prototype.
 *
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created from i386 version.
 *
 * $Log:$
 */

#include <cthreads.h>
#include "cthread_internals.h"


#include <mach.h>

/*
 * C library imports:
 */
extern bzero();

/* 
 * If __GNUC__ an inlined macro is used instead.
 * This function is for loosers.
 */
int __cthread_sp()
{
	int x;

	return (int) &x;
}

/*
 * Set up the initial state of a MACH thread
 * so that it will invoke cthread_body(child)
 * when it is resumed.
 * 
 * No need to set the float state here. All regs will be set to zero by 
 * the kernel.
 */
void
cproc_setup(cproc_t child, thread_t thread, void (*routine)(cproc_t))
{
	register int *top = (int *) (child->stack_base + child->stack_size);
	struct ns532_thread_state state;
	register struct ns532_thread_state *ts = &state;
	kern_return_t r;
	unsigned int count;

	/*
	 * Set up ns532 call frame and registers.
	 * Read registers first to get correct segment values.
	 */
	count = NS532_THREAD_STATE_COUNT;
	MACH_CALL(thread_get_state(thread, 
				   NS532_THREAD_STATE,
				   (thread_state_t) &state, 
				   &count),
		  r);

	ts->pc = routine;
	*--top = (int) child;	/* argument to function */
	*--top = 0;		/* fake return address */
	ts->sp = (int) top;	/* set stack pointer */
	ts->fp = 0;		/* clear frame pointer */

	MACH_CALL(thread_set_state(thread,
				   NS532_THREAD_STATE,
				   (thread_state_t) &state,
				   NS532_THREAD_STATE_COUNT),
		  r);
}
