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
 * $Log:	kern_resource.c,v $
 * Revision 2.1  92/04/21  17:12:46  rwd
 * BSDSS
 * 
 *
 */

/*-
 * Copyright (c) 1982, 1986, 1991 The Regents of the University of California.
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
 *	@(#)kern_resource.c	7.13 (Berkeley) 5/9/91
 */

#include <sys/param.h>
#include <sys/resourcevar.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/synch.h>
#include <sys/syscall.h>

#include <uxkern/import_mach.h>

/*
 * Resource controls and accounting.
 */

getpriority(curp, uap, retval)
	struct proc *curp;
	register struct args {
		int	which;
		int	who;
	} *uap;
	int *retval;
{
	register struct proc *p;
	register int low = PRIO_MAX + 1;

	switch (uap->which) {

	case PRIO_PROCESS:
		if (uap->who == 0)
			p = curp;
		else
			p = pfind(uap->who);
		if (p == 0)
			break;
		low = p->p_nice;
		break;

	case PRIO_PGRP: {
		register struct pgrp *pg;

		if (uap->who == 0)
			pg = curp->p_pgrp;
		else if ((pg = pgfind(uap->who)) == NULL)
			break;
		for (p = pg->pg_mem; p != NULL; p = p->p_pgrpnxt) {
			if (p->p_nice < low)
				low = p->p_nice;
		}
		break;
	}

	case PRIO_USER:
		if (uap->who == 0)
			uap->who = curp->p_ucred->cr_uid;
		for (p = allproc; p != NULL; p = p->p_nxt) {
			if (p->p_ucred->cr_uid == uap->who &&
			    p->p_nice < low)
				low = p->p_nice;
		}
		break;

	default:
		return (EINVAL);
	}
	if (low == PRIO_MAX + 1)
		return (ESRCH);
	*retval = low;
	return (0);
}

/* ARGSUSED */
setpriority(curp, uap, retval)
	struct proc *curp;
	register struct args {
		int	which;
		int	who;
		int	prio;
	} *uap;
	int *retval;
{
	register struct proc *p;
	int found = 0, error = 0;

	switch (uap->which) {

	case PRIO_PROCESS:
		if (uap->who == 0)
			p = curp;
		else
			p = pfind(uap->who);
		if (p == 0)
			break;
		error = donice(curp, p, uap->prio);
		found++;
		break;

	case PRIO_PGRP: {
		register struct pgrp *pg;
		 
		if (uap->who == 0)
			pg = curp->p_pgrp;
		else if ((pg = pgfind(uap->who)) == NULL)
			break;
		for (p = pg->pg_mem; p != NULL; p = p->p_pgrpnxt) {
			error = donice(curp, p, uap->prio);
			found++;
		}
		break;
	}

	case PRIO_USER:
		if (uap->who == 0)
			uap->who = curp->p_ucred->cr_uid;
		for (p = allproc; p != NULL; p = p->p_nxt)
			if (p->p_ucred->cr_uid == uap->who) {
				error = donice(curp, p, uap->prio);
				found++;
			}
		break;

	default:
		return (EINVAL);
	}
	if (found == 0)
		return (ESRCH);
	return (0);
}

donice(curp, chgp, n)
	register struct proc *curp, *chgp;
	register int n;
{
	register struct pcred *pcred = curp->p_cred;

	if (pcred->pc_ucred->cr_uid && pcred->p_ruid &&
	    pcred->pc_ucred->cr_uid != chgp->p_ucred->cr_uid &&
	    pcred->p_ruid != chgp->p_ucred->cr_uid)
		return (EPERM);
	if (n > PRIO_MAX)
		n = PRIO_MAX;
	if (n < PRIO_MIN)
		n = PRIO_MIN;
	if (n < chgp->p_nice && suser(pcred->pc_ucred, &curp->p_acflag))
		return (EACCES);
	chgp->p_nice = n;
	{
	    thread_array_t	thread_list;
	    unsigned int	thread_count;
	    register int	i, newpri;

	    newpri = 12 + n/2;

	    (void) task_threads(chgp->p_task, &thread_list, &thread_count);
	    for (i = 0; i < thread_count; i++)
		if (MACH_PORT_VALID(thread_list[i])) {
		    set_thread_priority(thread_list[i], newpri);
		    (void) mach_port_deallocate(mach_task_self(),
						thread_list[i]);
		}
	    (void) vm_deallocate(mach_task_self(),
				 (vm_offset_t)thread_list,
				 (vm_size_t)thread_count * sizeof(thread_t));
	}
/*	(void) setpri(chgp);*/
	return (0);
}

/* ARGSUSED */
setrlimit(p, uap, retval)
	struct proc *p;
	register struct args {
		u_int	which;
		struct	rlimit *lim;
	} *uap;
	int *retval;
{
	struct rlimit alim;
	register struct rlimit *alimp;
	extern unsigned maxdmap;
	int error;

	if (uap->which >= RLIM_NLIMITS)
		return (EINVAL);
	alimp = &p->p_rlimit[uap->which];
	if (error =
	    copyin((caddr_t)uap->lim, (caddr_t)&alim, sizeof (struct rlimit)))
		return (error);
	if (alim.rlim_cur > alimp->rlim_max || alim.rlim_max > alimp->rlim_max)
		if (error = suser(p->p_ucred, &p->p_acflag))
			return (error);
	if (p->p_limit->p_refcnt > 1 &&
	    (p->p_limit->p_lflags & PL_SHAREMOD) == 0) {
		p->p_limit->p_refcnt--;
		p->p_limit = limcopy(p->p_limit);
	}

	switch (uap->which) {

	case RLIMIT_DATA:
		if (alim.rlim_cur > maxdmap)
			alim.rlim_cur = maxdmap;
		if (alim.rlim_max > maxdmap)
			alim.rlim_max = maxdmap;
		break;

	case RLIMIT_STACK:
		if (alim.rlim_cur > maxdmap)
			alim.rlim_cur = maxdmap;
		if (alim.rlim_max > maxdmap)
			alim.rlim_max = maxdmap;
		/*
		 * Stack is allocated to the max at exec time with only
		 * "rlim_cur" bytes accessible.  If stack limit is going
		 * up make more accessible, if going down make inaccessible.
		 */
		if (alim.rlim_cur != alimp->rlim_cur) {
			vm_offset_t addr;
			vm_size_t size;
			vm_prot_t prot;

			if (alim.rlim_cur > alimp->rlim_cur) {
				prot = VM_PROT_ALL;
				size = alim.rlim_cur - alimp->rlim_cur;
				addr = USRSTACK - alim.rlim_cur;
			} else {
				prot = VM_PROT_NONE;
				size = alimp->rlim_cur - alim.rlim_cur;
				addr = USRSTACK - alimp->rlim_cur;
			}
			addr = trunc_page(addr);
			size = round_page(size);
/*			(void) vm_map_protect(&p->p_task,
					      addr, addr+size, prot, FALSE);*/
		}
		break;
	}
	p->p_rlimit[uap->which] = alim;
	return (0);
}

/* ARGSUSED */
getrlimit(p, uap, retval)
	struct proc *p;
	register struct args {
		u_int	which;
		struct	rlimit *rlp;
	} *uap;
	int *retval;
{

	if (uap->which >= RLIM_NLIMITS)
		return (EINVAL);
	return (copyout((caddr_t)&p->p_rlimit[uap->which], (caddr_t)uap->rlp,
	    sizeof (struct rlimit)));
}

/* ARGSUSED */
getrusage(p, uap, retval)
	register struct proc *p;
	register struct args {
		int	who;
		struct	rusage *rusage;
	} *uap;
	int *retval;
{
    panic("get rusage called");
}

/* ARGSUSED */
int
bsd_getrusage(proc_port, interrupt, which, rusage)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		which;
	struct rusage	*rusage;	/* OUT */
{
        register int error;
	register struct rusage *rup;
	register struct proc *p;

	error = start_server_op(proc_port, SYS_getrusage);
	if (error)
	    return (error);

	p = (struct proc *)cthread_data(cthread_self());

	switch (which) {

	case RUSAGE_SELF: {
		int s;

		rup = &p->p_stats->p_ru;
		rup->ru_stime = p->p_stime;
		rup->ru_utime = p->p_utime;
		break;
	}

	case RUSAGE_CHILDREN:
		rup = &p->p_stats->p_cru;
		break;

	default:
		return (end_server_op(p, EINVAL, interrupt));
	}
	*rusage = *rup;
	return (end_server_op(p, 0, interrupt));
}

ruadd(ru, ru2)
	register struct rusage *ru, *ru2;
{
	register long *ip, *ip2;
	register int i;

	timevaladd(&ru->ru_utime, &ru2->ru_utime);
	timevaladd(&ru->ru_stime, &ru2->ru_stime);
	if (ru->ru_maxrss < ru2->ru_maxrss)
		ru->ru_maxrss = ru2->ru_maxrss;
	ip = &ru->ru_first; ip2 = &ru2->ru_first;
	for (i = &ru->ru_last - &ru->ru_first; i > 0; i--)
		*ip++ += *ip2++;
}

/*
 * Make a copy of the plimit structure.
 * We share these structures copy-on-write after fork,
 * and copy when a limit is changed.
 */
struct plimit *
limcopy(lim)
	struct plimit *lim;
{
	register struct plimit *copy;

	MALLOC(copy, struct plimit *, sizeof(struct plimit),
	    M_SUBPROC, M_WAITOK);
	bcopy(lim->pl_rlimit, copy->pl_rlimit,
	    sizeof(struct rlimit) * RLIM_NLIMITS);
	copy->p_lflags = 0;
	copy->p_refcnt = 1;
	return (copy);
}
