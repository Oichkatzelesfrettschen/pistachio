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
 * $Log:	emul_machdep.c,v $
 * Revision 2.4  92/07/08  16:16:40  mrt
 * 	Added recasts to remove gcc warnings.
 * 
 * Revision 2.3  92/06/25  17:23:20  mrt
 * 	Changed emul_high_entry to 201.
 * 	[92/06/15            rwd]
 * 
 * 	Still had 63 as nosysent in one error case.  Change to 179.
 * 	[92/06/15            rwd]
 * 
 * Revision 2.2  92/04/22  14:00:56  rwd
 * 	Fix mach.h include.
 * 	[92/04/22            rwd]
 * 
 * Revision 2.1  92/04/21  17:27:25  rwd
 * BSDSS
 * 
 *
 */

/*
 * Take/Return from signals - machine-dependent.
 */
#include <mach.h>
#include <sys/types.h>
#include <sys/param.h>
#define KERNEL
#include <sys/errno.h>
#undef KERNEL
#include <sys/signal.h>
#include <sys/syscall.h>
#include <machine/eflags.h>
#include <machine/param.h>
#include <uxkern/bsd_msg.h>	/* error code definitions */

#include <syscall_table.h>

#ifdef	MAP_UAREA
#include <sys/ushared.h>

extern int shared_enabled;
extern struct ushared_ro *shared_base_ro;
extern struct ushared_rw *shared_base_rw;
#endif	MAP_UAREA

extern mach_port_t	our_bsd_server_port;

vm_offset_t	sigreturnaddr;	/* signal trampoline, set by sigvec */

#define	E_JUSTRETURN	255	/* return without changing registers */

/*
 * User's registers are stored on partially on the user's stack
 * and partially on the emulator's stack.
 */
struct emul_regs_2 {
	int	edx;
	int	ecx;
	int	eax;
	int	eflags;
	int	eip;
};

struct emul_regs {
	int	ebp;
	int	edi;
	int	esi;
	int	ebx;
	struct emul_regs_2 *uesp;
				/* pointer to rest of user registers */
};

void	take_signals();	/* forward */

/*
 * Change system call table for sigvec to point to
 * our own routine.
 */

#if 0
#ifdef	COMPAT_43
int	i386_osigvec();		/* forward */
struct sysent i386_syscall_osigvec =
	{ E_CHANGE_REGS, i386_osigvec };
#endif	/* COMPAT_43 */
int	i386_sigaction(), i386_signal() ;	/* forward */
struct sysent i386_syscall_sigaction =
        { E_CHANGE_REGS, i386_sigaction };
struct sysent i386_syscall_signal = 
	{ E_CHANGE_REGS, i386_signal };
#endif 0

int	emul_low_entry = -9;
int	emul_high_entry = 201;   /* was 181, changed for syscall to mmap_hi_res_clock   */

extern emul_common();

void
emul_setup(task)
	task_t	task;
{
	register int i;
	register kern_return_t	rc;


	for (i = emul_low_entry;
	     i <= emul_high_entry;
	     i++) {
		rc = task_set_emulation(task,
					(vm_address_t) emul_common,
					i);
	}
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-6);
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-33);
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-34);
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-41);
	rc = task_set_emulation(task,
			(vm_address_t) emul_common,
			-52);

	/*
	 * Change sigvec to point to our routine - the signal
	 * trampoline address is passed in VERY strangely.
	 */
#if 0
#ifdef	COMPAT_43
	sysent[SYS_osigvec] = i386_syscall_osigvec;
#endif	/* COMPAT_43 */
	sysent[SYS_signal] = i386_syscall_signal;
	sysent[SYS_sigaction] = i386_syscall_sigaction;
#endif 0
}

/*
 * System calls enter here.
 */
void
emul_syscall(regs)
	register struct emul_regs *regs;
{
	register int	syscode;
	register int	error;
	register struct sysent *callp;
	int		rval[2];
	boolean_t	interrupt = FALSE;
	register struct emul_regs_2 *regs2,*regs2_mv;
	register int	*args;

	regs2 = regs->uesp;
	args = (int *)(regs2+1);	/* args on stack top */
	args++;	/* point to first argument - skip return address */

	syscode = regs2->eax;

#ifdef	MAP_UAREA
	if (shared_enabled) {
	    spin_lock(&in_emulator_lock);
	    shared_base_rw->us_in_emulator++;
	    spin_unlock(&in_emulator_lock);
	    if (shared_base_ro->us_cursig) {
		error = ERESTART;
		goto signal;
	    }
	}
#endif	MAP_UAREA

	if (syscode == 0) {
	    /*
	     * Indirect system call.
	     */
	    syscode = *args++;
	}

	/*
	 * Find system call table entry for the system call.
	 */
	if (syscode >= nsysent)
	    callp = &sysent[179];	/* nosysent */
	else if (syscode >= 0)
	    callp = &sysent[syscode];
	else {
	    /*
	     * Negative system call numbers are CMU extensions.
	     */
	    if (syscode == -33)
		callp = &sysent_task_by_pid;
	    else if (syscode == -6)
		callp = &sysent[SYS_table];
	    else if (syscode == -34)
		callp = &sysent_pid_by_task;
	    else if (syscode == -41)
		callp = &sysent_init_process;
	    else if (syscode == -59)
		callp = &sysent_htg_ux_syscall;
	    else
		callp = &sysent[179];	/* nosysent */
	}

	/*
	 * Set up the initial return values.
	 */
	rval[0] = 0;
	rval[1] = regs2->edx;

	/*
	 * Call the routine, passing arguments according to the table
	 * entry.
	 */
	switch (callp->nargs) {
	    case 0:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				rval);
		break;
	    case 1:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0],
				rval);
		break;
	    case 2:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1],
				rval);
		break;
	    case 3:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1], args[2],
				rval);
		break;
	    case 4:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1], args[2], args[3],
				rval);
		break;
	    case 5:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1], args[2], args[3], args[4],
				rval);
		break;
	    case 6:
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args[0], args[1], args[2],
				args[3], args[4], args[5],
				rval);
		break;

	    case -1:	/* generic */
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				syscode,
				args,
				rval);
		break;

	    case -2:	/* pass registers to modify */
		error = (*callp->routine)(our_bsd_server_port,
				&interrupt,
				args,
				rval,
				regs);
		regs2 = regs->uesp;	/* if changed */
		break;
	}

	/*
	 * Set up return values.
	 */

#ifdef	MAP_UAREA
signal:
#endif	MAP_UAREA

	switch (error) {
	    case E_JUSTRETURN:
		/* Do not alter registers */
		break;

	    case 0:
		/* Success */
		regs2->eflags &= ~EFL_CF;
		regs2->eax = rval[0];
		regs2->edx = rval[1];
		break;

	    case ERESTART:
		/* restart call */
		regs2->eip -= 7;
		break;

	    default:
		/* error */
		regs2->eflags |= EFL_CF;
		regs2->eax = error;
		break;
	}

#ifdef	MAP_UAREA
	if (shared_enabled) {
		spin_lock(&in_emulator_lock);
		shared_base_rw->us_in_emulator--;
		spin_unlock(&in_emulator_lock);
	}
#endif	/* MAP_UAREA */

	/*
	 * Handle interrupt request
	 */
	if (error == ERESTART || error == EINTR || interrupt)
	    take_signals(regs);
}

/*
 * Exec starts here to save registers.
 */
struct execa {
    char	*fname;
    char	**argp;
    char	**envp;
};

int
e_execv(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register struct execa	*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	struct execa		execa;

	execa.fname = argp->fname;
	execa.argp  = argp->argp;
	execa.envp  = (char **)0;

	return (e_execve(serv_port, interrupt, &execa, rval, regs));
}

int
e_execve(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register struct execa	*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register struct emul_regs_2 *regs2;
	int		entry[2];
	unsigned int	entry_count;
	vm_offset_t	arg_addr;
	register int	error;

	/*
	 * Do not have to save user registers on old stack;
	 * they will all be cleared.
	 */

	/*
	 * Call exec.  If error, return without changing registers.
	 */
	entry_count = 2;
	error = e_exec_call(serv_port,
			    interrupt,
			    argp->fname,
			    argp->argp,
			    argp->envp,
			    &arg_addr,
			    entry,
			    &entry_count);
	if (error)
	    return (error);

	/*
	 * Put new user stack just below arguments.
	 */
	regs2 = ((struct emul_regs_2 *)arg_addr) - 1;

	regs2->eip = entry[0];
	regs2->eflags = EFL_USER_SET;
	regs->uesp = regs2;

	/*
	 * Return to new stack.
	 */
	return (E_JUSTRETURN);
}


struct sigframe {
	int	sf_signum;
	int	sf_code;
	struct	sigcontext *sf_scp;
	sig_t	sf_handler;
	int	sf_eax;	
	int	sf_edx;	
	int	sf_ecx;	
	struct	sigcontext sf_sc;
} ;

/*
 * Take a signal.
 */
void
take_signals(regs)
	register struct emul_regs *regs;
{
	struct emul_regs_2	save_regs2;
	register struct emul_regs_2	*regs2;

	register struct sigframe *fp;

	int	old_mask, old_onstack, sig, code, handler, new_sp;
	boolean_t	interrupt;

	/*
	 * Get anything valuable off user stack first.
	 */
	save_regs2 = *regs->uesp;

	/*
	 * Get the signal to take from the server.  It also
	 * switches the signal mask and the stack, so we must
	 * be off the old user stack before calling it.
	 */
	(void) bsd_take_signal(our_bsd_server_port,
			&interrupt,
			&old_mask,
			&old_onstack,
			&sig,
			&code,
			&handler,
			&new_sp);

	/*
	 * If there really were no signals to take, return.
	 */
	if (sig == 0)
	    return;

	/*
	 * Put the signal context and signal frame on the signal stack.
	 */
	if (new_sp == 0) {
	    /*
	     * Build signal frame and context on user's stack.
	     */
	    new_sp = (int)regs->uesp;
	}

	fp  = ((struct sigframe *)new_sp) - 1;

	/*
	 * Build the argument list for the signal handler.
	 */
	fp->sf_signum = sig;
	fp->sf_code = code;
	fp->sf_scp = &fp->sf_sc;
	fp->sf_handler = (sig_t)handler;

	/* save scracth registers */
	fp->sf_edx = save_regs2.edx;
	fp->sf_ecx = save_regs2.ecx;
	fp->sf_eax = save_regs2.eax;

	/*
	 * Build the signal context to be used by sigreturn.
	 */
	fp->sf_sc.sc_onstack = old_onstack;
	fp->sf_sc.sc_mask = old_mask;

	fp->sf_sc.sc_pc = save_regs2.eip;
	fp->sf_sc.sc_ps = save_regs2.eflags;
	fp->sf_sc.sc_sp = (int)regs->uesp;
	fp->sf_sc.sc_fp = regs->ebp;

	/*
	 * Set up the new stack and handler addresses.
	 */
	regs2 = ((struct emul_regs_2 *)fp) - 1;
	*regs2 = save_regs2;
	regs->uesp = regs2;
	regs->ebp = (int)fp;
	regs2->eip = USRSTACK-TRAMPOLINE_MAX_SIZE;
}

int
e_sigreturn(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	struct emul_regs_2		save_regs2;
	register struct emul_regs_2	*regs2;
	register int			rc;
	register struct sigframe 	fp;
	struct a {
	    struct sigcontext *sigcp;
	} *uap = (struct a *)argp;

	fp = *(struct sigframe *)regs->ebp;

	/*
	 * Change signal stack and mask.  If new signals are pending,
	 * do not take them until we switch user stack.
	 */
#ifdef	MAP_UAREA
    if (shared_enabled) {
	rc = e_shared_sigreturn(serv_port,
				interrupt,
				fp.sf_sc.sc_onstack & 01,
				fp.sf_sc.sc_mask);

    } else {
#endif	MAP_UAREA
	rc = bsd_sigreturn(serv_port,
			interrupt,
			fp.sf_sc.sc_onstack & 01,
			fp.sf_sc.sc_mask);
#ifdef	MAP_UAREA
    }
#endif	MAP_UAREA

	/*
	 * Change registers.
	 */
	regs2 = (struct emul_regs_2 *)fp.sf_sc.sc_sp;

	regs->uesp = regs2;
	regs->ebp = fp.sf_sc.sc_fp;
	regs2->edx = fp.sf_edx;
	regs2->ecx = fp.sf_ecx;
	regs2->eax = fp.sf_eax;
	regs2->eip = fp.sf_sc.sc_pc;
	regs2->eflags = (fp.sf_sc.sc_ps & ~EFL_USER_CLEAR) | EFL_USER_SET;

	return (E_JUSTRETURN);
}

/*
 * Compatibility with 4.2 chmk $139 used by longjmp()
 */
e_osigcleanup()
{
}

#if 0
#ifdef	COMPAT_43
/*
 * Sigvec hides the signal trampoline address in a VERY
 * strange place.
 */
int
i386_osigvec(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register int		*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register int	rc;
	register vm_offset_t	sigtramp;

	sigtramp = regs->uesp->edx;	/* (!) */

	rc = e_osigvec(serv_port,
			interrupt,
			argp[0],			/* signo */
			(struct sigvec *)argp[1],	/* nsv */
			(struct sigvec *)argp[2],	/* osv */
			sigtramp);

	if (rc == 0)
	    sigreturnaddr = sigtramp;

	return (rc);
}
#endif	COMPAT_43

int
i386_signal(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register int		*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register int	rc;
	register vm_offset_t	sigtramp;

	sigtramp = regs->uesp->edx;	/* (!) */

	rc = e_signal(serv_port,
		     interrupt,
		     argp[0],			/* signo */
		     (int (*)())argp[1],	/* handler */
		     sigtramp);

	if (rc == 0)
		sigreturnaddr = sigtramp;

	return (rc);
}

int
i386_sigaction(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register int		*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register int	rc;
	register vm_offset_t	sigtramp;	/* sigreturn() in fact */

	sigtramp = regs->uesp->edx;	/* (!) */

	rc = e_sigaction(serv_port,
			interrupt,
			argp[0],			/* signo */
			(struct sigaction *)argp[1],	/* nsv */
			(struct sigaction *)argp[2],	/* osv */
			sigtramp);

	if (rc == 0)
	    sigreturnaddr = sigtramp;

	return (rc);
}

#endif 0
/*
 * Wait has a weird parameter passing mechanism.
 */
int
e_wait(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register struct emul_regs_2 *regs2 = regs->uesp;

	int	new_args[4];		/* POSSIBLE HACK (jqr) */

#define	EFL_ALLCC	(EFL_CF|EFL_PF|EFL_ZF|EFL_SF)

	if ((regs2->eflags & EFL_ALLCC) == EFL_ALLCC) {
	    new_args[2] = regs2->ecx;	/* options */
	    new_args[3] = regs2->edx;	/* rusage_p */
	}
	else {
	    new_args[2] = 0;
	    new_args[3] = 0;
	}
	return (emul_generic(serv_port, interrupt,
			84/*SYS_wait*/, &new_args[0], rval));
}
int
e_fork(serv_port, interrupt, argp, rval, regs)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	int			*argp;
	int			*rval;
	struct emul_regs	*regs;
{
	register struct emul_regs_2 *regs2 = regs->uesp;
	register int error;

	struct i386_thread_state	child_regs;
	int				seg_regs[6];
	
	extern int	child_fork();
	extern void	get_seg_regs();

	/*
	 * Set up registers for child.  It resumes on its own stack.
	 */
	get_seg_regs(seg_regs);

	child_regs.gs = seg_regs[5];
	child_regs.fs = seg_regs[4];
	child_regs.es = seg_regs[3];
	child_regs.ds = seg_regs[2];
	child_regs.edi = regs->edi;
	child_regs.esi = regs->esi;
	child_regs.ebp = regs->ebp;
     /*	child_regs.esp */
	child_regs.ebx = regs->ebx;
	child_regs.edx = regs2->edx;
	child_regs.ecx = regs2->ecx;
	child_regs.eax = regs2->eax;
	child_regs.eip = (int)child_fork;
	child_regs.cs = seg_regs[0];
	child_regs.efl = regs2->eflags;
	child_regs.uesp = (int)regs->uesp;
	child_regs.ss = seg_regs[1];
	
	/* FP regs!!!! */

	/*
	 * Create the child.
	 */
	error = bsd_fork(serv_port, interrupt,
			 (int *)&child_regs,
			 i386_THREAD_STATE_COUNT,
			 &rval[0]);

	if (error == 0)
	    rval[1] = 0;

	return (error);
}

vm_offset_t
set_arg_addr(arg_size)
	vm_size_t	arg_size;
{
	/*
	 * Round arg size to fullwords
	 */
	arg_size = (arg_size + NBPW-1) & ~(NBPW - 1);

	/*
	 * Put argument list at top of stack.
	 */
	return (USRSTACK - TRAMPOLINE_MAX_SIZE - arg_size);
}
