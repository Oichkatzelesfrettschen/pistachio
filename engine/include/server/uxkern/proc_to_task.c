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
 * $Log:	proc_to_task.c,v $
 * Revision 2.1  92/04/21  17:11:03  rwd
 * BSDSS
 * 
 *
 */

/*
 * Task->process and request_port->process routines.
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/queue.h>

#include <uxkern/import_mach.h>

extern void	ux_server_add_port();
extern void	ux_server_remove_port();

#define	NTASKHASH	32

#if	((NTASKHASH-1)&NTASKHASH) == 0
#define	TASKHASH(t)	(((unsigned int)t) & (NTASKHASH-1))
#else
#define	TASKHASH(t)	(((unsigned int)t) % NTASKHASH)
#endif

struct proc_hash {
	queue_chain_t	task_to_proc_chain;
	task_t		task;
	struct proc *	proc;
};

queue_head_t	task_to_proc_hash[NTASKHASH];

struct mutex	task_to_proc_lock = MUTEX_INITIALIZER;

void task_to_proc_init()
{
	register int	i;

	for (i = 0; i < NTASKHASH; i++)
	    queue_init(&task_to_proc_hash[i]);
}

/*
 * Enter task and process in hash table.
 * Allocate request port for process, and return it.
 */
mach_port_t
task_to_proc_enter(task, p)
	task_t		task;
	struct proc *	p;
{
	register struct proc_hash *ph;
	register queue_t q;
	mach_port_t	req_port;
	kern_return_t	kr;

	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
				&req_port);
	if (kr != KERN_SUCCESS)
	    panic("task_to_proc_enter: can't allocate request port");

	kr = mach_port_insert_right(mach_task_self(), req_port, req_port,
				    MACH_MSG_TYPE_MAKE_SEND);
	if (kr != KERN_SUCCESS)
	    panic("task_to_proc_enter: can't acquire send rights");

	kr = mach_port_rename(mach_task_self(), req_port, (mach_port_t) p);
	if (kr != KERN_SUCCESS)
	    panic("task_to_proc_enter: can't rename port");

	req_port = (mach_port_t) p;
	ux_server_add_port(req_port);

	ph = (struct proc_hash *)malloc(sizeof(struct proc_hash));

	ph->task = task;
	ph->proc = p;

	mutex_lock(&task_to_proc_lock);

	q = &task_to_proc_hash[TASKHASH(task)];
	queue_enter(q, ph, struct proc_hash *, task_to_proc_chain);

	mutex_unlock(&task_to_proc_lock);

	return (req_port);
}

struct proc *
task_to_proc_lookup(task)
	register task_t	task;
{
	struct proc *p = 0;

	register struct proc_hash *ph;
	register queue_t q;

	mutex_lock(&task_to_proc_lock);

	q = &task_to_proc_hash[TASKHASH(task)];
	for (ph = (struct proc_hash *)queue_first(q);
	     !queue_end(q, (queue_entry_t)ph);
	     ph = (struct proc_hash *)queue_next(&ph->task_to_proc_chain)) {
	    if (ph->task == task) {
		p = ph->proc;
		break;
	    }
	}

	mutex_unlock(&task_to_proc_lock);
	return (p);
}

void
task_to_proc_remove(task)
	register task_t	task;
{
	register struct proc_hash *ph;
	register queue_t q;

	register struct proc *p;
	mach_port_t req_port;

	mutex_lock(&task_to_proc_lock);

	q = &task_to_proc_hash[TASKHASH(task)];
	for (ph = (struct proc_hash *)queue_first(q);
	     !queue_end(q, (queue_entry_t)ph);
	     ph = (struct proc_hash *)queue_next(&ph->task_to_proc_chain)) {
	    if (ph->task == task) {
		p = ph->proc;
		break;
	    }
	}
	if (p == 0)
	    panic("task_to_proc_remove");

	req_port = (mach_port_t) p;

	queue_remove(q, ph, struct proc_hash *, task_to_proc_chain);

	mutex_unlock(&task_to_proc_lock);

	free((char *)ph);

	(void) mach_port_destroy(mach_task_self(), req_port);
}
