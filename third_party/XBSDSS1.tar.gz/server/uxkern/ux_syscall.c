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
 * $Log:	ux_syscall.c,v $
 * Revision 2.2  92/05/25  14:46:48  rwd
 * 	Add SSTRC code.
 * 	[92/05/21            rwd]
 * 
 * 	Clean up master_lock panic.  Fix race condition with exiting
 * 	     proc getting reused.
 * 	[92/05/19            rwd]
 * 
 * Revision 2.1  92/04/21  17:11:08  rwd
 * BSDSS
 * 
 *
 */

#include <uxkern/import_mach.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/parallel.h>
#include <sys/proc.h>
#include <sys/user.h>

#include <uxkern/bsd_msg.h>
#include <uxkern/syscalltrace.h>

extern struct sysent	sysent[];
int			nsysent;

#if	SYSCALLTRACE
int		syscalltrace;		/* no processes */
extern char	*syscallnames[];
#endif

/*
 * Generic UX server message handler.
 * Generic system call.
 */
boolean_t
ux_generic_server(InHeadP, OutHeadP)
	mach_msg_header_t *InHeadP, *OutHeadP;
{
	register struct bsd_request	*req = (struct bsd_request *)InHeadP;
	register struct bsd_reply	*rep = (struct bsd_reply *)OutHeadP;
	register struct sysent	*callp;
	register int		syscode = req->syscode;
	int			reply_size;
	struct proc *p = (struct proc *)req->hdr.msgh_local_port;
	struct proc *proc;
#if	SYSCALLTRACE
	int			pid;
#endif	SYSCALLTRACE
	int error, retval[2];

	/*
	 * Fix the reply message.
	 */
	static mach_msg_type_t bsd_rep_int_type = {
	    /* msgt_name */		MACH_MSG_TYPE_INTEGER_32,
	    /* msgt_size */		32,
	    /* msgt_number */		4,
	    /* msgt_inline */		TRUE,
	    /* msgt_longform */		FALSE,
	    /* msgt_deallocate */	FALSE,
	    /* msgt_unused */		0
	};

	/*
	 * Set up standard reply.
	 */
	rep->hdr.msgh_bits =
		MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(InHeadP->msgh_bits), 0);
	rep->hdr.msgh_remote_port = InHeadP->msgh_remote_port;
	rep->hdr.msgh_local_port = MACH_PORT_NULL;
	rep->hdr.msgh_kind = InHeadP->msgh_kind;
	rep->hdr.msgh_id = InHeadP->msgh_id + 100;

	if (InHeadP->msgh_id != BSD_REQ_MSG_ID) {
	    static mach_msg_type_t RetCodeType = {
	        /* msgt_name */			MACH_MSG_TYPE_INTEGER_32,
	     	/* msgt_size */			32,
		/* msgt_number */		1,
	    	/* msgt_inline */		TRUE,
	    	/* msgt_longform */		FALSE,
	    	/* msgt_deallocate */		FALSE,
		/* msgt_unused */		0
	    };
	    mig_reply_header_t *OutP = (mig_reply_header_t *)OutHeadP;
	    OutP->RetCodeType = RetCodeType;
	    OutP->RetCode = MIG_BAD_ID;
	    OutP->Head.msgh_size = sizeof *OutP;
	    return (FALSE);
	}

	rep->int_type = bsd_rep_int_type;
	rep->hdr.msgh_size = sizeof(struct bsd_reply);

	/*
	 * Set up server thread to handle process
	 */
	if ((rep->retcode = start_server_op(req->hdr.msgh_local_port,
					    req->syscode))
		!= KERN_SUCCESS)
	{
	    return (TRUE);
	}

	/*
	 * Save the reply msg and initialize current_output.
	 * The user_copy/user_reply_msg code uses them for copyout.
	 */
	p->p_reply_msg = &rep->hdr;
	p->p_current_size = 0;

	retval[0] = 0;
	retval[1] = req->rval2;

	/*
	 * Find the system call table descriptor for this call.
	 */
	if (syscode < 0) {
	    callp = &sysent[0];
	}
	else {
	    if (syscode >= nsysent)
		callp = &sysent[0];
	    else
		callp = &sysent[syscode];
	}

	unix_master();

	/*
	 * Catch any signals.  If no other error and not restartable,
	 * return EINTR.
	 */
#if 0
#if	SYSCALLTRACE
	    pid = p->p_pid;
	    if (syscalltrace &&
		    (syscalltrace == pid || syscalltrace < 0)) {

		register int	j;
		char		*cp;

		if (syscode >= nsysent ||
			syscode < 0)
		    printf("[%d]%d", pid, syscode);
		else
		    printf("[%d]%s", pid, syscallnames[syscode]);

		cp = "(";
		for (j = 0; j < callp->sy_narg; j++) {
		    printf("%s%x", cp, req->arg[j]);
		    cp = ", ";
		}
		if (j)
		    cp = ")";
		else
		    cp = "";
		printf("%s\n", cp);
	    }
#endif	SYSCALLTRACE
#endif 0

#if SSTRC
	if (p->p_flag & STRC && p->p_flag & SSTRC) {
	    /* Copy args to u. */
	    bcopy((char *)req->arg, (char *)p->p_addr->u_arg, 
		  callp->sy_narg*sizeof(int));
	}
#endif SSTRC
	    /*
	     * OSF1_SERVER: Do the system call with arguments on the stack
	     */
	    error = (*callp->sy_call)((struct proc *)req->hdr.msgh_local_port,
				      req->arg,
				      retval);

#if SSTRC
	if (p->p_flag & STRC && p->p_flag & SSTRC) {
	    /* Copy args to u. */
	    bcopy((char *)req->arg, (char *)p->p_addr->u_arg, 
		  callp->sy_narg*sizeof(int));
	}
#endif SSTRC

#if	MAP_UAREA
	if (p->p_share_lock_count) {
	    panic("Share lock still held", p->p_share_lock_count);
	    p->p_share_lock_count = 0;
	}
#endif	MAP_UAREA

	if (p->p_master_lock > 1) {
	    panic("Master still held", p->p_master_lock);
	    p->p_master_lock = 1;
	}

	if (p->p_master_lock < 1) {
	    panic("Master not held", p->p_master_lock);
	    p->p_master_lock = 1;
	}

	/* if exiting, deregister and short circut end_server_op */
	if (p->p_task == MACH_PORT_NULL) { 
	    proc = NULL;
	    server_thread_deregister(p);
	} else
	    proc = p;

	unix_release();

	/*
	 * Wrap up any trailing data in the reply message.
	 */
	if (proc) finish_reply_msg();

	rep->retcode = end_server_op(proc, error, &rep->interrupt);
	rep->rval[0] = retval[0];
	rep->rval[1] = retval[1];

#ifdef SYSCALLTRACE
	if (syscalltrace &&
		(syscalltrace == pid || syscalltrace < 0)) {
	    printf(" (%x,%x)", rep->rval[0], rep->rval[1]);
#if	0 
	    if (syscode >= nsysent ||
		    syscode < 0)
		printf("    [%d]%d", pid, syscode);
	    else
		printf("    [%d]%s", pid, syscallnames[syscode]);
	    printf(" returns %d", rep->retcode);
	    printf(" (%x,%x)", rep->rval[0], rep->rval[1]);
	    printf("%s\n",
		   (rep->interrupt) ? " Interrupt" : "");
#endif 0
	}
#endif	SYSCALLTRACE

	return (TRUE);
}

#if 0
/* 
 *  rpsleep - perform a resource pause sleep
 *
 *  rsleep = function to perform resource specific sleep
 *  arg1   = first function parameter
 *  arg2   = second function parameter
 *  mesg1  = first component of user pause message
 *  mesg2  = second component of user pause message
 *
 *  Display the appropriate pause message on the user's controlling terminal.
 *  Save the current non-local goto information and establish a new return
 *  environment to transfer here.  Invoke the supplied function to sleep
 *  (possibly interruptably) until the resource becomes available.  When the
 *  sleep finishes (either normally or abnormally via a non-local goto caused
 *  by a signal), restore the old return environment and display a resume
 *  message on the terminal.  The notify flag bit is set when the pause message
 *  is first printed.  If it is cleared on return from the function, the
 *  continue message is printed here.  If not, this bit will remain set for the
 *  duration of the polling process and the rpcont() routine will be called
 *  directly from the poller when the resource pause condition is no longer
 *  pending.
 *
 *  Return: true if the resource has now become available, or false if the wait
 *  was interrupted by a signal.
 */

boolean_t
rpsleep(rsleep, arg1, arg2, mesg1, mesg2)
int (*rsleep)();
int arg1;
int arg2;
char *mesg1;
char *mesg2;
{
    label_t lsave;
    boolean_t ret = TRUE;

    if ((u.u_rpswhich&URPW_NOTIFY) == 0)
    {
        u.u_rpswhich |= URPW_NOTIFY;
	uprintf("[%s: %s%s, pausing ...]\r\n", u.u_comm, mesg1, mesg2);
    }

    bcopy((caddr_t)&u.u_qsave, (caddr_t)&lsave, sizeof(lsave));
    if (setjmp(&u.u_qsave) == 0)
	(*rsleep)(arg1, arg2);
    else
	ret = FALSE;
    bcopy((caddr_t)&lsave, (caddr_t)&u.u_qsave, sizeof(lsave));

    if ((u.u_rpswhich&URPW_NOTIFY) == 0)
	rpcont();
    return(ret);
}


/* 
 *  rpcont - continue from resource pause sleep
 *
 *  Clear the notify flag and print the continuation message on the controlling
 *  terminal.  When this routine is called, the resource pause condition is no
 *  longer pending and we can afford to clear all bits since only the notify
 *  bit should be set to begin with.
 */

rpcont()
{
    u.u_rpswhich = 0;
    uprintf("[%s: ... continuing]\r\n", u.u_comm);
}


#endif 0

#if 0
nosys()
{
#ifndef	OSF1_SERVER
	if (u.u_signal[SIGSYS] == SIG_IGN || u.u_signal[SIGSYS] == SIG_HOLD)
	    u.u_error = EINVAL;
	psignal(u.u_procp, SIGSYS);
#else
	int error;
	if (u.u_signal[SIGSYS] == SIG_IGN || u.u_signal[SIGSYS] == SIG_HOLD)
	    error = EINVAL; 
	psignal3(u.u_procp, SIGSYS, FALSE);
	return(error);
#endif	OSF1_SERVER
}
#endif 0
