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
 * $Log:	kern_sig.c,v $
 * Revision 2.3  92/07/08  16:19:26  mrt
 * 	  SA_OLDMASK is in ps->ps_flags NOT p->p_flags.
 * 	  [92/07/04            rwd]
 * 	  Added debugging code.  Fixed ^Z problems.
 * 	  [92/07/02            rwd]
 * 
 * Revision 2.2.1.2  92/07/06  15:31:16  rwd
 * 	SA_OLDMASK is in ps->ps_flags NOT p->p_flags.
 * 	[92/07/04            rwd]
 * 
 * Revision 2.2.1.1  92/07/02  17:31:52  rwd
 * 	Added debugging code.  Fixed ^Z problems.
 * 	[92/07/02            rwd]
 * 
 * Revision 2.2  92/05/25  14:43:44  rwd
 * 	Fix this and that.
 * 	[92/05/21            rwd]
 * 
 * Revision 2.1  92/04/21  17:12:48  rwd
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
 *	@(#)kern_sig.c	7.35 (Berkeley) 6/28/91
 */

#define	SIGPROP		/* include signal properties table */

#include <ktrace.h>
#include <diagnostic.h>

#include <sys/param.h>
#include <sys/signalvar.h>
#include <sys/resourcevar.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/buf.h>
#include <sys/seg.h>
#include <sys/acct.h>
#include <sys/file.h>
#include <sys/kernel.h>
#include <sys/wait.h>
#include <sys/ktrace.h>
#include <sys/syscall.h>
#include <sys/parallel.h>

#include <uxkern/import_mach.h>
#include <sys/kinfo_proc.h>
#include <sys/user.h>		/* for coredump */
#include <sys/synch.h>

#define SIGNAL_DEBUG 1
#if SIGNAL_DEBUG
int signal_debug = 0;
#define SD(statement) if (signal_debug) statement
#else
#define SD(x)
#endif SIGNAL_DEBUG

/*
 * Can process p, with pcred pc, send the signal signo to process q?
 */
#define CANSIGNAL(p, pc, q, signo) \
	((pc)->pc_ucred->cr_uid == 0 || \
	    (pc)->p_ruid == (q)->p_cred->p_ruid || \
	    (pc)->pc_ucred->cr_uid == (q)->p_cred->p_ruid || \
	    (pc)->p_ruid == (q)->p_ucred->cr_uid || \
	    (pc)->pc_ucred->cr_uid == (q)->p_ucred->cr_uid || \
	    ((signo) == SIGCONT && (q)->p_session == (p)->p_session))

/* ARGSUSED */
sigaction(p, uap, retval)
	struct proc *p;
	register struct args {
		int	signo;
		struct	sigaction *nsa;
		struct	sigaction *osa;
	} *uap;
	int *retval;
{
	struct sigaction vec;
	register struct sigaction *sa;
	register struct sigacts *ps = p->p_sigacts;
	register int sig;
	int bit, error;

	SD(printf("%8x: sigaction %x\n",p, uap->signo));
	sig = uap->signo;
	if (sig <= 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP)
		return (EINVAL);
	sa = &vec;
	if (uap->osa) {
		sa->sa_handler = ps->ps_sigact[sig];
		sa->sa_mask = ps->ps_catchmask[sig];
		bit = sigmask(sig);
		sa->sa_flags = 0;
		if ((ps->ps_sigonstack & bit) != 0)
			sa->sa_flags |= SA_ONSTACK;
		if ((ps->ps_sigintr & bit) == 0)
			sa->sa_flags |= SA_RESTART;
		if (p->p_flag & SNOCLDSTOP)
			sa->sa_flags |= SA_NOCLDSTOP;
		if (error = copyout((caddr_t)sa, (caddr_t)uap->osa,
		    sizeof (vec)))
			return (error);
	}
	if (uap->nsa) {
		if (error = copyin((caddr_t)uap->nsa, (caddr_t)sa,
		    sizeof (vec)))
			return (error);
		setsigvec(p, sig, sa);
	}
	return (0);
}

setsigvec(p, sig, sa)
	register struct proc *p;
	int sig;
	register struct sigaction *sa;
{
	register struct sigacts *ps = p->p_sigacts;
	register int bit;

	SD(printf("%8x: setsigvec %x %x\n",p, sig, (int)sa));
	bit = sigmask(sig);
	/*
	 * Change setting atomically.
	 */
	(void) splhigh();
	ps->ps_sigact[sig] = sa->sa_handler;
	ps->ps_catchmask[sig] = sa->sa_mask &~ sigcantmask;
	if ((sa->sa_flags & SA_RESTART) == 0)
		ps->ps_sigintr |= bit;
	else
		ps->ps_sigintr &= ~bit;
	if (sa->sa_flags & SA_ONSTACK)
		ps->ps_sigonstack |= bit;
	else
		ps->ps_sigonstack &= ~bit;
	if (sig == SIGCHLD) {
		if (sa->sa_flags & SA_NOCLDSTOP)
			p->p_flag |= SNOCLDSTOP;
		else
			p->p_flag &= ~SNOCLDSTOP;
	}
	/*
	 * Set bit in p_sigignore for signals that are set to SIG_IGN,
	 * and for signals set to SIG_DFL where the default is to ignore.
	 * However, don't put SIGCONT in p_sigignore,
	 * as we have to restart the process.
	 */
	if (sa->sa_handler == SIG_IGN ||
	    (sigprop[sig] & SA_IGNORE && sa->sa_handler == SIG_DFL)) {
		p->p_sig &= ~bit;		/* never to be seen again */
		if (sig != SIGCONT)
			p->p_sigignore |= bit;	/* easier in psignal */
		p->p_sigcatch &= ~bit;
	} else {
		p->p_sigignore &= ~bit;
		if (sa->sa_handler == SIG_DFL)
			p->p_sigcatch &= ~bit;
		else
			p->p_sigcatch |= bit;
	}
	(void) spl0();
}

/*
 * Initialize signal state for process 0;
 * set to ignore signals that are ignored by default.
 */
void
siginit(p)
	struct proc *p;
{
	register int i;

	for (i = 0; i < NSIG; i++)
		if (sigprop[i] & SA_IGNORE && i != SIGCONT)
			p->p_sigignore |= sigmask(i);
}

/*
 * Reset signals for an exec of the specified process.
 */
void
execsigs(p)
	register struct proc *p;
{
	register struct sigacts *ps = p->p_sigacts;
	register int nc, mask;

	/*
	 * Reset caught signals.  Held signals remain held
	 * through p_sigmask (unless they were caught,
	 * and are now ignored by default).
	 */
	while (p->p_sigcatch) {
		nc = ffs((long)p->p_sigcatch);
		mask = sigmask(nc);
		p->p_sigcatch &= ~mask;
		if (sigprop[nc] & SA_IGNORE) {
			if (nc != SIGCONT)
				p->p_sigignore |= mask;
			p->p_sig &= ~mask;
		}
		ps->ps_sigact[nc] = SIG_DFL;
	}
	/*
	 * Reset stack state to the user stack.
	 * Clear set of signals caught on the signal stack.
	 */
	ps->ps_onstack = 0;
	ps->ps_sigsp = 0;
	ps->ps_sigonstack = 0;
}

/*
 * Manipulate signal mask.
 * Note that we receive new mask, not pointer,
 * and return old mask as return value;
 * the library stub does the rest.
 */
sigprocmask(p, uap, retval)
	register struct proc *p;
	struct args {
		int	how;
		sigset_t mask;
	} *uap;
	int *retval;
{
	int error = 0;
	SD(printf("%8x: sigprocmask %x %x\n",p, uap->how, uap->mask));

	*retval = p->p_sigmask;
	(void) splhigh();

	switch (uap->how) {
	case SIG_BLOCK:
		p->p_sigmask |= uap->mask &~ sigcantmask;
		break;

	case SIG_UNBLOCK:
		p->p_sigmask &= ~uap->mask;
		break;

	case SIG_SETMASK:
		p->p_sigmask = uap->mask &~ sigcantmask;
		break;
	
	default:
		error = EINVAL;
		break;
	}
	(void) spl0();
	return (error);
}

/* ARGSUSED */
sigpending(p, uap, retval)
	struct proc *p;
	void *uap;
	int *retval;
{

	SD(printf("%8x: sigpending %x\n",p, p->p_sig));
	*retval = p->p_sig;
	return (0);
}

#ifdef COMPAT_43
/*
 * Generalized interface signal handler, 4.3-compatible.
 */
/* ARGSUSED */
osigvec(p, uap, retval)
	struct proc *p;
	register struct args {
		int	signo;
		struct	sigvec *nsv;
		struct	sigvec *osv;
	} *uap;
	int *retval;
{
	struct sigvec vec;
	register struct sigacts *ps = p->p_sigacts;
	register struct sigvec *sv;
	register int sig;
	int bit, error;

	sig = uap->signo;
	if (sig <= 0 || sig >= NSIG || sig == SIGKILL || sig == SIGSTOP)
		return (EINVAL);
	sv = &vec;
	if (uap->osv) {
		*(sig_t *)&sv->sv_handler = ps->ps_sigact[sig];
		sv->sv_mask = ps->ps_catchmask[sig];
		bit = sigmask(sig);
		sv->sv_flags = 0;
		if ((ps->ps_sigonstack & bit) != 0)
			sv->sv_flags |= SV_ONSTACK;
		if ((ps->ps_sigintr & bit) != 0)
			sv->sv_flags |= SV_INTERRUPT;
		if (p->p_flag & SNOCLDSTOP)
			sv->sv_flags |= SA_NOCLDSTOP;
		if (error = copyout((caddr_t)sv, (caddr_t)uap->osv,
		    sizeof (vec)))
			return (error);
	}
	if (uap->nsv) {
		if (error = copyin((caddr_t)uap->nsv, (caddr_t)sv,
		    sizeof (vec)))
			return (error);
		sv->sv_flags ^= SA_RESTART;	/* opposite of SV_INTERRUPT */
		setsigvec(p, sig, (struct sigaction *)sv);
	}
	return (0);
}

osigblock(p, uap, retval)
	register struct proc *p;
	struct args {
		int	mask;
	} *uap;
	int *retval;
{

	(void) splhigh();
	*retval = p->p_sigmask;
	p->p_sigmask |= uap->mask &~ sigcantmask;
	(void) spl0();
	return (0);
}

osigsetmask(p, uap, retval)
	struct proc *p;
	struct args {
		int	mask;
	} *uap;
	int *retval;
{

	(void) splhigh();
	*retval = p->p_sigmask;
	p->p_sigmask = uap->mask &~ sigcantmask;
	(void) spl0();
	return (0);
}
#endif

/*
 * Suspend process until signal, providing mask to be set
 * in the meantime.  Note nonstandard calling convention:
 * libc stub passes mask, not pointer, to save a copyin.
 */
/* ARGSUSED */
sigsuspend(p, uap, retval)
	register struct proc *p;
	struct args {
		sigset_t mask;
	} *uap;
	int *retval;
{
	register struct sigacts *ps = p->p_sigacts;
	SD(printf("%8x: sigsuspend %x\n",p, uap->mask));

	/*
	 * When returning from sigpause, we want
	 * the old mask to be restored after the
	 * signal handler has finished.  Thus, we
	 * save it here and mark the proc structure
	 * to indicate this (should be in sigacts).
	 */
	ps->ps_oldmask = p->p_sigmask;
	ps->ps_flags |= SA_OLDMASK;
	p->p_sigmask = uap->mask &~ sigcantmask;
	(void) tsleep((caddr_t) ps, PPAUSE|PCATCH, "pause", 0);
	/* always return EINTR rather than ERESTART... */
	return (EINTR);
}

/* ARGSUSED */
sigstack(p, uap, retval)
	struct proc *p;
	register struct args {
		struct	sigstack *nss;
		struct	sigstack *oss;
	} *uap;
	int *retval;
{
	struct sigstack ss;
	int error = 0;
	SD(printf("%8x: sigstack\n",p));

	if (uap->oss && (error = copyout((caddr_t)&p->p_sigacts->ps_sigstack,
	    (caddr_t)uap->oss, sizeof (struct sigstack))))
		return (error);
	if (uap->nss && (error = copyin((caddr_t)uap->nss, (caddr_t)&ss,
	    sizeof (ss))) == 0)
		p->p_sigacts->ps_sigstack = ss;
	return (error);
}

/* ARGSUSED */
kill(cp, uap, retval)
	register struct proc *cp;
	register struct args {
		int	pid;
		int	signo;
	} *uap;
	int *retval;
{
	register struct proc *p;
	register struct pcred *pc = cp->p_cred;

	SD(printf("%8x: kill %d %x\n",p, uap->pid, uap->signo));
	if ((unsigned) uap->signo >= NSIG)
		return (EINVAL);
	if (uap->pid > 0) {
		/* kill single process */
		p = pfind(uap->pid);
		if (p == 0)
			return (ESRCH);
		if (!CANSIGNAL(cp, pc, p, uap->signo))
			return (EPERM);
		if (uap->signo)
			psignal(p, uap->signo);
		return (0);
	}
	switch (uap->pid) {
	case -1:		/* broadcast signal */
		return (killpg1(cp, uap->signo, 0, 1));
	case 0:			/* signal own process group */
		return (killpg1(cp, uap->signo, 0, 0));
	default:		/* negative explicit process group */
		return (killpg1(cp, uap->signo, -uap->pid, 0));
	}
	/* NOTREACHED */
}

#ifdef COMPAT_43
/* ARGSUSED */
okillpg(p, uap, retval)
	struct proc *p;
	register struct args {
		int	pgid;
		int	signo;
	} *uap;
	int *retval;
{

	if ((unsigned) uap->signo >= NSIG)
		return (EINVAL);
	return (killpg1(p, uap->signo, uap->pgid, 0));
}
#endif

/*
 * Common code for kill process group/broadcast kill.
 * cp is calling process.
 */
killpg1(cp, signo, pgid, all)
	register struct proc *cp;
	int signo, pgid, all;
{
	register struct proc *p;
	register struct pcred *pc = cp->p_cred;
	struct pgrp *pgrp;
	int nfound = 0;
	
	if (all)	
		/* 
		 * broadcast 
		 */
		for (p = allproc; p != NULL; p = p->p_nxt) {
			if (p->p_pid <= 1 || p->p_flag&SSYS || 
			    p == cp || !CANSIGNAL(cp, pc, p, signo))
				continue;
			nfound++;
			if (signo)
				psignal(p, signo);
		}
	else {
		if (pgid == 0)		
			/* 
			 * zero pgid means send to my process group.
			 */
			pgrp = cp->p_pgrp;
		else {
			pgrp = pgfind(pgid);
			if (pgrp == NULL)
				return (ESRCH);
		}
		for (p = pgrp->pg_mem; p != NULL; p = p->p_pgrpnxt) {
			if (p->p_pid <= 1 || p->p_flag&SSYS ||
			    p->p_stat == SZOMB || !CANSIGNAL(cp, pc, p, signo))
				continue;
			nfound++;
			if (signo)
				psignal(p, signo);
		}
	}
	return (nfound ? 0 : ESRCH);
}

/*
 * Send the specified signal to
 * all processes with 'pgid' as
 * process group.
 */
void
gsignal(pgid, sig)
	int pgid, sig;
{
	struct pgrp *pgrp;

	if (pgid && (pgrp = pgfind(pgid)))
		pgsignal(pgrp, sig, 0);
}

/*
 * Send sig to every member of a process group.
 * If checktty is 1, limit to members which have a controlling
 * terminal.
 */
void
pgsignal(pgrp, sig, checkctty)
	struct pgrp *pgrp;
	int sig, checkctty;
{
	register struct proc *p;

	if (pgrp)
		for (p = pgrp->pg_mem; p != NULL; p = p->p_pgrpnxt)
			if (checkctty == 0 || p->p_flag&SCTTY)
				psignal(p, sig);
}

/*
 * Send the specified signal to the specified process.
 * If the signal has an action, the action is usually performed
 * by the target process rather than the caller; we simply add
 * the signal to the set of pending signals for the process.
 * Exceptions:
 *   o When a stop signal is sent to a sleeping process that takes the default
 *     action, the process is stopped without awakening it.
 *   o SIGCONT restarts stopped processes (or puts them back to sleep)
 *     regardless of the signal action (eg, blocked or ignored).
 * Other ignored signals are discarded immediately.
 */
void
psignal(p, sig)
	register struct proc *p;
	register int sig;
{
    	struct proc *curproc = (struct proc*)cthread_data(cthread_self());
	register int s, prop;
	register sig_t action;
	int mask;

	SD(printf("%8x: psignal %x %x\n",curproc, p, sig));
	if ((unsigned)sig >= NSIG || sig == 0)
		panic("psignal sig");
	mask = sigmask(sig);
	prop = sigprop[sig];

	/*
	 * If proc is traced, always give parent a chance.
	 */
	if (p->p_flag & STRC)
		action = SIG_DFL;
	else {
		/*
		 * If the signal is being ignored,
		 * then we forget about it immediately.
		 * (Note: we don't set SIGCONT in p_sigignore,
		 * and if it is set to SIG_IGN,
		 * action will be SIG_DFL here.)
		 */
		if (p->p_sigignore & mask)
			return;
		if (p->p_sigmask & mask)
			action = SIG_HOLD;
		else if (p->p_sigcatch & mask)
			action = SIG_CATCH;
		else
			action = SIG_DFL;
	}

	if (p->p_nice > NZERO && (sig == SIGKILL ||
	    sig == SIGTERM && (p->p_flag&STRC || action != SIG_DFL)))
		p->p_nice = NZERO;

	if (prop & SA_CONT)
		p->p_sig &= ~stopsigmask;

	if (prop & SA_STOP) {
		/*
		 * If sending a tty stop signal to a member of an orphaned
		 * process group, discard the signal here if the action
		 * is default; don't stop the process below if sleeping,
		 * and don't clear any pending SIGCONT.
		 */
		if (prop & SA_TTYSTOP && p->p_pgrp->pg_jobc == 0 &&
		    action == SIG_DFL)
		        return;
		p->p_sig &= ~contsigmask;
	}
	p->p_sig |= mask;

	/*
	 * Defer further processing for signals which are held,
	 * except that stopped processes must be continued by SIGCONT.
	 */
	if (action == SIG_HOLD && ((prop & SA_CONT) == 0 || p->p_stat != SSTOP))
		return;
	s = splhigh();
	switch (p->p_stat) {

	case SSLEEP:
		/*
		 * If process is sleeping uninterruptibly
		 * we can't interrupt the sleep... the signal will
		 * be noticed when the process returns through
		 * trap() or syscall().
		 */
		if ((p->p_flag & SSINTR) == 0)
			goto out;
		/*
		 * Process is sleeping and traced... make it runnable
		 * so it can discover the signal in issig() and stop
		 * for the parent.
		 */
		if (p->p_flag&STRC)
			goto run;
		/*
		 * When a sleeping process receives a stop
		 * signal, process immediately if possible.
		 * All other (caught or default) signals
		 * cause the process to run.
		 */
		if (prop & SA_STOP) {
			if (action != SIG_DFL)
				goto runfast;
			/*
			 * If a child holding parent blocked,
			 * stopping could cause deadlock.
			 */
			if (p->p_flag&SPPWAIT)
				goto out;
			p->p_sig &= ~mask;
			p->p_cursig = sig;
			if ((p->p_pptr->p_flag & SNOCLDSTOP) == 0)
				psignal(p->p_pptr, SIGCHLD);
			stop(p);
			goto out;
		} else
			goto runfast;
		/*NOTREACHED*/

	case SSTOP:
		/*
		 * If traced process is already stopped,
		 * then no further action is necessary.
		 */
		if (p->p_flag&STRC)
			goto out;

		/*
		 * Kill signal always sets processes running.
		 */
		if (sig == SIGKILL)
			goto runfast;

		if (prop & SA_CONT) {
			/*
			 * If SIGCONT is default (or ignored), we continue
			 * the process but don't leave the signal in p_sig,
			 * as it has no further action.  If SIGCONT is held,
			 * continue the process and leave the signal in p_sig.
			 * If the process catches SIGCONT, let it handle
			 * the signal itself.  If it isn't waiting on
			 * an event, then it goes back to run state.
			 * Otherwise, process goes back to sleep state.
			 */
			if (action == SIG_DFL)
				p->p_sig &= ~mask;
			if (action == SIG_CATCH)
				goto runfast;
			if (p->p_wchan == 0)
				goto runfast;
			p->p_stat = SSLEEP;
			goto out;
		}

		if (prop & SA_STOP) {
			/*
			 * Already stopped, don't need to stop again.
			 * (If we did the shell could get confused.)
			 */
			p->p_sig &= ~mask;		/* take it away */
			goto out;
		}

		/*
		 * If process is sleeping interruptibly, then
		 * simulate a wakeup so that when it is continued,
		 * it will be made runnable and can look at the signal.
		 * But don't setrun the process, leave it stopped.
		 */
		if (p->p_wchan && p->p_flag & SSINTR)
			unsleep(p);
		goto out;

	default:
		/*
		 * SRUN, SIDL, SZOMB do nothing with the signal,
		 * other than kicking ourselves if we are running.
		 * It will either never be noticed, or noticed very soon.
		 */
		goto run;
	}
	/*NOTREACHED*/

runfast:
	/*
	 * Raise priority to at least PUSER.
	 */
	if (p->p_pri > PUSER)
		p->p_pri = PUSER;
	p->p_stat = SRUN;
	wakeup((caddr_t)&p->p_stat);
	task_resume(p->p_task);
run:
	unsleep(p);
out:
	splx(s);
}

/*
 * If the current process has a signal to process (should be caught
 * or cause termination, should interrupt current syscall),
 * return the signal number.  Stop signals with default action
 * are processed immediately, then cleared; they aren't returned.
 * This is checked after each entry to the system for a syscall
 * or trap (though this can usually be done without actually calling
 * issig by checking the pending signal masks in the CURSIG macro.)
 * The normal call sequence is
 *
 *	while (sig = CURSIG(curproc))
 *		psig(sig);
 */
issig(p)
	register struct proc *p;
{
	register int sig, mask, prop;

	SD(printf("%8x: issig\n",p));
	for (;;) {
		mask = p->p_sig &~ p->p_sigmask;
		if (p->p_flag&SPPWAIT)
			mask &= ~stopsigmask;
		if (mask == 0) { 	/* no signal to send */
		    p->p_cursig = 0;
		    return (0);
		}
		sig = ffs((long)mask);
		mask = sigmask(sig);
		prop = sigprop[sig];

		p->p_cursig = sig;
		p->p_sig &= ~mask;

		/*
		 * We should see pending but ignored signals
		 * only if STRC was on when they were posted.
		 */
		if (mask & p->p_sigignore && (p->p_flag&STRC) == 0)
			continue;
		if (p->p_flag&STRC && (p->p_flag&SPPWAIT) == 0) {
			/*
			 * If traced, always stop, and stay
			 * stopped until released by the parent.
			 */
			psignal(p->p_pptr, SIGCHLD);
			stop(p);
			sleep((caddr_t)&p->p_stat, PSPECL);

			/*
			 * If the traced bit got turned off,
			 * go back up to the top to rescan signals.
			 * This ensures that p_sig* and ps_sigact
			 * are consistent.
			 */
			if ((p->p_flag&STRC) == 0)
				continue;

			/*
			 * If parent wants us to take the signal,
			 * then it will leave it in p->p_cursig;
			 * otherwise we just look for signals again.
			 */
			sig = p->p_cursig;
			if (sig == 0)
				continue;

			/*
			 * Put the new signal into p_sig.
			 * If signal is being masked,
			 * look for other signals.
			 */
			mask = sigmask(sig);
			p->p_sig |= mask;
			if (p->p_sigmask & mask)
				continue;
		}

		/*
		 * Decide whether the signal should be returned.
		 * Return the signal's number, or fall through
		 * to clear it from the pending mask.
		 */
		switch ((int)p->p_sigacts->ps_sigact[sig]) {

		case SIG_DFL:
			/*
			 * Don't take default actions on system processes.
			 */
			if (p->p_pid <= 1)
				break;		/* == ignore */
			/*
			 * If there is a pending stop signal to process
			 * with default action, stop here,
			 * then clear the signal.  However,
			 * if process is member of an orphaned
			 * process group, ignore tty stop signals.
			 */
			if (prop & SA_STOP) {
				if (p->p_flag&STRC ||
		    		    (p->p_pgrp->pg_jobc == 0 &&
				    prop & SA_TTYSTOP))
					break;	/* == ignore */
				stop(p);
				if ((p->p_pptr->p_flag & SNOCLDSTOP) == 0)
					psignal(p->p_pptr, SIGCHLD);
				break;
			} else if (prop & SA_IGNORE) {
				/*
				 * Except for SIGCONT, shouldn't get here.
				 * Default action is to ignore; drop it.
				 */
				break;		/* == ignore */
			} else
				return (sig);

			/*NOTREACHED*/

		case SIG_IGN:
			/*
			 * Masking above should prevent us ever trying
			 * to take action on an ignored signal other
			 * than SIGCONT, unless process is traced.
			 */
			if ((prop & SA_CONT) == 0 && (p->p_flag&STRC) == 0)
				printf("issig\n");
			break;		/* == ignore */

		default:
			/*
			 * This signal has an action, let
			 * psig process it.
			 */
			return (sig);
		}
	}
	/* NOTREACHED */
}

/*
 * Put the argument process into the stopped
 * state and notify the parent via wakeup.
 * Signals are handled elsewhere.
 * The process must not be on the run queue.
 */
stop(p)
	register struct proc *p;
{

    SD(printf("%8x: stop\n",p));
    task_suspend(p->p_task);
    p->p_stat = SSTOP;
    p->p_flag &= ~SWTED;
    wakeup((caddr_t)p->p_pptr);
}

void psig(sig)
    register int	sig;
{
	register struct proc *p = (struct proc *)cthread_data(cthread_self());
	register struct sigacts *ps = p->p_sigacts;
	register sig_t action;

	SD(printf("%8x: psig %x\n",p, sig));
	action = ps->ps_sigact[sig];
	if (action != SIG_DFL) {

	    /*
	     * User handles sending himself signals.
	     */
	    return;
	}

	p->p_sig &= ~sigmask(sig);
	sig_default(sig);
}

/*
 * Take default action on signal.
 */
sig_default(sig)
	register int	sig;
{
	register struct proc *p = (struct proc *)cthread_data(cthread_self());
	boolean_t	dump;

	SD(printf("%8x: sig_default %x\n",p, sig));
	switch (sig) {
	    /*
	     *	The new signal code for multiple threads makes it possible
	     *	for a multi-threaded task to get here (a thread that didn`t
	     *	originally process a "stop" signal notices that cursig is
	     *	set), therefore, we must handle this.
	     */
	    case SIGIO:
	    case SIGURG:
	    case SIGCHLD:
	    case SIGCONT:
	    case SIGWINCH:
	    case SIGTSTP:
	    case SIGTTIN:
	    case SIGTTOU:
	    case SIGSTOP:
		return;

	    case SIGILL:
	    case SIGIOT:
	    case SIGBUS:
	    case SIGQUIT:
	    case SIGTRAP:
	    case SIGEMT:
	    case SIGFPE:
	    case SIGSEGV:
	    case SIGSYS:
		/*
		 * Kill the process with a core dump.
		 */
		dump = TRUE;
		break;
	
	    default:
		dump = FALSE;
		break;
	}

	sigexit(p,sig);
}

/*
 * New thread_psignal.  Runs as part of the normal service - thus
 * we have a thread per user exception, so it can wait.
 */
thread_psignal(task, thread, sig, code)
	task_t		task;
	thread_t	thread;
	register int	sig;
	int		code;
{
	register struct proc *p = (struct proc *)task_to_proc_lookup(task);

	SD(printf("%8x: thread_psignal %x\n",p, sig));
	if (p == 0) {
	    (void) task_terminate(task);
	    return;
	}
	if (sig < 0 || sig > NSIG)
	    return;

	/*
	 * Register thread as service thread
	 */
	server_thread_register(p);
	unix_master();

	thread_signal(p, thread, sig, code);

	unix_release();

	/*
	 * If thread_signal killed the process, then proc_exit
	 * might have already deregistered us.
	 */
/*	if (uth->uu_procp == 0)
		return;*/

	server_thread_deregister(p);

	if (p->p_flag & SWEXIT)
	    wakeup((caddr_t)&p->p_flag);
}

thread_signal(p, thread, sig, code)
	register struct proc *p;
	thread_t	thread;
	register int	sig;
	int		code;
{
	register int	mask;

	mask = sigmask(sig);

	SD(printf("%8x: thread_signal %x\n",p, sig));
	for (;;) {
	    if (p->p_sigmask & mask) {
		/*
		 * Save the signal in p_sig.
		 */
		p->p_sig |= mask;
		return;
	    }

	    while (p->p_stat == SSTOP || (p->p_flag & SWEXIT)) {
		if (p->p_flag & SWEXIT) {
		    return;
		}
		sleep((caddr_t)&p->p_stat, PSPECL);
	    }
	    if (p->p_sigmask & mask) {
		/* could have changed */
		continue;
	    }

	    if (p->p_flag & STRC) {
		/*
		 * Stop for trace.
		 */
		psignal(p->p_pptr, SIGCHLD);
		stop(p);
		sleep((caddr_t)&p->p_stat, PSPECL);

		if (p->p_flag & SWEXIT) {
		    return;
		}

		if ((p->p_flag & STRC) == 0) {
		    continue;
		}

		sig = p->p_cursig;
		if (sig == 0) {
		    return;
		}

		mask = sigmask(sig);
		if (p->p_sigmask & mask) {
		    continue;
		}
	    }
	    break;
	}

	switch ((int)p->p_sigacts->ps_sigact[sig]) {
	    case SIG_IGN:
	    case SIG_HOLD:
		/*
		 * Should not get here unless traced.
		 */
		break;

	    case SIG_DFL:
		sig_default(sig);
		break;

	    default:
		/*
		 * Send signal to user thread.
		 */
		send_signal(p, thread, sig, code);
		break;
	}
}

/*
 * Unblock thread signal if it was masked off.
 */
check_proc_signals(p)
	register struct proc *p;
{
	wakeup((caddr_t)&p->p_sigmask);
}

/*
 * Send the signal to a thread.  The thread must not be executing
 * any kernel calls.
 */
send_signal(p, thread, sig, code)
	register struct proc *p;
	thread_t	thread;
	register int	sig;
	int		code;
{
	register int	mask;
	int	returnmask;
	register struct sigacts *ps = p->p_sigacts;
	register sig_t action;

	SD(printf("%8x: send_signal %x\n",p, sig));
	action = ps->ps_sigact[sig];

	mask = sigmask(sig);

	/*
	 * At this point, thread is in either exception_raise,
	 * or emulator::take_signal, waiting for a reply message.
	 * There should be no need to suspend it.
	 */

	/*
	 * Signal action may have changed while we blocked for
	 * task_suspend or thread_abort.  Check it again.
	 */

	if ((p->p_sigcatch & mask) == 0) {
	    /*
	     * No longer catching signal.  Put it back if
	     * not ignoring it, and start up the task again.
	     */
	    psignal(p, sig);
	    (void) task_resume(p->p_task);
	    return;
	}
/*	if (p->p_flag & SOUSIG) {
	    if (sig != SIGILL && sig != SIGTRAP) {
		ps->ps_sigact[sig]= SIG_DFL;
		p->p_sigcatch &= ~mask;
	    }
	    mask = 0;
	}*/
	p->p_sigmask |= mask;
	if (ps->ps_flags & SA_OLDMASK) {
	    returnmask = ps->ps_oldmask;
	    ps->ps_flags &= ~SA_OLDMASK;
	} else
	    returnmask = p->p_sigmask;
	p->p_sigmask |= sigmask(sig);

	sendsig(p, thread, action, sig, code, returnmask);

	/*
	 * At this point, thread is in either exception_raise,
	 * or emulator::take_signal, waiting for a reply message.
	 * The reply message should resume it.
	 */
}

/*
 * Called by user to take a pending signal.
 */
int
bsd_take_signal(proc_port, interrupt, old_mask, old_onstack, o_sig, o_code,
		o_handler, new_sp)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* out */
	int		*old_mask;	/* out */
	int		*old_onstack;	/* out */
	int		*o_sig;		/* out */
	int		*o_code;	/* out */
	int		*o_handler;	/* out */
	int		*new_sp;	/* out */
{
	register struct proc *	p;
	register int	error;
	register int	sig, mask, sigbits;
	register struct sigacts *ps;
	register sig_t action;

	int		returnmask;
	int		oonstack;

	error = start_server_op(proc_port,1000);
		/* XXX code for take-signal */
	if (error)
	    return (error);

	p = (struct proc *)cthread_data(cthread_self());
	SD(printf("%8x: bsd_take_signal\n",p));
	ps = p->p_sigacts;
	unix_master();

	/*
	 * Process should be running.
	 * Should not get here if process is stopped.
	 */
	if (p->p_stat != SRUN || (p->p_flag & SWEXIT)) {
	    unix_release();
	    return (end_server_op(p, ESRCH, interrupt));
	}

	/*
	 * Set up return values in case no signals pending.
	 */
	*old_mask = 0;
	*old_onstack = 0;
	*o_sig = 0;
	*o_code = 0;
	*o_handler = 0;
	*new_sp = 0;

	/*
	 * Get pending signal.
	 */
	sig = p->p_cursig;
	if (sig != 0) {
	    p->p_cursig = 0;
	}
	else {
	    /*
	     * If no pending signal, get from signal masks.
	     * Yes, this can happen.
	     */
	    sig = issig(p);
	}
	if (sig == 0) {
	    /*
	     * No signals - return to user.
	     */
	    unix_release();
	    return (end_server_op(p, 0, interrupt));
	}

	action = ps->ps_sigact[sig];
	switch ((int)action) {
	    case SIG_IGN:
	    case SIG_HOLD:
		/*
		 * Should not get here.
		 */
		sig = 0;
		break;

	    case SIG_DFL:
		/*
		 * take default action
		 */
		sig_default(sig);
		sig = 0;
		break;

	    default:
		/*
		 * user gets signal
		 */
		mask = sigmask(sig);

		if ((p->p_sigcatch & mask) == 0) {
		    psignal(p, sig);
		    sig = 0;
		    break;
		}
/*		if (p->p_flag & SOUSIG) {
		    if (sig != SIGILL && sig != SIGTRAP) {
			ps->ps_sigact[sig]= SIG_DFL;
			p->p_sigcatch &= ~mask;
		    }
		    mask = 0;
		}*/
		if (ps->ps_flags & SA_OLDMASK) {
		    returnmask = ps->ps_oldmask;
		    ps->ps_flags &= ~SA_OLDMASK;
		}
		else
		    returnmask = p->p_sigmask;
		p->p_sigmask |= mask;
		oonstack = ps->ps_onstack;
		if (!oonstack && (ps->ps_sigonstack & mask)) {
		    *new_sp = (int) ps->ps_sigstack.ss_sp;
		    ps->ps_sigstack.ss_onstack = 1;
		}
		else
		    *new_sp = 0;	/* use existing stack */

		*old_mask = returnmask;
		*old_onstack = oonstack;
		*o_sig = sig;
		*o_code = 0;
		*o_handler = (int)action;
		break;
	}

	unix_release();
	return (end_server_op(p, 0, interrupt));
}

int
bsd_sigreturn(proc_port, interrupt, old_on_stack, old_sigmask)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		old_on_stack;
	int		old_sigmask;
{
	register int error;
	register struct proc *p;

	error = start_server_op(proc_port, SYS_sigreturn);
	if (error)
	    return (error);

	p = (struct proc *)cthread_data(cthread_self());

	SD(printf("%8x: bsd_sigreturn\n",p));
	unix_master();

	p->p_sigacts->ps_onstack = old_on_stack & 01;

	p->p_sigmask = old_sigmask & ~sigcantmask;

	check_proc_signals(p);

	unix_release();

	return (end_server_op(p, 0, interrupt));
}
#if 0
/*
 * Take the action for the specified signal
 * from the current set of pending signals.
 */
void
psig(sig)
	register int sig;
{
    	struct proc *p = (struct proc*)cthread_data(cthread_self());
	register struct sigacts *ps = p->p_sigacts;
	register sig_t action;
	int mask, returnmask;

#if DIAGNOSTIC
	if (sig == 0)
		panic("psig");
#endif
	mask = sigmask(sig);
	p->p_sig &= ~mask;
	action = ps->ps_sigact[sig];
#if KTRACE
	if (KTRPOINT(p, KTR_PSIG))
		ktrpsig(p->p_tracep, sig, action, ps->ps_flags & SA_OLDMASK ?
		    ps->ps_oldmask : p->p_sigmask, 0);
#endif
	if (action == SIG_DFL) {
		/*
		 * Default action, where the default is to kill
		 * the process.  (Other cases were ignored above.)
		 */
		sigexit(p, sig);
		/* NOTREACHED */
	} else {
		/*
		 * If we get here, the signal must be caught.
		 */
#if DIAGNOSTIC
		if (action == SIG_IGN || (p->p_sigmask & mask))
			panic("psig action");
#endif
		/*
		 * Set the new mask value and also defer further
		 * occurences of this signal.
		 *
		 * Special case: user has done a sigpause.  Here the
		 * current mask is not of interest, but rather the
		 * mask from before the sigpause is what we want
		 * restored after the signal processing is completed.
		 */
		(void) splhigh();
		if (ps->ps_flags & SA_OLDMASK) {
			returnmask = ps->ps_oldmask;
			ps->ps_flags &= ~SA_OLDMASK;
		} else
			returnmask = p->p_sigmask;
		p->p_sigmask |= ps->ps_catchmask[sig] | mask;
		(void) spl0();
		p->p_stats->p_ru.ru_nsignals++;
		sendsig(action, sig, returnmask, 0);
	}
}

#endif 0
/*
 * Force the current process to exit with the specified
 * signal, dumping core if appropriate.  We bypass the normal
 * tests for masked and caught signals, allowing unrecoverable
 * failures to terminate the process without changing signal state.
 * Mark the accounting record with the signal termination.
 * If dumping core, save the signal number for the debugger.
 * Calls exit and does not return.
 */
sigexit(p, sig)
	register struct proc *p;
	int sig;
{

	p->p_acflag |= AXSIG;
	if (sigprop[sig] & SA_CORE) {
		p->p_sigacts->ps_sig = sig;
		if (coredump(p) == 0)
			sig |= WCOREFLAG;
	}
	exit(p, W_EXITCODE(0, sig));
	/* NOTREACHED */
}

#if 0
/*
 * Create a core dump.
 * The file name is "core.progname".
 * Core dumps are not created if the process is setuid.
 */
coredump(p)
	register struct proc *p;
{
	register struct vnode *vp;
	register struct pcred *pcred = p->p_cred;
	register struct ucred *cred = pcred->pc_ucred;
	register struct vmspace *vm = p->p_vmspace;
	struct vattr vattr;
	int error, error1;
	struct nameidata nd;
	char name[MAXCOMLEN+6];	/* core.progname */

	if (pcred->p_svuid != pcred->p_ruid ||
	    pcred->p_svgid != pcred->p_rgid)
		return (EFAULT);
	if (ctob(UPAGES + vm->vm_dsize + vm->vm_ssize) >=
	    p->p_rlimit[RLIMIT_CORE].rlim_cur)
		return (EFAULT);
	sprintf(name, "core.%s", p->p_comm);
	nd.ni_dirp = name;
	nd.ni_segflg = UIO_SYSSPACE;
	if (error = vn_open(&nd, p, O_CREAT|FWRITE, 0644))
		return (error);
	vp = nd.ni_vp;
	if (vp->v_type != VREG || VOP_GETATTR(vp, &vattr, cred, p) ||
	    vattr.va_nlink != 1) {
		error = EFAULT;
		goto out;
	}
	VATTR_NULL(&vattr);
	vattr.va_size = 0;
	VOP_SETATTR(vp, &vattr, cred, p);
	p->p_acflag |= ACORE;
	bcopy(p, &p->p_addr->u_kproc.kp_proc, sizeof(struct proc));
	fill_eproc(p, &p->p_addr->u_kproc.kp_eproc);
#ifdef HPUXCOMPAT
	/*
	 * BLETCH!  If we loaded from an HPUX format binary file
	 * we have to dump an HPUX style user struct so that the
	 * HPUX debuggers can grok it.
	 */
	if (p->p_addr->u_pcb.pcb_flags & PCB_HPUXBIN)
		error = hpuxdumpu(vp, cred);
	else
#endif
	error = vn_rdwr(UIO_WRITE, vp, (caddr_t) p->p_addr, ctob(UPAGES),
	    (off_t)0, UIO_SYSSPACE, IO_NODELOCKED|IO_UNIT, cred, (int *) NULL,
	    p);
	if (error == 0)
		error = vn_rdwr(UIO_WRITE, vp, vm->vm_daddr,
		    (int)ctob(vm->vm_dsize), (off_t)ctob(UPAGES), UIO_USERSPACE,
		    IO_NODELOCKED|IO_UNIT, cred, (int *) NULL, p);
	if (error == 0)
		error = vn_rdwr(UIO_WRITE, vp,
		    (caddr_t) trunc_page(USRSTACK - ctob(vm->vm_ssize)),
		    round_page(ctob(vm->vm_ssize)),
		    (off_t)ctob(UPAGES) + ctob(vm->vm_dsize), UIO_USERSPACE,
		    IO_NODELOCKED|IO_UNIT, cred, (int *) NULL, p);
out:
	VOP_UNLOCK(vp);
	error1 = vn_close(vp, FWRITE, cred, p);
	if (error == 0)
		error = error1;
	return (error);
	return 0;
}

#endif 0
/*
 * Nonexistent system call-- signal process (may want to handle it).
 * Flag error in case process won't see signal immediately (blocked or ignored).
 */
/* ARGSUSED */
nosys(p, args, retval)
	struct proc *p;
	void *args;
	int *retval;
{

	psignal(p, SIGSYS);
	return (EINVAL);
}
coredump(p)
	register struct proc *p;
{}
