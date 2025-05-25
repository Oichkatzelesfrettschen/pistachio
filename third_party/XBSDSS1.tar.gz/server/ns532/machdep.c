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
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created from i386/machdep.c.
 *
 * $Log:$
 */
/*
 *	Machine-dependent routines evicted from rest of BSD
 *	files.
 */

#include <ns532/psl.h>

#include <sys/param.h>

#include <sys/proc.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/exec.h>
#include <sys/systm.h>		/* boothowto */
#include <sys/parallel.h>
#include <sys/syscall.h>

#include <ufs/quota.h>
#include <ufs/inode.h>

#include <uxkern/import_mach.h>

set_arg_addr(arg_size,
	     arg_start_p,
	     arg_size_p)
	u_int		arg_size;
	vm_offset_t	*arg_start_p;
	vm_size_t	*arg_size_p;
{
	/*
	 * Round arg size to fullwords
	 */
	arg_size = (arg_size + NBPW-1) & ~(NBPW - 1);

	/*
	 * Put argument list at top of stack.
	 */
	*arg_start_p = USRSTACK - arg_size;
	*arg_size_p = arg_size;
}

extern boolean_t	first_exec;

set_entry_address(ep, entry, entry_count)
	struct exec	*ep;
	int		*entry;		/* pointer to OUT array */
	unsigned int	*entry_count;	/* out */
{
	entry[0] = ep->a_entry;
	*entry_count = 1;
}

/*
 * Set up the process' registers to start the emulator - duplicate
 * what execve() in the emulator would do.
 */
set_emulator_state(entry, entry_count, arg_addr)
    int entry[];
    int entry_count;
    vm_offset_t	arg_addr;
{
    struct ns532_thread_state		ts;
    unsigned int			count;
    register struct proc *p = (struct proc *)cthread_data(cthread_self());
    kern_return_t ret;

    /*
     * Read the registers first, to get the correct
     * segments.
     */
    count = NS532_THREAD_STATE_COUNT;
    if ((ret=thread_get_state(p->p_thread,
			      NS532_THREAD_STATE,
			      (thread_state_t)&ts,
			      &count)) != KERN_SUCCESS)
	panic("set_emulator_state",ret);
    ts.pc = (int)entry[0];
    ts.sp = (int)arg_addr;
    ts.psr = PSR_U;
    ts.sb = 0;
    ts.fp = 0;

    if ((ret=thread_set_state(p->p_thread,
			      NS532_THREAD_STATE,
			      (thread_state_t)&ts,
			      NS532_THREAD_STATE_COUNT)) != KERN_SUCCESS)
	panic("set_emulator_state",ret);
}

/*
 * Do weird things to the first process' address space before loading
 * the emulator.
 */
void
emul_init_process(p)
	struct proc	*p;
{
	/*
	 * Clear out the entire address space.
	 */
	(void) vm_deallocate(p->p_task,
			VM_MIN_ADDRESS,
			(vm_size_t)(VM_MAX_ADDRESS - VM_MIN_ADDRESS));

}


#if 0
machine_core(p, ip)
	register struct proc	*p;
	register struct inode	*ip;
{
}
#endif 0

/* 
 * No need for ptrace on the pc532. The only debugger we have,
 * the Mach 3 version of gdb, doesn't use ptrace anyways.
 * Besides ptrace sucks.
 */

int ptrace_get_u_word(p, offset, value)
	struct proc *p;
	register int	offset;
	int	*value;	/* out */
{
	return ENOSYS;
}

int ptrace_set_u_word(p, offset, value, old)
	struct proc *p;
	register int offset;
	int	value;
	int	*old;
{
	return ENOSYS;
}

ptrace_set_trace(p, new_addr, trace_on)
	struct proc *p;
	int	new_addr;
	int	trace_on;
{
	return ENOSYS;
}

ptrace_get_regs(p, regs)
    struct proc *p;
    int regs[NIPCREG];
{
	return ENOSYS;
}

ptrace_set_regs(p, regs)
    struct proc *p;
    int regs[NIPCREG];
{
	return ENOSYS;
}

/*
 * Send an interrupt to process.
 *
 * Stack is set up to allow sigcode stored in u. to call routine,
 * followed by chmk to sigreturn routine below.  After sigreturn
 * resets the signal mask, the stack, the frame pointer, and the
 * argument pointer, it returns to the user-specified pc/psl.
 */
void
sendsig(p, thread, sig_hdlr, sig, code, mask)
	struct proc	*p;
	thread_t	thread;
        void (*sig_hdlr)();
	int		sig, code, mask;
{
	struct sigframe {
	    int	sf_signum;
	    int	sf_code;
	    struct	sigcontext *sf_scp;
	    sig_t	sf_handler;
	    int	sf_r2;	
	    int	sf_r1;	
	    int	sf_r0;	
	    struct	sigcontext sf_sc;
	} *fp;
	int	oonstack;

	struct ns532_thread_state regs;
	unsigned int		reg_size;

	vm_offset_t	user_start_addr, user_end_addr, kern_addr;
	vm_size_t	user_size;

	register struct sigframe   *kfp;
	register struct sigacts *ps = p->p_sigacts;

	/*
	 * Get the registers for the thread.
	 */
	reg_size = NS532_THREAD_STATE_COUNT;
	(void) thread_get_state(thread, NS532_THREAD_STATE,
				(thread_state_t)&regs, &reg_size);

	oonstack = ps->ps_onstack;

	if (!oonstack && (ps->ps_sigonstack & sigmask(sig))) {
	    fp = (struct sigframe *)ps->ps_sigstack.ss_sp - 1;
	    ps->ps_sigstack.ss_onstack = 1;
	}
	else {
	    fp = (struct sigframe *)regs.sp - 1;
	}

	/*
	 * Copy the signal frame from the user into the kernel,
	 * to modify it.
	 */
	user_start_addr = trunc_page((vm_offset_t)fp);
	user_end_addr   = round_page((vm_offset_t)fp
					+ sizeof(*fp));
	user_size = user_end_addr - user_start_addr;

	if (vm_read(p->p_task, user_start_addr, user_size,
			&kern_addr, &user_size) != KERN_SUCCESS)
	    goto error;

	kfp = (struct sigframe *)((vm_offset_t)fp - user_start_addr
					+ kern_addr);

	/*
	 * Build the argument list for the signal handler.
	 */
	kfp->sf_signum = sig;
	kfp->sf_code = code;
	kfp->sf_scp = &fp->sf_sc;
	kfp->sf_handler = sig_hdlr;

	kfp->sf_r0 = regs.r0;
	kfp->sf_r1 = regs.r1;
	kfp->sf_r2 = regs.r2;

	/*
	 * Build the signal context to be used by sigreturn.
	 */
	kfp->sf_sc.sc_onstack = oonstack;
	kfp->sf_sc.sc_mask = mask;

	kfp->sf_sc.sc_fp = regs.fp;
	kfp->sf_sc.sc_pc = regs.pc;
	kfp->sf_sc.sc_ps = regs.psr;
	kfp->sf_sc.sc_sp = regs.sp;

	/* XXX be compatible with libc, the emulator, and signal.h CHECK */

	/*
	 * Set up the new stack and handler address.
	 */
	regs.sp = (int)fp;
	regs.pc = USRSTACK-TRAMPOLINE_MAX_SIZE;

	/*
	 * Write signal frame and context back to user.
	 */
	if (vm_write(p->p_task, user_start_addr, kern_addr, user_size)
		!= KERN_SUCCESS) {
		(void)vm_deallocate(mach_task_self(), kern_addr, user_size);
		goto error;
	}

	/*
	 * Write changed registers back to thread.
	 */
	(void) thread_set_state(thread, NS532_THREAD_STATE,
				(thread_state_t)&regs, reg_size);

	(void)vm_deallocate(mach_task_self(), kern_addr, user_size);
	return;

    error:
	/*
	 * Process has trashed its stack; kill it and core dump it.
	 * The idea is to imitate the default action for a SIGILL.
	 */
	do_exit(p, SIGILL);
}

sigreturn()
{
    panic("sigreturn - MiG interface");
}
osigcleanup()
{
    panic("osigcleanup - MiG interface");
}
	
/*
 * Clone the parent's registers into the child thread for fork.
 */
boolean_t
thread_dup(child_thread, new_state, new_state_count, parent_pid, rc)
	thread_t	child_thread;
	thread_state_t	new_state;
	unsigned int	new_state_count;
	int		parent_pid, rc;
{
	struct ns532_thread_state *regs = (struct ns532_thread_state *)new_state;

	if (new_state_count != NS532_THREAD_STATE_COUNT)
	    return (FALSE);

	regs->r0 = parent_pid;
	regs->r1 = rc;

	(void) thread_set_state(child_thread, NS532_THREAD_STATE,
				new_state, new_state_count);
	return (TRUE);
}
