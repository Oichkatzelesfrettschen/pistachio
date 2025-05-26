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
 * $Log:	sys_process.c,v $
 * Revision 2.2  92/05/25  14:44:13  rwd
 * 	Picked up version from net and mangled it.
 * 	[92/05/20            rwd]
 * 
 * Revision 2.1  92/04/21  17:13:31  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *	from: @(#)sys_process.c	7.22 (Berkeley) 5/11/91
 */

#include <uxkern/import_mach.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/seg.h>
#include <sys/buf.h>
#include <sys/ptrace.h>
#include <sys/parallel.h>
#include <sys/synch.h>

#include <machine/reg.h>
#include <machine/psl.h>

#include <sys/user.h>

/*
 * NOTES.
 *
 * The following ptrace calls have been defined in addition to
 * the standard ones found in original <sys/ptrace.h>:
 *
 * PT_ATTACH		-	attach to running process
 * PT_DETACH		-	detach from running process
 * PT_SYSCALL		-	trace system calls
 * PT_GETREG		-	get register file
 * PT_SETREG		-	set register file
 * PT_BREAD_[IDU]	-	block read from process (not yet implemented)
 * PT_BWRITE_[IDU]	-	block write	"		"
 * PT_INHERIT		-	make forked processes inherit trace flags
 *
 */

/* Define to prevent extraneous clutter in source */
#ifndef SSTRC
#define	SSTRC	0
#endif
#ifndef SFTRC
#define	SFTRC	0
#endif

/*
 * Process debugging system call.
 */
ptrace(curp, uap, retval)
	struct proc *curp;
	register struct args {
		int	req;
		int	pid;
		int	*addr;
		int	data;
	} *uap;
	int *retval;
{
    struct proc *p;
    int s, error = 0;
    vm_offset_t	start_addr, end_addr,
    kern_addr, offset;
    vm_size_t	size;
    unsigned	old_data;
    int	regs[NIPCREG];		/* PT_[GS]ETREG */
    
    *retval = 0;
    if (uap->req == PT_TRACE_ME) {
	curp->p_flag |= STRC;
	/*p->p_tptr = p->p_pptr; * What shall we do here ? */
	return 0;
    }
    if ((p = pfind(uap->pid)) == NULL) {
	return ESRCH;
    }
    
    if (
#ifdef PT_ATTACH
	uap->req != PT_ATTACH &&
#endif
	((p->p_flag & STRC) == 0 ||
	 (p->p_tptr && curp != p->p_tptr) ||
	 (!p->p_tptr && curp != p->p_pptr)))
	return ESRCH;
    
    switch (uap->req) {
      case PT_ATTACH:
	if (curp->p_ucred->cr_uid != 0 &&
	    (curp->p_ucred->cr_uid != p->p_ucred->cr_uid ||
	     curp->p_ucred->cr_uid != p->p_cred->p_svuid))
	    return EACCES;
	
	p->p_tptr = curp;
	p->p_flag |= STRC;
	psignal(p, SIGTRAP);
	return 0;
	
      case PT_DETACH:
	if ((unsigned)uap->data >= NSIG)
	    return EINVAL;
	p->p_flag &= ~(STRC|SSTRC|SFTRC);
	p->p_tptr = NULL;
	psignal(p->p_pptr, SIGCHLD);
	wakeup((caddr_t)p->p_pptr);
	s = splhigh();
	if (p->p_stat == SSTOP) {
	    p->p_xstat = uap->data;
	    p->p_stat = SRUN;
	    wakeup((caddr_t)&p->p_stat);
	    thread_resume(p->p_task);
	} else if (uap->data) {
	    psignal(p, uap->data);
	}
	splx(s);
	return 0;
	
#ifdef PT_INHERIT
      case PT_INHERIT:
	if ((p->p_flag & STRC) == 0)
	    return ESRCH;
	p->p_flag |= SFTRC;
	return 0;
#endif
      case PT_READ_I:
      case PT_READ_D:
	/*
	 *	Read victim's memory
	 */
	start_addr = trunc_page((int)uap->addr);
	end_addr = round_page((int)uap->addr + sizeof(int));
	size = end_addr - start_addr;
	
	/*
	 * Unlock master lock to touch user data.
	 */
	if (p->p_master_lock)
	    master_unlock();
	
	if (vm_read(p->p_task, start_addr, size,
		    &kern_addr, &size)
	    != KERN_SUCCESS) {
	    if (p->p_master_lock)
		master_lock();
	    return EIO;
	}
	/*
	 *	Read the data from the copy in the kernel map.
	 *	Use bcopy to avoid alignment restrictions.
	 */
	offset = (vm_offset_t) uap->addr - start_addr;
	bcopy((caddr_t)(kern_addr + offset),
	      (caddr_t)retval,
	      sizeof(int));
	
	/*
	 *	Discard the kernel's copy.
	 */
	(void)vm_deallocate(mach_task_self(), kern_addr, size);
	if (p->p_master_lock)
	    master_lock();
	
	return 0;
	
      case PT_WRITE_I:
      case PT_WRITE_D:
	/*
	 *	Write victim's memory
	 */
	start_addr = trunc_page((int)uap->addr);
	end_addr = round_page((int)uap->addr + sizeof(int));
	size = end_addr - start_addr;
	
	/*
	 *	Allocate some pageable memory in the kernel map,
	 *	and copy the victim's memory to it.
	 */
	
	/*
	 * Unlock master lock to touch user data.
	 */
	if (p->p_master_lock)
	    master_unlock();
	
	if (vm_read(p->p_task, start_addr, size,
		    &kern_addr, &size)
	    != KERN_SUCCESS) {
	    if (p->p_master_lock)
		master_lock();
	    return EIO;
	}
	/*
	 *	Change the data in the copy in the kernel map.
	 *	Use bcopy to avoid alignment restrictions.
	 */
	offset = (vm_offset_t)uap->addr - start_addr;
	bcopy((caddr_t)(kern_addr + offset), &old_data, sizeof(int));
	bcopy((caddr_t)&uap->data,
	      (caddr_t)(kern_addr + offset),
	      sizeof(int));
	
	/*
	 *	Copy it back to the victim.
	 */
	if (vm_write(p->p_task, start_addr,
		     kern_addr, size)
	    != KERN_SUCCESS) {
	    /*
	     * Area may not be writable.  Try changing
	     * its protection.
	     */
	    if (vm_protect(p->p_task, start_addr, size,
			   FALSE,
			   VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE)
		!= KERN_SUCCESS)
		error = EIO;
	    else {
		if (vm_write(p->p_task, start_addr,
			     kern_addr, size)
		    != KERN_SUCCESS)
		    error = EIO;
		(void) vm_protect(p->p_task, start_addr, size,
				  FALSE,
				  VM_PROT_READ|VM_PROT_EXECUTE);
	    }
	}
	/*
	 *	Discard the kernel's copy.
	 */
	(void)vm_deallocate(mach_task_self(), kern_addr, size);
	
	if (p->p_master_lock)
	    master_lock();
	
#ifdef	mips
    {
	vm_machine_attribute_val_t flush = MATTR_VAL_CACHE_FLUSH;
	kern_return_t ret;
	ret = vm_machine_attribute(
				   p->p_task, uap->addr, sizeof(int),
				   MATTR_CACHE, &flush);
    }
#endif	mips
	*retval = old_data;
	return error;
	
      case PT_READ_U:
	/*
	 *	Read victim's U-area or registers.
	 *	Offsets are into BSD kernel stack, and must
	 *	be faked to match MACH.
	 */
	if (ptrace_get_u_word(p, uap->addr, retval))
	    error = EIO;
	return error;
	
      case PT_WRITE_U:
	/*
	 *	Write victim's registers.
	 *	Offsets are into BSD kernel stack, and must
	 *	be faked to match MACH.
	 */
	if (ptrace_set_u_word(p, uap->addr, uap->data, retval))
	    error = EIO;
	return error;
	
      case PT_KILL:
	/*
	 *	Force process to exit.
	 */
	return exit(p, 0l);
	
      case PT_STEP:
	/*
	 *	Single step the child.
	 */
	if ((unsigned)uap->data > NSIG)
	    return error;
	
	error = ptrace_set_trace(p, (int)uap->addr, TRUE);
	p->p_cursig = uap->data;
	p->p_stat = SRUN;
	wakeup((caddr_t)&p->p_stat);
	task_resume(p->p_task);
	return error;
	
      case PT_CONTINUE:
	/*
	 *	Continue the child.
	 */
	if ((unsigned)uap->data > NSIG) {
	    return EIO;
	}
	
	if ((int)uap->addr != 1)
	    error = ptrace_set_trace(p, (int)uap->addr, FALSE);
	p->p_cursig = uap->data;
	p->p_stat = SRUN;
	wakeup((caddr_t)&p->p_stat);
	task_resume(p->p_task);
	return error;
	
#ifdef PT_GETREGS
      case PT_SETREGS:
	error = copyin((char *)uap->addr, (char *)regs, sizeof(regs));
	if (error)
	    return EIO;
	ptrace_set_regs(p, regs);
	return error;
      case PT_GETREGS:
	ptrace_get_regs(p, regs);
	error = copyout((char *)regs, (char *)uap->addr, sizeof(regs));
	return error;
#endif
    
      default:
	return EIO;
	break;
    }
}

/*
 * Enable process profiling system call.
 */
/* ARGSUSED */
profil(p, uap, retval)
	struct proc *p;
	register struct args {
		short	*bufbase; /* base of data buffer */
		unsigned bufsize; /* size of data buffer */
		unsigned pcoffset; /* pc offset (for subtraction) */
		unsigned pcscale; /* scaling factor for offset pc */
	} *uap;
	int *retval;
{
	/* I'm glad I installed Gnu grep -- made finding profiling
	 * data structures a lot easier.
	 *
	 * From looking at man pages on various *IX systems, It seems
	 * that this just starts profiling...  From looking at
	 * the kernel code, it looks like all we have to do is
	 * set up the fields of p->p_stats->p_prof ...
	 * these fields correspond directly to the args of this function.
	 *
	 * cgd
	 */

	p->p_stats->p_prof.pr_base = uap->bufbase;
	p->p_stats->p_prof.pr_size = uap->bufsize;
	p->p_stats->p_prof.pr_off = uap->pcoffset;
	p->p_stats->p_prof.pr_scale = uap->pcscale;

	return 0;    /* how can you have an error with simple assignments? */
}
