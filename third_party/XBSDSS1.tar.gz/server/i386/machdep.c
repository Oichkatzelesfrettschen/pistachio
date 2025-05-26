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
 * $Log:	machdep.c,v $
 * Revision 2.2  92/05/25  14:42:13  rwd
 * 	Added ptrace support from UX and additional to support
 * 	     386BSD gdb.
 * 	[92/05/21            rwd]
 * 
 * Revision 2.1  92/04/21  17:19:12  rwd
 * BSDSS
 * 
 *
 */

/*
 *	Machine-dependent routines evicted from rest of BSD
 *	files.
 */

#include <i386/reg.h>
#include <i386/eflags.h>

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
    struct i386_thread_state	ts;
    unsigned int			count;
    register struct proc *p = (struct proc *)cthread_data(cthread_self());
    kern_return_t ret;

    /*
     * Read the registers first, to get the correct
     * segments.
     */
    count = i386_THREAD_STATE_COUNT;
    if ((ret=thread_get_state(p->p_thread,
			      i386_THREAD_STATE,
			      (thread_state_t)&ts,
			      &count)) != KERN_SUCCESS)
	panic("set_emulator_state",ret);
    ts.eip = (int)entry[0];
    ts.uesp = (int)arg_addr;
    ts.efl = EFL_USER_SET;

    if ((ret=thread_set_state(p->p_thread,
			      i386_THREAD_STATE,
			      (thread_state_t)&ts,
			      i386_THREAD_STATE_COUNT)) != KERN_SUCCESS)
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
	vm_offset_t	stack_addr;
	vm_size_t	stack_size;
	off_t		offset = 0;
	int		error;

	struct i386_thread_state	ts;
	unsigned int		count;

	/*
	 * Get the registers for the thread.
	 */

	count = i386_THREAD_STATE_COUNT;
	(void) thread_get_state(p->p_thread,
				i386_THREAD_STATE,
				(thread_state_t)&ts,
				&count);

	/*
	 * Fake the stack segment size from the current
	 * stack pointer value.
	 */
	stack_addr = trunc_page(ts.uesp);
	stack_size = round_page(USRSTACK - stack_addr);
	p->p_utask.uu_ssize = btoc(stack_size);

	/*
	 * Build a fake U-area for the bottom of the
	 * 'kernel-stack' in the core file.
	 */
	{
	    struct user fake_uarea;

	    fake_u(&fake_uarea, p, p->p_thread);

	    error = RDWRI(UIO_WRITE, ip,
		(caddr_t)&fake_uarea, sizeof(struct user),
		offset, UIO_SYSSPACE, (int *)0);
	}
	if (error)
	    goto out;

	/*
	 * Build a fake register save area for the top of
	 * the 'kernel-stack' in the core file.
	 */
	{
	    int		regs[21];
#define	LOCR0_OFFSET	21

	    bzero((caddr_t)regs, sizeof(regs));

	    regs[EAX + LOCR0_OFFSET] = ts.eax;
	    regs[EBX + LOCR0_OFFSET] = ts.ebx;
	    regs[ECX + LOCR0_OFFSET] = ts.ecx;
	    regs[EDX + LOCR0_OFFSET] = ts.edx;
	    regs[ESP + LOCR0_OFFSET] = ts.esp;
	    regs[EBP + LOCR0_OFFSET] = ts.ebp;
	    regs[ESI + LOCR0_OFFSET] = ts.esi;
	    regs[EDI + LOCR0_OFFSET] = ts.edi;
	    regs[EIP + LOCR0_OFFSET] = ts.eip;
	    regs[EFL + LOCR0_OFFSET] = ts.efl;
	    regs[UESP+ LOCR0_OFFSET] = ts.uesp;
	    regs[CS  + LOCR0_OFFSET] = ts.cs;
	    regs[SS  + LOCR0_OFFSET] = ts.ss;
	    regs[DS  + LOCR0_OFFSET] = ts.ds;
	    regs[ES  + LOCR0_OFFSET] = ts.es;
	    regs[FS  + LOCR0_OFFSET] = ts.fs;
	    regs[GS  + LOCR0_OFFSET] = ts.gs;

	    offset = ctob(UPAGES);
	    error = RDWRI(UIO_WRITE, ip,
		(caddr_t)regs, (int) sizeof(regs),
		offset - sizeof(regs),
		UIO_SYSSPACE, (int *)0);
	}
	if (error)
	    goto out;

	/*
	 * Write the data segment.
	 */
	error = core_file_write(ip, &offset, p->p_task,
			p->p_utask.uu_data_start,
			(int)ctob(p->p_utask.uu_dsize));
	if (error)
	    goto out;

	/*
	 * Write the stack.
	 */
	error = core_file_write(ip, &offset, p->p_task,
			stack_addr, stack_size);

    out:
	return (error);

#undef	LOCR0_OFFSET
}

#endif 0

struct reg_offsets {
	int	reg_offset;
	int	state_offset;
};
#define	state_off(s)	(int)&(((struct i386_thread_state *)0)->s)
#define	reg_off(r)	sizeof(int)*(r)+ctob(UPAGES)
#define regs(r,s)	{ reg_off(r), state_off(s) }

struct reg_offsets ipcregs[] = {
    	regs(tES, es),
    	regs(tDS, ds),
	regs(tEDI, edi),
	regs(tESI, esi),
	regs(tEBP, ebp),
	regs(tEBX, ebx),
	regs(tEDX, edx),
	regs(tECX, ecx),
	regs(tEAX, eax),
	regs(tEIP, eip),
	regs(tCS, cs),
	regs(tEFLAGS, efl),
	regs(tESP, uesp),
	regs(tSS, ss)
};

#undef	regs
#undef	reg_off
#undef	state_off

int ptrace_get_u_word(p, offset, value)
	struct proc *p;
	register int	offset;
	int	*value;	/* out */
{
	if (offset < 0 || offset > ctob(UPAGES) + NIPCREG*sizeof(int))
	    return (EIO);

	if (offset < sizeof(struct user)) {
	    int *addr;

	    addr = (int *)(((caddr_t)p->p_addr) + offset);
	    if (addr == (int *)&p->p_addr->u_pcb.pcb_flags) {
		*value = -1;
	    } else if (addr == (int *)&p->p_addr->u_kproc.kp_proc.p_regs) {
		*value = ctob(UPAGES) + 0xFDBFE000; /*XXX old USRSTACK */
	    } else {
		*value = *addr;
	    }
	} else {
	    struct i386_thread_state	thread_state;
	    unsigned int		count;
	    register int		j;

	    count = i386_THREAD_STATE_COUNT;
	    if (thread_get_state(p->p_thread,
				 i386_THREAD_STATE,
				 (thread_state_t)&thread_state,
				 &count) != KERN_SUCCESS)
		return EIO;

	    *value = 0;

	    for (j = 0; j < NIPCREG; j++) {
		if (ipcregs[j].reg_offset == offset) {
		    *value = *(int *)(((caddr_t)&thread_state)
					+ ipcregs[j].state_offset);
		    break;
		}
	    }

	}
	return (0);
}

int ptrace_set_u_word(p, offset, value, old)
	struct proc *p;
	register int offset;
	int	value;
	int	*old;
{
	/*
	 *	Write victim's registers.
	 *	Offsets are into BSD kernel stack, and must
	 *	be faked to match MACH.
	 */
	struct i386_thread_state	thread_state;
	unsigned int	count;
	register int	j;

	count = i386_THREAD_STATE_COUNT;
	if (thread_get_state(p->p_thread,
			     i386_THREAD_STATE,
			     (thread_state_t)&thread_state,
			     &count)!= KERN_SUCCESS)
	    return EIO;

	for (j = 0; j < NIPCREG; j++)
	    if (ipcregs[j].reg_offset == offset)
		break;

	if (j == NIPCREG) {
	    /* wrong register */
	    return (EIO);
	}

	/* thread_set_state handles checking PSL for valid values */
	*(int *)(((caddr_t)&thread_state)
			+ ipcregs[j].state_offset)
		      = value;
	if (thread_set_state(p->p_thread,
			     i386_THREAD_STATE,
			     (thread_state_t)&thread_state,
			     count) != KERN_SUCCESS)
	    return EIO;
	return (0);
}

ptrace_set_trace(p, new_addr, trace_on)
	struct proc *p;
	int	new_addr;
	int	trace_on;
{
	struct i386_thread_state	thread_state;
	unsigned int		count;

	count = i386_THREAD_STATE_COUNT;
	if (thread_get_state(p->p_thread,
			     i386_THREAD_STATE,
			     (thread_state_t)&thread_state,
			     &count) != KERN_SUCCESS)
	    return EIO;

	if (new_addr != 1)
		thread_state.eip = new_addr;

	if (trace_on)
		thread_state.efl   |= EFL_TF;

	if (thread_set_state(p->p_thread,
			     i386_THREAD_STATE,
			     (thread_state_t)&thread_state,
			     count) != KERN_SUCCESS)
	    return EIO;
	return 0;
}

ptrace_get_regs(p, regs)
    struct proc *p;
    int regs[NIPCREG];
{
	struct i386_thread_state	thread_state;
	unsigned int		count;
	int j;

	count = i386_THREAD_STATE_COUNT;
	if (thread_get_state(p->p_thread,
			     i386_THREAD_STATE,
			     (thread_state_t)&thread_state,
			     &count) != KERN_SUCCESS)
	    return EIO;

	for (j = 0; j < NIPCREG; j++) {
	    regs[ipcregs[j].reg_offset] = *(int *)(((caddr_t)&thread_state)
						   + ipcregs[j].state_offset);
	}
}

ptrace_set_regs(p, regs)
    struct proc *p;
    int regs[NIPCREG];
{
	struct i386_thread_state	thread_state;
	unsigned int		count;
	int j;

	for (j = 0; j < NIPCREG; j++) {
	    *(int *)(((caddr_t)&thread_state) + ipcregs[j].state_offset) =
		 regs[ipcregs[j].reg_offset];
	}

	/* thread_set_state handles checking PSL for valid values */
	count = i386_THREAD_STATE_COUNT;
	if (thread_set_state(p->p_thread,
			     i386_THREAD_STATE,
			     (thread_state_t)&thread_state,
			     count) != KERN_SUCCESS)
	    return EIO;
	return 0;
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
        int sig, code, mask;
	void(*sig_hdlr)();
{
	struct sigframe {
	    int	sf_signum;
	    int	sf_code;
	    struct	sigcontext *sf_scp;
	    sig_t	sf_handler;
	    int	sf_eax;	
	    int	sf_edx;	
	    int	sf_ecx;	
	    struct	sigcontext sf_sc;
	} *fp;
	int	oonstack;

	struct i386_thread_state regs;
	unsigned int		reg_size;

	vm_offset_t	user_start_addr, user_end_addr, kern_addr;
	vm_size_t	user_size;

	register struct sigframe   *kfp;
	register struct sigacts *ps = p->p_sigacts;

	/*
	 * Get the registers for the thread.
	 */
	reg_size = i386_THREAD_STATE_COUNT;
	(void) thread_get_state(thread, i386_THREAD_STATE,
				(thread_state_t)&regs, &reg_size);

	oonstack = ps->ps_onstack;

	if (!oonstack && (ps->ps_sigonstack & sigmask(sig))) {
	    fp = (struct sigframe *)ps->ps_sigstack.ss_sp - 1;
	    ps->ps_sigstack.ss_onstack = 1;
	}
	else {
	    fp = (struct sigframe *)regs.uesp - 1;
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

	kfp->sf_eax = regs.eax;
	kfp->sf_edx = regs.edx;
	kfp->sf_ecx = regs.ecx;

	/*
	 * Build the signal context to be used by sigreturn.
	 */
	kfp->sf_sc.sc_onstack = oonstack;
	kfp->sf_sc.sc_mask = mask;

	kfp->sf_sc.sc_fp = regs.ebp;
	kfp->sf_sc.sc_pc = regs.eip;
	kfp->sf_sc.sc_ps = regs.efl;
	kfp->sf_sc.sc_sp = regs.uesp;

	/*
	 * Set up the new stack and handler address.
	 */
	regs.uesp = (int)fp;
	regs.eip = USRSTACK-TRAMPOLINE_MAX_SIZE;

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
	(void) thread_set_state(thread, i386_THREAD_STATE,
				(thread_state_t)&regs, reg_size);

	(void)vm_deallocate(mach_task_self(), kern_addr, user_size);
	return;

    error:
	/*
	 * Process has trashed its stack; kill it and core dump it.
	 * The idea is to imitate the default action for a SIGILL.
	 */
	exit(p, SIGILL);
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
	struct i386_thread_state *regs = (struct i386_thread_state *)new_state;

	if (new_state_count != i386_THREAD_STATE_COUNT)
	    return (FALSE);

	regs->eax = parent_pid;
	regs->edx = rc;

	(void) thread_set_state(child_thread, i386_THREAD_STATE,
				new_state, new_state_count);
	return (TRUE);
}
