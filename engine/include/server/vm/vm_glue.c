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
 * $Log:	vm_glue.c,v $
 * Revision 2.1  92/04/21  17:12:15  rwd
 * BSDSS
 * 
 *
 */

/* 
 * Copyright (c) 1991 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * The Mach Operating System project at Carnegie-Mellon University.
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
 *	@(#)vm_glue.c	7.8 (Berkeley) 5/15/91
 *
 *
 * Copyright (c) 1987, 1990 Carnegie-Mellon University.
 * All rights reserved.
 * 
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND 
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <debug.h>
#include <sysvshm.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/resourcevar.h>
#include <sys/buf.h>
#include <sys/user.h>

#include <uxkern/import_mach.h>

int	avefree = 0;		/* XXX */
unsigned maxdmap = MAXDSIZ;	/* XXX */
int	readbuffers = 0;	/* XXX allow kgdb to read kernel buffer pool */

#if 0
kernacc(addr, len, rw)
	caddr_t addr;
	int len, rw;
{
	boolean_t rv;
	vm_offset_t saddr, eaddr;
	vm_prot_t prot = rw == B_READ ? VM_PROT_READ : VM_PROT_WRITE;

	saddr = trunc_page(addr);
	eaddr = round_page(addr+len-1);
	rv = vm_map_check_protection(kernel_map, saddr, eaddr, prot);
	/*
	 * XXX there are still some things (e.g. the buffer cache) that
	 * are managed behind the VM system's back so even though an
	 * address is accessible in the mind of the VM system, there may
	 * not be physical pages where the VM thinks there is.  This can
	 * lead to bogus allocation of pages in the kernel address space
	 * or worse, inconsistencies at the pmap level.  We only worry
	 * about the buffer cache for now.
	 */
	if (!readbuffers && rv && (eaddr > (vm_offset_t)buffers &&
		   saddr < (vm_offset_t)buffers + MAXBSIZE * nbuf))
		rv = FALSE;
	return(rv == TRUE);
}

useracc(addr, len, rw)
	caddr_t addr;
	int len, rw;
{
	boolean_t rv;
	vm_prot_t prot = rw == B_READ ? VM_PROT_READ : VM_PROT_WRITE;

	rv = vm_map_check_protection(&curproc->p_vmspace->vm_map,
	    trunc_page(addr), round_page(addr+len-1), prot);
	return(rv == TRUE);
}

vslock(addr, len)
	caddr_t	addr;
	u_int	len;
{
	vm_map_pageable(&curproc->p_vmspace->vm_map, trunc_page(addr),
			round_page(addr+len-1), FALSE);
}

vsunlock(addr, len, dirtied)
	caddr_t	addr;
	u_int	len;
	int dirtied;
{
#ifdef	lint
	dirtied++;
#endif	lint
	vm_map_pageable(&curproc->p_vmspace->vm_map, trunc_page(addr),
			round_page(addr+len-1), TRUE);
}

/*
 * Implement fork's actions on an address space.
 * Here we arrange for the address space to be copied or referenced,
 * allocate a user struct (pcb and kernel stack), then call the
 * machine-dependent layer to fill those in and make the new process
 * ready to run.
 * NOTE: the kernel stack may be at a different location in the child
 * process, and thus addresses of automatic variables may be invalid
 * after cpu_fork returns in the child process.  We do nothing here
 * after cpu_fork returns.
 */
#endif 0
vm_fork(p1, p2, isvfork)
	register struct proc *p1, *p2;
	int isvfork;
{
	register struct user *up;
	vm_offset_t addr;

	p2->p_vmspace = (struct vmspace *)malloc(sizeof(struct vmspace));
	bcopy(p1->p_vmspace, p2->p_vmspace, sizeof(struct vmspace));
	p2->p_vmspace->vm_refcnt = 1;
	p2->p_vmspace->vm_shm = 0;

#if SYSVSHM
	if (p1->p_vmspace->vm_shm)
		shmfork(p1, p2, isvfork);
#endif

	/*
	 * Allocate a wired-down (for now) pcb and kernel stack for the process
	 */
	(void) vm_allocate(mach_task_self(), &addr, ctob(UPAGES), TRUE);
	up = (struct user *)addr;
	p2->p_addr = up;

	/*
	 * p_stats and p_sigacts currently point at fields
	 * in the user struct but not at &u, instead at p_addr.
	 * Copy p_sigacts and parts of p_stats; zero the rest
	 * of p_stats (statistics).
	 */
	p2->p_stats = &up->u_stats;
	p2->p_sigacts = &up->u_sigacts;
	up->u_sigacts = *p1->p_sigacts;
	bzero(&up->u_stats.pstat_startzero,
	    (unsigned) ((caddr_t)&up->u_stats.pstat_endzero -
	    (caddr_t)&up->u_stats.pstat_startzero));
	bcopy(&p1->p_stats->pstat_startcopy, &up->u_stats.pstat_startcopy,
	    ((caddr_t)&up->u_stats.pstat_endcopy -
	     (caddr_t)&up->u_stats.pstat_startcopy));

	return 0;
}

vm_exit(p)
    register struct proc *p;
{
    free(p->p_vmspace);
    vm_deallocate(mach_task_self(), p->p_addr, ctob(UPAGES));
}

#if 0
/*
 * Set default limits for VM system.
 * Called for proc 0, and then inherited by all others.
 */
vm_init_limits(p)
	register struct proc *p;
{

	/*
	 * Set up the initial limits on process VM.
	 * Set the maximum resident set size to be all
	 * of (reasonably) available memory.  This causes
	 * any single, large process to start random page
	 * replacement once it fills memory.
	 */
        p->p_rlimit[RLIMIT_STACK].rlim_cur = DFLSSIZ;
        p->p_rlimit[RLIMIT_STACK].rlim_max = MAXSSIZ;
        p->p_rlimit[RLIMIT_DATA].rlim_cur = DFLDSIZ;
        p->p_rlimit[RLIMIT_DATA].rlim_max = MAXDSIZ;
	p->p_rlimit[RLIMIT_RSS].rlim_cur = p->p_rlimit[RLIMIT_RSS].rlim_max =
		ptoa(vm_page_free_count);
}

#include "../vm/vm_pageout.h"

#if DEBUG
int	enableswap = 1;
int	swapdebug = 0;
#define	SDB_FOLLOW	1
#define SDB_SWAPIN	2
#define SDB_SWAPOUT	4
#endif

/*
 * Brutally simple:
 *	1. Attempt to swapin every swaped-out, runnable process in
 *	   order of priority.
 *	2. If not enough memory, wake the pageout daemon and let it
 *	   clear some space.
 */
sched()
{
	register struct proc *p;
	register int pri;
	struct proc *pp;
	int ppri;
	vm_offset_t addr;
	vm_size_t size;

loop:
#if DEBUG
	if (!enableswap) {
		pp = NULL;
		goto noswap;
	}
#endif
	pp = NULL;
	ppri = INT_MIN;
	for (p = allproc; p != NULL; p = p->p_nxt)
		if (p->p_stat == SRUN && (p->p_flag & SLOAD) == 0) {
			pri = p->p_time + p->p_slptime - p->p_nice * 8;
			if (pri > ppri) {
				pp = p;
				ppri = pri;
			}
		}
#if DEBUG
	if (swapdebug & SDB_FOLLOW)
		printf("sched: running, procp %x pri %d\n", pp, ppri);
noswap:
#endif
	/*
	 * Nothing to do, back to sleep
	 */
	if ((p = pp) == NULL) {
		sleep((caddr_t)&proc0, PVM);
		goto loop;
	}

	/*
	 * We would like to bring someone in.
	 * This part is really bogus cuz we could deadlock on memory
	 * despite our feeble check.
	 */
	size = round_page(ctob(UPAGES));
	addr = (vm_offset_t) p->p_addr;
	if (vm_page_free_count > atop(size)) {
#if DEBUG
		if (swapdebug & SDB_SWAPIN)
			printf("swapin: pid %d(%s)@%x, pri %d free %d\n",
			       p->p_pid, p->p_comm, p->p_addr,
			       ppri, vm_page_free_count);
#endif
		vm_map_pageable(kernel_map, addr, addr+size, FALSE);
		(void) splclock();
		if (p->p_stat == SRUN)
			setrq(p);
		p->p_flag |= SLOAD;
		(void) spl0();
		p->p_time = 0;
		goto loop;
	}
	/*
	 * Not enough memory, jab the pageout daemon and wait til the
	 * coast is clear.
	 */
#if DEBUG
	if (swapdebug & SDB_FOLLOW)
		printf("sched: no room for pid %d(%s), free %d\n",
		       p->p_pid, p->p_comm, vm_page_free_count);
#endif
	(void) splhigh();
	VM_WAIT;
	(void) spl0();
#if DEBUG
	if (swapdebug & SDB_FOLLOW)
		printf("sched: room again, free %d\n", vm_page_free_count);
#endif
	goto loop;
}

#define	swappable(p) \
	(((p)->p_flag & (SSYS|SLOAD|SKEEP|SWEXIT|SPHYSIO)) == SLOAD)

/*
 * Swapout is driven by the pageout daemon.  Very simple, we find eligible
 * procs and unwire their u-areas.  We try to always "swap" at least one
 * process in case we need the room for a swapin.
 * If any procs have been sleeping/stopped for at least maxslp seconds,
 * they are swapped.  Else, we swap the longest-sleeping or stopped process,
 * if any, otherwise the longest-resident process.
 */
swapout_threads()
{
	register struct proc *p;
	struct proc *outp, *outp2;
	int outpri, outpri2;
	int didswap = 0;
	extern int maxslp;

#if DEBUG
	if (!enableswap)
		return;
#endif
	outp = outp2 = NULL;
	outpri = outpri2 = 0;
	for (p = allproc; p != NULL; p = p->p_nxt) {
		if (!swappable(p))
			continue;
		switch (p->p_stat) {
		case SRUN:
			if (p->p_time > outpri2) {
				outp2 = p;
				outpri2 = p->p_time;
			}
			continue;
			
		case SSLEEP:
		case SSTOP:
			if (p->p_slptime > maxslp) {
				swapout(p);
				didswap++;
			} else if (p->p_slptime > outpri) {
				outp = p;
				outpri = p->p_slptime;
			}
			continue;
		}
	}
	/*
	 * If we didn't get rid of any real duds, toss out the next most
	 * likely sleeping/stopped or running candidate.  We only do this
	 * if we are real low on memory since we don't gain much by doing
	 * it (UPAGES pages).
	 */
	if (didswap == 0 &&
	    vm_page_free_count <= atop(round_page(ctob(UPAGES)))) {
		if ((p = outp) == 0)
			p = outp2;
#if DEBUG
		if (swapdebug & SDB_SWAPOUT)
			printf("swapout_threads: no duds, try procp %x\n", p);
#endif
		if (p)
			swapout(p);
	}
}
/*
 * DEBUG stuff
 */

int indent = 0;

/*ARGSUSED2*/
iprintf(a, b, c, d, e, f, g, h)
	char *a;
{
	register int i;

	i = indent;
	while (i >= 8) {
		printf("\t");
		i -= 8;
	}
	for (; i > 0; --i)
		printf(" ");
	printf(a, b, c, d, e, f, g, h);
}
#endif 0
