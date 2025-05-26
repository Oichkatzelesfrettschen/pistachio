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
 * $Log:	kern_fork.c,v $
 * Revision 2.2  92/05/25  14:43:31  rwd
 * 	Include ptrace code.
 * 	[92/05/20            rwd]
 * 
 * Revision 2.1  92/04/21  17:13:27  rwd
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
 *	@(#)kern_fork.c	7.29 (Berkeley) 5/15/91
 */

#include <ktrace.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/resourcevar.h>
#include <sys/vnode.h>
#include <sys/seg.h>
#include <sys/file.h>
#include <sys/acct.h>
#include <sys/ktrace.h>
#include <sys/synch.h>
#include <uxkern/import_mach.h>

int max_pid = 0;

struct proc *newproc();

/* ARGSUSED */
fork(p, uap, retval)
	struct proc *p;
	void *uap;
	int retval[];
{
    panic("fork called");
	return (fork1(p, 0, retval));
}

/* ARGSUSED */
vfork(p, uap, retval)
	struct proc *p;
	void *uap;
	int retval[];
{
    panic("vfork called");
	return (fork1(p, 1, retval));
}

int	nprocs = 1;		/* process 0 */

fork1(p1, args, retval, isvfork)
    register struct proc *p1;
    void *args;
    int retval[];
    int isvfork;
{
	register struct proc *p2;
	register int count, uid;
	register struct args {
		thread_state_t	new_state;
		unsigned int	new_state_count;
	} *uap = (struct args *) args;

	count = 0;
	if ((uid = p1->p_ucred->cr_uid) != 0) {
		for (p2 = allproc; p2; p2 = p2->p_nxt)
			if (p2->p_ucred->cr_uid == uid)
				count++;
		for (p2 = zombproc; p2; p2 = p2->p_nxt)
			if (p2->p_ucred->cr_uid == uid)
				count++;
	}
	/*
	 * Although process entries are dynamically entries,
	 * we still keep a global limit on the maximum number
	 * we will create.  Don't allow a nonprivileged user
	 * to exceed its current limit or to bring us within one
	 * of the global limit; don't let root exceed the limit.
	 * nprocs is the current number of processes,
	 * maxproc is the limit.
	 */
	if (nprocs >= maxproc || uid == 0 && nprocs >= maxproc + 1) {
		tablefull("proc");
		retval[1]=0;
		return EAGAIN;
	}
	if (count > p1->p_rlimit[RLIMIT_NPROC].rlim_cur) {
	    retval[1]=0;
	    return EAGAIN;
	}

	/*
	 * Create new user process, with task.
	 */
	p2 = newproc(p1, FALSE,isvfork);
	if (p2 == 0) {
	    /*
	     * No tasks or threads available.
	     */
	    retval[1]=0;
	    return EAGAIN;
	}

	/*
	 * Clone the parent's registers, but mark it as the child
	 * process.
	 */
	if (!thread_dup(p2->p_thread,
			uap->new_state, uap->new_state_count,
			p1->p_pid, 1))	{
	    retval[1]=0;
	    task_terminate(p2->p_task);
	    return EFAULT;
	}

	/*
	 * Child process.  Set start time and get to work.
	 */
	get_time(&p2->p_stats->p_start);
	p2->p_acflag = AFORK;

	/*
	 * And start it.
	 */
	(void) thread_resume(p2->p_thread);

	/*
	 * Preserve synchronization semantics of vfork.
	 * If waiting for child to exec or exit, set SPPWAIT
	 * on child, and sleep on our proc (in case of exit).
	 */
	if (isvfork)
		while (p2->p_flag & SPPWAIT)
			tsleep((caddr_t)p1, PWAIT, "ppwait", 0);

	retval[0] = p2->p_pid;
	return (0);
}

void proc_allocate(np)
    struct proc **np;
{
    mutex_lock(&allproc_lock);

    MALLOC(*np, struct proc *, sizeof(struct proc), M_PROC, M_WAITOK);
    nprocs++;
    bzero(*np, sizeof(struct proc));
    (*np)->p_nxt = allproc;
    (*np)->p_nxt->p_prev = &(*np)->p_nxt;	/* allproc is never NULL */
    (*np)->p_prev = &allproc;
    allproc = (*np);
    (*np)->p_link = NULL;			/* shouldn't be necessary */
    (*np)->p_rlink = NULL;			/* shouldn't be necessary */

    mutex_unlock(&allproc_lock);
}


struct proc *
newproc(p1, is_sys_proc, isvfork)
    register struct proc *p1;
    boolean_t is_sys_proc, isvfork;
{
	static int nextpid, pidchecked = 0;
	register struct proc *p2;

	/*
	 * Find an unused process ID.
	 * We remember a range of unused IDs ready to use
	 * (from nextpid+1 through pidchecked-1).
	 */
	nextpid++;
retry:
	/*
	 * If the process ID prototype has wrapped around,
	 * restart somewhat above 0, as the low-numbered procs
	 * tend to include daemons that don't exit.
	 */
	if (nextpid >= PID_MAX) {
		nextpid = 100;
		pidchecked = 0;
	}
	if (nextpid >= pidchecked) {
		int doingzomb = 0;

		pidchecked = PID_MAX;
		/*
		 * Scan the active and zombie procs to check whether this pid
		 * is in use.  Remember the lowest pid that's greater
		 * than nextpid, so we can avoid checking for a while.
		 */
		mutex_lock(&allproc_lock);
		p2 = allproc;
again:
		for (; p2 != NULL; p2 = p2->p_nxt) {
			if (p2->p_pid == nextpid ||
			    p2->p_pgrp->pg_id == nextpid) {
				nextpid++;
				if (nextpid >= pidchecked) {
				    mutex_unlock(&allproc_lock);
					goto retry;
				    }
			}
			if (p2->p_pid > nextpid && pidchecked > p2->p_pid)
				pidchecked = p2->p_pid;
			if (p2->p_pgrp->pg_id > nextpid && 
			    pidchecked > p2->p_pgrp->pg_id)
				pidchecked = p2->p_pgrp->pg_id;
		}
		if (!doingzomb) {
			doingzomb = 1;
			p2 = zombproc;
			goto again;
		}
		mutex_unlock(&allproc_lock);
	}

	max_pid = (max_pid>nextpid?max_pid:nextpid);

	/*
	 * Allocate new proc.
	 * Link onto allproc (this should probably be delayed).
	 */

    {
	struct proc *np;
	proc_allocate(&np);
	p2 = np;
    }

	/*
	 * Make a proc table entry for the new process.
	 * Start by zeroing the section of proc that is zero-initialized,
	 * then copy the section that is copied directly from the parent.
	 */
	bzero(&p2->p_startzero,
	    (unsigned) ((caddr_t)&p2->p_endzero - (caddr_t)&p2->p_startzero));
	bcopy(&p1->p_startcopy, &p2->p_startcopy,
	    (unsigned) ((caddr_t)&p2->p_endcopy - (caddr_t)&p2->p_startcopy));
	p2->p_spare[0] = 0;	/* XXX - should be in zero range */
	p2->p_spare[1] = 0;	/* XXX - should be in zero range */
	p2->p_spare[2] = 0;	/* XXX - should be in zero range */
	p2->p_spare[3] = 0;	/* XXX - should be in zero range */

	{
	    kern_return_t	result;
	    mach_port_t		new_req_port;

	    result = task_create(p1->p_task, TRUE, &p2->p_task);
	    if (result != KERN_SUCCESS) {
		printf("kern_fork:task create failure %x\n",result);
		return 0;
	    }
	    result = thread_create(p2->p_task, &p2->p_thread);
	    if (result != KERN_SUCCESS) {
		(void) task_terminate(p2->p_task);
		(void) mach_port_deallocate(mach_task_self(), p2->p_task);
		printf("mach_fork:thread create failure %x\n",result);
		return 0;
	    }
	    new_req_port = task_to_proc_enter(p2->p_task, p2);

	    /*
	     * Insert the BSD request port for the task as
	     * its bootstrap port.
	     */
	    result = task_set_bootstrap_port(p2->p_task,
					     new_req_port);
	    if (result != KERN_SUCCESS)
		panic("set bootstrap port on fork");
	}

	/*
	 * Duplicate sub-structures as needed.
	 * Increase reference counts on shared objects.
	 * The p_stats and p_sigacts substructs are set in vm_fork.
	 */
	MALLOC(p2->p_cred, struct pcred *, sizeof(struct pcred),
	    M_SUBPROC, M_WAITOK);
	bcopy(p1->p_cred, p2->p_cred, sizeof(*p2->p_cred));
	p2->p_cred->p_refcnt = 1;
	crhold(p1->p_ucred);

	p2->p_fd = fdcopy(p1);
	/*
	 * If p_limit is still copy-on-write, bump refcnt,
	 * otherwise get a copy that won't be modified.
	 * (If PL_SHAREMOD is clear, the structure is shared
	 * copy-on-write.)
	 */
	if (p1->p_limit->p_lflags & PL_SHAREMOD)
		p2->p_limit = limcopy(p1->p_limit);
	else {
		p2->p_limit = p1->p_limit;
		p2->p_limit->p_refcnt++;
	}

	p2->p_flag = SLOAD | (p1->p_flag & SHPUX);
	if (p1->p_session->s_ttyvp != NULL && p1->p_flag & SCTTY)
		p2->p_flag |= SCTTY;
	p2->p_stat = SIDL;
	if (isvfork)
		p2->p_flag |= SPPWAIT;
	p2->p_pid = nextpid;
	{
	struct proc **hash = &pidhash[PIDHASH(p2->p_pid)];

	p2->p_hash = *hash;
	*hash = p2;
	}
	p2->p_pgrpnxt = p1->p_pgrpnxt;
	p1->p_pgrpnxt = p2;
	p2->p_pptr = p1;
	p2->p_osptr = p1->p_cptr;
	if (p1->p_cptr)
		p1->p_cptr->p_ysptr = p2;
	p1->p_cptr = p2;
#if KTRACE
	/*
	 * Copy traceflag and tracefile if enabled.
	 * If not inherited, these were zeroed above.
	 */
	if (p1->p_traceflag&KTRFAC_INHERIT) {
		p2->p_traceflag = p1->p_traceflag;
		if ((p2->p_tracep = p1->p_tracep) != NULL)
			VREF(p2->p_tracep);
	}
#endif
#ifdef SFTRC
	if ((p1->p_flag & (STRC|SFTRC)) == (STRC|SFTRC)) {
		/* Inherit trace flags */
		p2->p_flag |= (p1->p_flag & (STRC|SFTRC|SSTRC));
		if (p1->p_tptr)
			p2->p_tptr = p1->p_tptr;
		else
			p2->p_tptr = p1->p_pptr;
	}
#endif

#if defined(tahoe)
	p2->p_vmspace->p_ckey = p1->p_vmspace->p_ckey; /* XXX move this */
#endif

	vm_fork(p1, p2, isvfork);

	p2->p_stat = SRUN;

	p2->p_ipl = 0;
	p2->p_master_lock = 0;
	p2->p_servers = 0;
	condition_init(&p2->p_condition);
	mutex_init(&p2->p_lock);

	return p2;
}
