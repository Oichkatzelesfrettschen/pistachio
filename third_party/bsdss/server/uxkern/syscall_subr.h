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
 * $Log:	syscall_subr.h,v $
 * Revision 2.1  92/04/21  17:11:16  rwd
 * BSDSS
 * 
 *
 */

/*
 * Glue to make MiG procedure interfaces look like old UX system calls.
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/parallel.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/errno.h>

#define	START_SERVER(syscode, nargs)			\
	register int	error;				\
        struct proc *p = (struct proc *)proc_port;	\
        int was_serial = TRUE;				\
							\
	error = start_server_op(proc_port, syscode);	\
	if (error)					\
	    return (error);				\
	p->p_reply_msg = 0;				\
	if (was_serial)					\
	    unix_master();				\
	{						\
		int	arg[nargs];

#define	END_SERVER(syscode)				\
	}						\
	if (was_serial)					\
	    unix_release();				\
	return (end_server_op(p, error, interrupt));

#define	END_SERVER_DEALLOC(syscode, data, size)		\
	}						\
        unix_release();		 		        \
	(void) vm_deallocate(mach_task_self(), data, size); \
	return (end_server_op(p, error, interrupt));

