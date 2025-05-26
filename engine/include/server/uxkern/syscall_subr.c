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
 * $Log:	syscall_subr.c,v $
 * Revision 2.2  92/05/25  14:46:36  rwd
 * 	Add SSTRC code.
 * 	[92/05/21            rwd]
 * 
 * 	Allow multithreaded programs by condition_waiting for other
 * 	thread to finish.
 * 	[92/04/22            rwd]
 * 
 * Revision 2.1  92/04/21  17:11:10  rwd
 * BSDSS
 * 
 *
 */

/*
 * Server thread service routines.
 */
#define VICE 0

#include <uxkern/import_mach.h>

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/parallel.h>
#include <sys/ptrace.h>

#include <sys/user.h>

#include <uxkern/bsd_msg.h>
#include <uxkern/proc_to_task.h>

#include <uxkern/syscalltrace.h>

#if	SYSCALLTRACE
extern int	nsysent;
int		syscalltrace = 0;
extern char	*syscallnames[];
#endif


/*
 * Register a server thread as serving a process.
 */
void
server_thread_register(p)
	register struct proc *	p;
{
	mutex_lock(&p->p_lock);
	if (p->p_servers) {
	    p->p_servers = (int)ur_cthread_self();
	    while (p->p_servers == (int)ur_cthread_self())
		condition_wait(&p->p_condition, &p->p_lock);
	}
	p->p_servers = (int)ur_cthread_self();
	mutex_unlock(&p->p_lock);
	cthread_set_data(cthread_self(),(any_t)p);
}

/*
 * Unregister a server thread.
 */
void
server_thread_deregister(p)
	register struct proc *	p;
{
	mutex_lock(&p->p_lock);
	if (p->p_servers != (int)ur_cthread_self())
	    condition_broadcast(&p->p_condition);
	p->p_servers = FALSE;
	mutex_unlock(&p->p_lock);
}

/*
 * Start UX server call: map port to process, set up
 * U area.
 */
int
start_server_op(port, syscode)
	mach_port_t	port;
	int		syscode;
{
	register struct proc	*p;
	int error = 0;

	if ((p = port_to_proc_lookup(port)) == (struct proc *)0)
		return (ESRCH);
	if (p->p_flag & SWEXIT) return (MIG_NO_REPLY);
	if (p->p_cursig && syscode < 1000)
		return (ERESTART);

	server_thread_register(p);

#if SSTRC
	if (p->p_flag & STRC && p->p_flag & SSTRC) {
	    unix_master();
	    /* Copy args to u. */
#if 0
	    bcopy((char *)args, (char *)p->p_addr->u_arg, sizeof(args));
#endif 0
	    p->p_addr->u_scno = syscode;
	    p->p_xstat = SIGTRAP;
	    if (p->p_tptr)
		psignal(p->p_tptr, SIGCHLD);
	    else
		psignal(p->p_pptr, SIGCHLD);
	    stop(p);
	    sleep((caddr_t)&p->p_stat, PSPECL);
	    p->p_addr->u_scno = 0;
	    unix_release();
	}
#endif
#if	SYSCALLTRACE
	if (syscalltrace &&
		(syscalltrace == p->p_pid || syscalltrace < 0)) {

	    char *s;
	    char num[10];
	    static char * extra_syscallnames[] = {
		    "1000(take_signal)",	/* 1000 */
		    "1001(task_by_pid)",	/* 1001 */
		    "1002(init_process)",	/* 1002 */
		    "1003(exec_args_set)",	/* 1003 */
		    "1004",			/* 1004 */
		    "1005(maprw_request)",	/* 1005 */
		    "1006(maprw_release_it)",	/* 1006 */
		    "1007(maprw_remap)"		/* 1007 */
	    };
	    static int extra_nsysent = sizeof(extra_syscallnames) / 
		    sizeof(extra_syscallnames[0]);

	    if (syscode >= nsysent || syscode < 0) {

		if (syscode - 1000 >= extra_nsysent || syscode < 0) {
			sprintf(num, "%d", syscode);
			s = num;
		} else {
			s = extra_syscallnames[syscode - 1000];
		}
	    }
	    else
		s = syscallnames[syscode];
	    printf("\n[%d]%s", p->p_pid, s);
	}
#endif	SYSCALLTRACE
	return(error);
}

int
end_server_op(p, error, interrupt)
    register int	error;
    boolean_t	*interrupt;
    register struct proc *p;
{
	if (p == 0) {
	    return (MIG_NO_REPLY);
	}

#if	CMUCS || VICE
	if (error) {
	    switch (error) {
#if	CMUCS
		case EDQUOT:
		case ENOSPC:
		    break;
#endif	CMUCS
#if	VICE
		case EVICEOP:
		    error = 0;
		    break;
#endif	VICE
		case EJUSTRETURN:
		    error = MIG_NO_REPLY;
		    break;
	    }
	}
#endif	CMUCS || VICE

	*interrupt = FALSE;
	if (!EXITING(p)) {
	    int sig;
	    if (p->p_cursig != 0 || HAVE_SIGNALS(p)) {
		unix_master();
		if ((sig = p->p_cursig) || (sig = issig(p)))
		    psig(sig);
		if (p->p_cursig)	/* user should take signal */
		    *interrupt = TRUE;
		unix_release();

#if 0
XXX		/* psig might have killed us */
		if (uth->uu_procp == 0) {
		    if (p->p_master_lock)
			panic("Master still held", uth->uu_master_lock);

#if	MAP_UAREA
		    if (uth->uu_share_lock_count) {
			 panic("Share lock still held", uth->uu_share_lock_count);
			uth->uu_share_lock_count = 0;
		    }
#endif	MAP_UAREA

		    return (MIG_NO_REPLY);
		}
#endif 0
	    }
	}

#ifdef SSTRC
	if (p->p_flag & STRC && p->p_flag & SSTRC) {
	    unix_master();
	    /* Copy args to u. */
#if 0
	    bcopy((char *)args, (char *)p->p_addr->u_arg, sizeof(args));
	    p->p_addr->u_rval[0] = rval[0];
	    p->p_addr->u_rval[1] = rval[1];
#endif 0
	    p->p_addr->u_error = error;
	    p->p_flag &= ~SSTRC;
	    p->p_xstat = SIGTRAP;
	    if (p->p_tptr)
		psignal(p->p_tptr, SIGCHLD);
	    else
		psignal(p->p_pptr, SIGCHLD);
	    stop(p);
	    sleep((caddr_t)&p->p_stat, PSPECL);
	    unix_release();
	}
#endif
#if	SYSCALLTRACE
	if (syscalltrace &&
		(syscalltrace == p->p_pid || syscalltrace < 0)) {

	    printf("    [%d] returns %d%s",
		p->p_pid,
		error,
		(*interrupt) ? " Interrupt" : "");
	}
#endif

	server_thread_deregister(p);

	if (p->p_flag & SWEXIT)
	    wakeup((caddr_t)&p->p_flag);

	if (p->p_master_lock)
	    panic("Master still held", p->p_master_lock);

	if (p->p_ipl)
	    panic("IPL above 0", p->p_ipl);

	return (error);
}

