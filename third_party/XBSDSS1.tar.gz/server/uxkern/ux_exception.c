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
 * $Log:	ux_exception.c,v $
 * Revision 2.1  92/04/21  17:11:14  rwd
 * BSDSS
 * 
 *
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/user.h>

#include <mach/exception.h>
#include <sys/ux_exception.h>

#include <uxkern/import_mach.h>

/*
 *	Unix exception handler.
 */

any_t		ux_handler();	/* forward */
extern void	ux_create_thread();

static mach_port_t	ux_local_port;

/*
 * Returns exception port to map exceptions to signals.
 */
mach_port_t	ux_handler_setup()
{
	register kern_return_t	r;

	/*
	 *	Allocate the exception port.
	 */
	r = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
			       &ux_local_port);
	if (r != KERN_SUCCESS)
		panic("ux_handler_setup: can't allocate");

	r = mach_port_insert_right(mach_task_self(),
				   ux_local_port, ux_local_port,
				   MACH_MSG_TYPE_MAKE_SEND);
	if (r != KERN_SUCCESS)
		panic("ux_handler_setup: can't acquire send right");

	/*
	 * Add it to the server port set.
	 */
	ux_server_add_port(ux_local_port);

	/*
	 * Return the exception port.
	 */

	return (ux_local_port);
}

void	ux_exception();	/* forward */

kern_return_t
catch_exception_raise(exception_port, thread, task,
	exception, code, subcode)
	mach_port_t	exception_port;
	thread_t	thread;
	task_t		task;
	int		exception, code, subcode;
{
	int	signal = 0, ux_code = 0;
	int	ret = KERN_SUCCESS;

#ifdef	lint
	exception_port++;
#endif	lint

	/*
	 *	Catch bogus ports
	 */
	if (MACH_PORT_VALID(task) && MACH_PORT_VALID(thread)) {

	    /*
	     *	Convert exception to unix signal and code.
	     */
	    ux_exception(exception, code, subcode, &signal, &ux_code);

	    /*
	     *	Send signal.
	     */
	    if (signal != 0) {
		thread_psignal(task, thread, signal, ux_code);
	    }
	} else {
	    printf("catch_exception_raise: task %x thread %x\n",
		   task, thread);
	    ret = KERN_INVALID_ARGUMENT;
	}

	if (MACH_PORT_VALID(task))
		(void) mach_port_deallocate(mach_task_self(), task);

	if (MACH_PORT_VALID(thread))
		(void) mach_port_deallocate(mach_task_self(), thread);

	return(ret);
}

extern boolean_t	machine_exception();

/*
 *	ux_exception translates a mach exception, code and subcode to
 *	a signal and u.u_code.  Calls machine_exception (machine dependent)
 *	to attempt translation first.
 */

void ux_exception(exception, code, subcode, ux_signal, ux_code)
int	exception, code, subcode;
int	*ux_signal, *ux_code;
{
	/*
	 *	Try machine-dependent translation first.
	 */
	if (machine_exception(exception, code, subcode, ux_signal, 
	    ux_code))
		return;
	
	switch(exception) {

	    case EXC_BAD_ACCESS:
		if (code == KERN_INVALID_ADDRESS)
		    *ux_signal = SIGSEGV;
		else
		    *ux_signal = SIGBUS;
		break;

	    case EXC_BAD_INSTRUCTION:
	        *ux_signal = SIGILL;
		break;

	    case EXC_ARITHMETIC:
	        *ux_signal = SIGFPE;
		break;

	    case EXC_EMULATION:
		*ux_signal = SIGEMT;
		break;

	    case EXC_SOFTWARE:
		switch (code) {
		    case EXC_UNIX_BAD_SYSCALL:
			*ux_signal = SIGSYS;
			break;
		    case EXC_UNIX_BAD_PIPE:
		    	*ux_signal = SIGPIPE;
			break;
		    case EXC_UNIX_ABORT:
			*ux_signal = SIGABRT;
			break;
		}
		break;

	    case EXC_BREAKPOINT:
		*ux_signal = SIGTRAP;
		break;
	}
}
