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
 * 12-Nov-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Added pc532.
 *
 * 12-Nov-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Changed name of exit() to do_exit() to be able to use builtin
 *	prototypes wtih gcc.
 *
 * $Log:	kern_exit.c,v $
 * Revision 2.2  92/05/25  14:43:24  rwd
 * 	Clean up so exit can be called on behalf of a proc
 * 	by another like in ptrace.
 * 	[92/05/25            rwd]
 * 
 * Revision 2.1  92/04/21  17:12:56  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1982, 1986, 1989, 1991 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)kern_exit.c	7.35 (Berkeley) 6/27/91
 */

#include <ktrace.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/syslog.h>
#include <sys/malloc.h>
#include <sys/resourcevar.h>
#include <sys/synch.h>
#include <sys/parallel.h>

#ifdef COMPAT_43
#include <machine/reg.h>
#include <machine/psl.h>
#endif

#include <uxkern/import_mach.h>

/*
 * Exit system call: pass back caller's arg
 */
/* ARGSUSED */
rexit(p, uap, retval)
	struct proc *p;
	struct args {
		int	rval;
	} *uap;
	int *retval;
{

	do_exit(p, W_EXITCODE(uap->rval, 0));
	return EJUSTRETURN;
}

/*
 * Exit: deallocate address space and other resources,
 * change proc state to zombie, and unlink proc from allproc
 * and parent's lists.  Save exit status and rusage for wait().
 * Check for child processes and orphan them.
 */
do_exit(p, rv)
	register struct proc *p;
	int rv;
{
	register struct proc *q, *nq;
	register struct proc **pp;
	int s;

	/*
	 * If the task is already exiting, quit.
	 */
	if (p->p_flag & SWEXIT)
	    return;
#ifdef PGINPROF
	vmsizmon();
#endif
	MALLOC(p->p_ru, struct rusage *, sizeof(struct rusage),
		M_ZOMBIE, M_WAITOK);
	/*
	 * If parent is waiting for us to exit or exec,
	 * SPPWAIT is set; we will wakeup the parent below.
	 */
	p->p_flag &= ~(STRC|SPPWAIT);
	p->p_flag |= SWEXIT;
	p->p_sigignore = ~0;
	p->p_sig = 0;
	untimeout(realitexpire, (caddr_t)p);

	/*
	 * Suspend the user task.
	 */
	(void) task_suspend(p->p_task);

	/*
	 * If the process is stopped, wake up the server threads.
	 * Do not use start() because that will change the stop signal.
	 */
	if (p->p_stat == SSTOP) {
	    p->p_stat = SRUN;
	    wakeup((caddr_t)&p->p_stat);
	}

	if (p->p_wchan)
	    panic("exit:wchan");

	/*
	 * Close open files and release open-file table.
	 * This may block!
	 */
	fdfree(p);

#if SYSVSHM
	if (p->p_vmspace->vm_shm)
		shmexit(p);
#endif

	if (p->p_pid == 1)
		panic("init died");
	if (SESS_LEADER(p)) {
		register struct session *sp = p->p_session;

		if (sp->s_ttyvp) {
			/*
			 * Controlling process.
			 * Signal foreground pgrp,
			 * drain controlling terminal
			 * and revoke access to controlling terminal.
			 */
			if (sp->s_ttyp->t_session == sp) {
				if (sp->s_ttyp->t_pgrp)
					pgsignal(sp->s_ttyp->t_pgrp, SIGHUP, 1);
				(void) ttywait(sp->s_ttyp);
				vgoneall(sp->s_ttyvp);
			}
			vrele(sp->s_ttyvp);
			sp->s_ttyvp = NULL;
			/*
			 * s_ttyp is not zero'd; we use this to indicate
			 * that the session once had a controlling terminal.
			 * (for logging and informational purposes)
			 */
		}
		sp->s_leader = NULL;
	}
	fixjobc(p, p->p_pgrp, 0);
	p->p_rlimit[RLIMIT_FSIZE].rlim_cur = RLIM_INFINITY;
	(void) acct(p);
	if (--p->p_limit->p_refcnt == 0)
		FREE(p->p_limit, M_SUBPROC);
#if KTRACE
	/* 
	 * release trace file
	 */
	if (p->p_tracep)
		vrele(p->p_tracep);
#endif

	/*
	 * Remove proc from allproc queue and pidhash chain.
	 * Place onto zombproc.  Unlink from parent's child list.
	 */
	if (*p->p_prev = p->p_nxt)
		p->p_nxt->p_prev = p->p_prev;
	if (p->p_nxt = zombproc)
		p->p_nxt->p_prev = &p->p_nxt;
	p->p_prev = &zombproc;
	zombproc = p;
	task_to_proc_remove(p->p_task);
	p->p_stat = SZOMB;
	for (pp = &pidhash[PIDHASH(p->p_pid)]; *pp; pp = &(*pp)->p_hash)
		if (*pp == p) {
			*pp = p->p_hash;
			goto done;
		}
	panic("do_exit");
done:

	if (p->p_cptr)		/* only need this if any child is S_ZOMB */
		wakeup((caddr_t) initproc);
	for (q = p->p_cptr; q != NULL; q = nq) {
		nq = q->p_osptr;
		if (nq != NULL)
			nq->p_ysptr = NULL;
		if (initproc->p_cptr)
			initproc->p_cptr->p_ysptr = q;
		q->p_osptr = initproc->p_cptr;
		q->p_ysptr = NULL;
		initproc->p_cptr = q;

		q->p_pptr = initproc;
		/*
		 * Traced processes are killed
		 * since their existence means someone is screwing up.
		 */
		if (q->p_flag&STRC) {
			q->p_flag &= ~STRC;
			psignal(q, SIGKILL);
		}
	}
	p->p_cptr = NULL;

	/*
	 * Save exit status and final rusage info,
	 * adding in child rusage info and self times.
	 */
	p->p_xstat = rv;
	*p->p_ru = p->p_stats->p_ru;
	p->p_ru->ru_stime = p->p_stime;
	p->p_ru->ru_utime = p->p_utime;
	ruadd(p->p_ru, &p->p_stats->p_cru);

	/*
	 * Notify parent that we're gone.
	 */
	psignal(p->p_pptr, SIGCHLD);
	wakeup((caddr_t)p->p_pptr);
#if defined(tahoe)
	/* move this to cpu_exit */
	p->p_addr->u_pcb.pcb_savacc.faddr = (float *)NULL;
#endif
	(void) task_terminate(p->p_task);

	vm_exit(p);

	/*
	 * Yes, it can happen.  The user-reference counts on
	 * the task/thread port names might be greater than one
	 * because other threads in the Unix server were doing
	 * things with the ports.
	 */

	(void) mach_port_deallocate(mach_task_self(), p->p_task);
	p->p_task = MACH_PORT_NULL;
	if (MACH_PORT_VALID(p->p_thread))
	    (void) mach_port_deallocate(mach_task_self(), p->p_thread);
	p->p_thread = MACH_PORT_NULL;
}

#ifdef COMPAT_43
owait(p, uap, retval)
	struct proc *p;
	register struct args {
		int	pid;
		int	*status;
		int	options;
		struct	rusage *rusage;
		int	compat;
	} *uap;
	int *retval;
{

#ifdef PC532			/* XXX pc532 proc struct has no regs */
		uap->options = 0;
		uap->rusage = 0;
#else PC532
	if ((p->p_regs[PS] & PSL_ALLCC) != PSL_ALLCC) {
		uap->options = 0;
		uap->rusage = 0;
	} else {
		uap->options = p->p_regs[R0];
		uap->rusage = (struct rusage *)p->p_regs[R1];
	}
#endif PC532
	uap->pid = WAIT_ANY;
	uap->status = 0;
	uap->compat = 1;
	return (wait1(p, uap, retval));
}

wait4(p, uap, retval)
	struct proc *p;
	struct args {
		int	pid;
		int	*status;
		int	options;
		struct	rusage *rusage;
		int	compat;
	} *uap;
	int *retval;
{

	uap->compat = 0;
	return (wait1(p, uap, retval));
}
#else
#define	wait1	wait4
#endif

/*
 * Wait: check child processes to see if any have exited,
 * stopped under trace, or (optionally) stopped by a signal.
 * Pass back status and deallocate exited child's proc structure.
 */
wait1(q, uap, retval)
	register struct proc *q;
	register struct args {
		int	pid;
		int	*status;
		int	options;
		struct	rusage *rusage;
#ifdef COMPAT_43
		int compat;
#endif
	} *uap;
	int retval[];
{
	register int nfound;
	register struct proc *p;
	int status, error;

	if (uap->pid == 0)
		uap->pid = -q->p_pgid;
#ifdef notyet
	if (uap->options &~ (WUNTRACED|WNOHANG))
		return (EINVAL);
#endif
loop:
	nfound = 0;
	for (p = q->p_cptr; p; p = p->p_osptr) {
		if (uap->pid != WAIT_ANY &&
		    p->p_pid != uap->pid && p->p_pgid != -uap->pid)
			continue;
		nfound++;
		if (p->p_stat == SZOMB) {
			retval[0] = p->p_pid;
#ifdef COMPAT_43
			if (uap->compat)
				retval[1] = p->p_xstat;
			else
#endif
			if (uap->status) {
				status = p->p_xstat;	/* convert to int */
				if (error = copyout((caddr_t)&status,
				    (caddr_t)uap->status, sizeof(status)))
					return (error);
			}
			if (uap->rusage && (error = copyout((caddr_t)p->p_ru,
			    (caddr_t)uap->rusage, sizeof (struct rusage))))
				return (error);
			p->p_xstat = 0;
			ruadd(&q->p_stats->p_cru, p->p_ru);
			FREE(p->p_ru, M_ZOMBIE);
			if (--p->p_cred->p_refcnt == 0) {
				crfree(p->p_cred->pc_ucred);
				FREE(p->p_cred, M_SUBPROC);
			}

			/*
			 * Finally finished with old proc entry.
			 * Unlink it from its process group and free it.
			 */
			leavepgrp(p);
			if (*p->p_prev = p->p_nxt)	/* off zombproc */
				p->p_nxt->p_prev = p->p_prev;
			if (q = p->p_ysptr)
				q->p_osptr = p->p_osptr;
			if (q = p->p_osptr)
				q->p_ysptr = p->p_ysptr;
			if ((q = p->p_pptr)->p_cptr == p)
				q->p_cptr = p->p_osptr;

			FREE(p, M_PROC);
			nprocs--;
			return (0);
		}
		if (p->p_stat == SSTOP && (p->p_flag & SWTED) == 0 &&
		    (p->p_flag & STRC || uap->options & WUNTRACED)) {
			p->p_flag |= SWTED;
			retval[0] = p->p_pid;
#ifdef COMPAT_43
			if (uap->compat) {
				retval[1] = W_STOPCODE(p->p_xstat);
				error = 0;
			} else
#endif
			if (uap->status) {
				status = W_STOPCODE(p->p_xstat);
				error = copyout((caddr_t)&status,
				    (caddr_t)uap->status, sizeof(status));
			} else
				error = 0;
			return (error);
		}
	}
	if (nfound == 0)
		return (ECHILD);
	if (uap->options & WNOHANG) {
		retval[0] = 0;
		return (0);
	}
	if (error = tsleep((caddr_t)q, PWAIT | PCATCH, "wait", 0))
		return (error);
	goto loop;
}

/*
 *	Make the current process an "init" process, meaning
 *	that it doesn't have a parent, and that it won't be
 *	gunned down by kill(-1, 0).
 */

kern_return_t	init_process(p)
	register struct proc *p;
{


	if (!suser(p->p_ucred, &p->p_acflag))
		return(KERN_NO_ACCESS);

	unix_master();

	/*
	 *	Take us out of the sibling chain, and
	 *	out of our parent's child chain.
	 */

	if (p->p_osptr)
		p->p_osptr->p_ysptr = p->p_ysptr;
	if (p->p_ysptr)
		p->p_ysptr->p_osptr = p->p_osptr;
	if (p->p_pptr->p_cptr == p)
		p->p_pptr->p_cptr = p->p_osptr;
	p->p_pptr = p;
	p->p_ysptr = p->p_osptr = 0;
	leavepgrp(p);
	p->p_pptr = 0;

	unix_release();
	return(KERN_SUCCESS);
}

int
bsd_init_process(proc_port, interrupt)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
{
	register int	error;
	register struct proc *p;

	if (error = start_server_op(proc_port, 1002))
	    return (error);

	p = (struct proc *)cthread_data(cthread_self());

	init_process(p);

	return (end_server_op(p, error, interrupt));
}
