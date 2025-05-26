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
 * $Log:	kern_exec.c,v $
 * Revision 2.3  92/07/08  15:48:55  mrt
 * 	Choose name of mach_servers directory based on whether we are
 * 	run as a second server or not.
 * 	[92/07/06            mrt]
 * 
 * Revision 2.2  92/05/25  14:43:16  rwd
 * 	Fix string overflow in strcpy of command name.
 * 	[92/05/24            rwd]
 * 
 * Revision 2.1  92/04/21  17:13:20  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1992 William Jolitz. All rights reserved.
 * Written by William Jolitz 1/92
 *
 * Redistribution and use in source and binary forms are freely permitted
 * provided that the above copyright notice and attribution and date of work
 * and this paragraph are duplicated in all such forms.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * This procedure implements a minimal program execution facility for
 * 386BSD. It interfaces to the BSD kernel as the execve system call.
 * Significant limitations and lack of compatiblity with POSIX are
 * present with this version, to make its basic operation more clear.
 *
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/seg.h>
#include <sys/file.h>
#include <sys/acct.h>
#include <sys/exec.h>
#include <sys/stat.h>
#include <sys/ktrace.h>
#include <sys/resourcevar.h>
#include <sys/synch.h>
#include <sys/parallel.h>

#include <sys/mman.h>
#include <uxkern/import_mach.h>
#include <sys/signalvar.h>
#include <sys/kinfo_proc.h>

int load_cmu_binaries = 1;

extern char *trampoline_page;

struct execa {
    char	*fname;
    char	**argp;
    char	**envp;
};

struct execr {				/* exec return arguments */
    char		*cfname;	/* shell file name */
    char		*cfarg;		/* shell args */
    int		*entry;		/* pointer to pc entry points */
    unsigned int	*entry_count;	/* number of entries */
};

/*
 * exec system call
 */
/* ARGSUSED */
execve(p, uap, retval)
    register struct proc *p;
    register struct args {
	char	*fname;
	char	**argp;
	char	**envp;
    } *uap;
    struct execr *retval;
{
    return execve_prim(p, uap, retval, FALSE);
}

execve_prim(p, uap, retval, emulator)
    register struct proc *p;
    register struct execa *uap;
    struct execr *retval;
    boolean_t emulator;
{
    int rv, amt;
    struct nameidata nd;
    register struct nameidata *ndp = &nd;
    struct exec hdr;
    char *cp;
    struct stat statb;

    int indir = 0;
    register struct pcred *pcred = p->p_cred;
    register struct ucred *cred ;
    struct vnode *vp;
    struct vattr vattr;
    uid_t uid;
    gid_t gid;
    char *shellname;
    union {
	char	ex_shell[MAXINTERP];	/* #! and interpreter name */
	struct	exec ex_exec;
    } exdata;

    retval->cfname[0] = '\0';
    retval->cfarg[0] = '\0';

    /*
     * Step 1. Lookup filename to see if we have something to execute.
     */
    bzero(ndp, sizeof(struct nameidata));
    ndp->ni_nameiop = LOOKUP | FOLLOW | LOCKLEAF | SAVENAME;
    ndp->ni_segflg = UIO_USERSPACE;
    ndp->ni_dirp = uap->fname;

    /* is it there? */
    if (rv = namei(ndp, p))
	return (rv);

    vp = ndp->ni_vp;
    cred = pcred->pc_ucred;
    uid = pcred->p_ruid;		/* get orginal uid/gid */
    gid = pcred->p_rgid;
    if (rv = VOP_GETATTR(vp, &vattr, cred, p))
	goto exec_fail;
    if (vp->v_mount->mnt_flag & MNT_NOEXEC) {	/* no exec on fs ? */
	rv = EACCES;
	goto exec_fail;
    }
    if ((vp->v_mount->mnt_flag & MNT_NOSUID) == 0) {
	if (vattr.va_mode & VSUID)	/* check for SUID */
	    uid = vattr.va_uid;
	if (vattr.va_mode & VSGID)	/* check for SGID */
	    gid = vattr.va_gid;
    }

  again:
    /* is it a regular file? */
    if (vp->v_type != VREG) {
	vput(vp);
	return(ENOEXEC);
    }

    /* is it executable? */
    if (rv = VOP_ACCESS(vp, VEXEC, cred, p))
	goto exec_fail;

    if ((p->p_flag & STRC) && (rv = VOP_ACCESS(vp, VREAD, cred, p)))
	goto exec_fail;

    rv = vn_stat(vp, &statb, p);
    if (rv)
	goto exec_fail;

    /*
     * Step 2. Does the file contain a format we can
     * understand and execute
     */
    /*
     * Read in first few bytes of file for segment sizes, magic number:
     *	ZMAGIC = demand paged RO text
     * Also an ASCII line beginning with #! is
     * the file name of a ``shell'' and arguments may be prepended
     * to the argument list if given here.
     */

    exdata.ex_shell[0] = '\0';	/* for zero length files */

    rv = vn_rdwr(UIO_READ, vp, (caddr_t)&exdata, sizeof (exdata),
		    (off_t)0, UIO_SYSSPACE, (IO_UNIT|IO_NODELOCKED), 
		    cred, &amt, p);
    if (rv)
	goto exec_fail;

    if (amt > sizeof(exdata) - sizeof(exdata.ex_exec) &&
	exdata.ex_shell[0] != '#') {
	rv = ENOEXEC;
	goto exec_fail;
    }

    /* that we recognize? */
    rv = ENOEXEC;

    if ((int)exdata.ex_exec.a_magic == ZMAGIC) {
	
	/* sanity check  "ain't not such thing as a sanity clause" -groucho */
	if (exdata.ex_exec.a_text > MAXTSIZ || exdata.ex_exec.a_text > statb.st_size)
	    goto exec_fail;
	
	if (exdata.ex_exec.a_data == 0 || exdata.ex_exec.a_data > DFLDSIZ
	    || exdata.ex_exec.a_data > statb.st_size
	    || exdata.ex_exec.a_data + exdata.ex_exec.a_text > statb.st_size)
	    goto exec_fail;
	
	if (exdata.ex_exec.a_bss > MAXDSIZ)
	    goto exec_fail;
	
	if (exdata.ex_exec.a_text + exdata.ex_exec.a_data + exdata.ex_exec.a_bss > MAXTSIZ + MAXDSIZ)
	    goto exec_fail;
    } else {
	if (exdata.ex_shell[0] != '#' ||
	    exdata.ex_shell[1] != '!' ||
	    indir) {
	    rv = ENOEXEC;
	    goto exec_fail;
	}
	for (cp = &exdata.ex_shell[2];; ++cp) {
	    if (cp >= &exdata.ex_shell[MAXINTERP]) {
		rv = ENOEXEC;
		goto exec_fail;
	    }
	    if (*cp == '\n') {
		*cp = '\0';
		break;
	    }
	    if (*cp == '\t')
		*cp = ' ';
	}
	cp = &exdata.ex_shell[2];	/* get shell interpreter name */
	while (*cp == ' ')
	    cp++;
	shellname = ndp->ni_dirp = cp;
	while (*cp && *cp != ' ')
	    cp++;
	retval->cfarg[0] = '\0';
	if (*cp) {
	    *cp++ = '\0';
	    while (*cp == ' ')
		cp++;
	    if (*cp)
		bcopy((caddr_t)cp, (caddr_t)retval->cfarg, MAXINTERP);
	}
	indir = 1;	/* indicate this is a script file */
	vput(vp);	/* find shell interpreter */
	ndp->ni_nameiop = LOOKUP | FOLLOW | LOCKLEAF | SAVENAME;
	ndp->ni_segflg = UIO_SYSSPACE;
	if (rv = namei(ndp, p)) {
	    return (rv);
	}
	vp = ndp->ni_vp;
	if (rv = VOP_GETATTR(vp, &vattr, cred, p))
	    goto exec_fail;
	strncpy((caddr_t)retval->cfname, (caddr_t)ndp->ni_pnbuf, MAXCOMLEN);
	uid = pcred->p_ruid;
	gid = pcred->p_rgid;
	goto again;
    }
    rv = getxfile(p, vp, &exdata.ex_exec, uid, gid, emulator);
    if (rv) {
	goto exec_fail;
    }
    vput(vp);
    vp = NULL;

    set_entry_address(&exdata.ex_exec, retval->entry, retval->entry_count);

    fdcloseexec(p);
    execsigs(p);

    p->p_acflag &= ~AFORK;
    if (indir)
	strncpy((caddr_t)p->p_comm, (caddr_t)retval->cfname, MAXCOMLEN);
    else
	strncpy((caddr_t)p->p_comm, (caddr_t)ndp->ni_pnbuf, MAXCOMLEN);
exec_fail:
    if (vp)
	vput(vp);
    if (ndp->ni_nameiop & HASBUF)
	FREE(ndp->ni_pnbuf, M_NAMEI);
    return (rv);
}

/*
 * Read in and set up memory for executed file.
 */
getxfile(p, vp, ep, uid, gid, emulator)
	register struct proc *p;
	register struct vnode *vp;
	register struct exec *ep;
	int uid, gid, emulator;
{
	register struct ucred *cred = p->p_ucred;
	int error, resid;
	vm_offset_t text_size, data_size, bigsize;
	vm_offset_t text_start, data_start, size;
	vm_offset_t data_end, copy_end;
	int on_master = TRUE;
	vm_offset_t addr;
	vm_size_t copy_size;
	kern_return_t kr;
	int cmu_binary;

	if (p->p_master_lock == 0)
	    panic("getxfile: no master lock");

	if (ep->a_text != 0 && (vp->v_flag & VTEXT) == 0 &&
	    vp->v_usecount != 1) {
		register struct file *fp;

		for (fp = filehead; fp; fp = fp->f_filef) {
			if (fp->f_type == DTYPE_VNODE &&
			    fp->f_count > 0 &&
			    (struct vnode *)fp->f_data == vp &&
			    (fp->f_flag & FWRITE)) {
				return (ETXTBSY);
			}
		}
	}

	if ((p->p_flag & SPPWAIT)) {
		p->p_flag &= ~SPPWAIT;
		wakeup((caddr_t)p->p_pptr);
	}

	p->p_flag |= SEXEC;

	/*
	 * Deallocate the entire address space for the task,
	 * except for the area holding the emulator.
	 */
	(void) vm_deallocate(p->p_task,
			     VM_MIN_ADDRESS,
			     (vm_size_t)((vm_offset_t)EMULATOR_BASE - VM_MIN_ADDRESS));

	(void) vm_deallocate(p->p_task,
			     EMULATOR_END,
			     (vm_size_t)(VM_MAX_ADDRESS - EMULATOR_END));

	cmu_binary = load_cmu_binaries&&ep->a_entry;
	if (emulator)
	    text_start = EMULATOR_BASE;
	else if (cmu_binary)
	    text_start = 0x10000;
	else
	    text_start = USRTEXT;
	text_size = round_page(ep->a_text);
	data_start = text_start + text_size;
	data_size = round_page(ep->a_data + ep->a_bss);
	size = round_page(ep->a_text + ep->a_data);
	bigsize = round_page(ep->a_text + ep->a_data + ep->a_bss);

	/*
	 *	Remember data starting point
	 */
	p->p_vmspace->vm_daddr = (caddr_t) data_start;

	error = 0;

	addr = text_start;
	if ((kr = vm_allocate(p->p_task, &addr, bigsize, 
			      FALSE)) != KERN_SUCCESS) {
	    printf("Cannot find space for exec image %x.\n",kr);
	    goto suicide;
	}

	if ((kr = vm_allocate(mach_task_self(), &addr, 
			      size, TRUE)) != KERN_SUCCESS) {
	    printf("vm_allocate failure %x\n",kr);
	    goto suicide;
	}

	/*
	 *  407 images have text and data contiguous
	 */
	error = vn_rdwr(UIO_READ, vp,
			(caddr_t)addr,
			ep->a_text+ep->a_data+
			((emulator||cmu_binary)?sizeof(struct exec):0),
			(off_t)((emulator||cmu_binary)?0:NBPG),
			UIO_SYSSPACE, (IO_UNIT|IO_NODELOCKED),
			cred, &resid, p);
	if (error == 0)
	    error = vm_write(p->p_task, text_start, addr,
			     round_page(ep->a_text+ep->a_data));
	if (error == 0) {
	    (void) vm_protect(p->p_task,
			      text_start,
			      trunc_page(ep->a_text),
			      FALSE,
			      VM_PROT_READ|VM_PROT_EXECUTE);
	    if (error)
		printf("vm_protect failure %x\n",error);
	}
	(void) vm_deallocate(mach_task_self(), addr, size);
	if (error) {
	    printf("kern_exec: reading in file %d\n",error);
	    goto suicide;
	}

	/*
	 *	Create the stack.
	 */

	size = round_page(p->p_vmspace->vm_ssize*NBPG);
	addr = round_page((vm_offset_t)USRSTACK) - size;

	if (vm_allocate(p->p_task, &addr, size, FALSE) != KERN_SUCCESS) {
	    printf("Cannot find space for stack.\n");
	    goto suicide;
	}

	kr = vm_write(p->p_task, trunc_page(USRSTACK - TRAMPOLINE_MAX_SIZE),
		      (vm_offset_t) trampoline_page, NBPG);
	if (kr != KERN_SUCCESS)
	    panic("vm_write trampoline",kr);

	if (!on_master) {
	    unix_master();
	    VOP_LOCK(vp);
	}

	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((p->p_flag&STRC)==0) {
		if (uid != cred->cr_uid || gid != cred->cr_gid)
			p->p_ucred = cred = crcopy(cred);
		cred->cr_uid = uid;
		cred->cr_gid = gid;
	} else
		psignal(p, SIGTRAP);
	p->p_cred->p_svuid = cred->cr_uid;
	p->p_cred->p_svgid = cred->cr_gid;
	p->p_vmspace->vm_tsize = text_size/NBPG;
	p->p_vmspace->vm_dsize = data_size/NBPG;
/*	u.u_prof.pr_scale = 0;*/
	return (0);

suicide:
	if (!on_master) {
	    unix_master();
	    VOP_LOCK(vp);
	}
	return (EINVAL);
}

void
copy_out_args_and_stack(p, cp, nc, na, ne, indir, new_arg_addr)
        struct proc *p;
	register char *	cp;	/* argument and environment strings */
	register int	nc;	/* total size of strings */
	register int	na;	/* number of arguments + number of env. */
	int		ne;	/* number of environment entries */
	char **		indir;	/* if not NULL, pointer to script args */
	vm_offset_t	*new_arg_addr;
				/* argument address in user code (OUT) */
{
        kern_return_t kr;
	register char **k_ap;	/* kernel arglist address */
	register char *	u_cp;	/* user argument string address */
	register char *	k_cp;	/* kernel argument string address */
	vm_offset_t	u_arg_start;
				/* user start of argument list block */
	vm_offset_t	k_arg_start;
				/* kernel start of argument list block */
	vm_size_t	arg_size;
				/* size of argument list block */
	vm_offset_t	u_arg_page_start;
				/* user start of args, page-aligned */
	vm_size_t	arg_page_size;
				/* page_aligned size of args */
	vm_offset_t	k_arg_page_start;
				/* kernel start of args, page-aligned */

	/*
	 * Ask machine-dependent code for argument list address
	 */
	set_arg_addr(na*NBPW + 3*NBPW + nc + NBPW,
		     &u_arg_start,
		     &arg_size);

	/*
	 * Round to page boundaries, and allocate kernel copy.
	 */
	u_arg_page_start = trunc_page(u_arg_start);
	arg_page_size = (vm_size_t)(round_page(u_arg_start + arg_size)
					- u_arg_page_start);

	(void) vm_allocate(mach_task_self(),
			   &k_arg_page_start,
			   (vm_size_t)arg_page_size,
			   TRUE);

	k_arg_start = k_arg_page_start + (u_arg_start - u_arg_page_start);

	k_ap = (char **)k_arg_start;
	u_cp = (char *)u_arg_start + na*NBPW + 3*NBPW;
	k_cp = (char *)k_arg_start + na*NBPW + 3*NBPW;

	*k_ap++ = (char *)(na - ne);	/* set number of arguments */

	for (;;) {
	    int	len;

	    if (na == ne)
		*k_ap++ = 0;

	    if (--na < 0)
		break;

	    *k_ap++ = u_cp;
	    if (indir && *indir) {
		(void) copystr(*indir++, k_cp, (unsigned)nc, (u_int *) &len);
	    }
	    else {
		(void) copystr(cp, k_cp, (unsigned)nc, (u_int *) &len);
		cp += len;
	    }
	    u_cp += len;
	    k_cp += len;
	    nc -= len;
	}
	*k_ap = 0;

	/*
	 * Write the argument list out to user space.
	 */
	kr = vm_write(p->p_task,
			u_arg_page_start,
			(pointer_t)k_arg_page_start,
			arg_page_size);
	if (kr != KERN_SUCCESS)
	    panic("vm_write in copy_out_args\n",kr);
	(void) vm_deallocate(mach_task_self(),
			     k_arg_page_start,
			     arg_page_size);

	/*
	 * Return the arg-list address.
	 */
	*new_arg_addr = u_arg_start;
}


/*
 * Load the system call emulator, and tell it to exec() the first
 * program.
 */
#include <sys/reboot.h>

char		*emulator_name;
char		*init_program_name;

char		default_arg_1[] = "-sa\0";
char		*(default_program_args[]) = {default_arg_1, 0};

				/* NOTE: Following may be changed by
				 *       starting server with excess args:
				 */
char		**init_program_args = default_program_args;

struct execa	init_exec_args;
int		init_attempts = 0;

load_emulator(target_proc)
	struct proc *target_proc;
{
	int		*old_ap;
	int		error = 0;
	vm_offset_t	new_arg_addr;

	struct execr	retval;		/* Following needed to call execve: */
	int		entry[4];
	unsigned int	entry_count;
	char		cfname[256];
	char		cfarg[256];

        struct proc *p = (struct proc *)cthread_data(cthread_self());
#if SECOND_SERVER
	if (second_server) {
	    emulator_name = "/mach_servers44/emulator";
	    init_program_name = "/mach_servers44/bsd_init";
	} else
	{
	    emulator_name = "/mach_servers/emulator";
	    init_program_name = "/mach_servers/bsd_init";
	}
#else /* SECOND_SERVER */
	emulator_name = "/mach_servers/emulator";
	init_program_name = "/mach_servers/bsd_init";
#endif /* SECOND_SERVER */

	unix_master();

        retval.cfname = cfname;
        retval.cfarg = cfarg;
        retval.entry = entry;
        retval.entry_count = &entry_count;

#ifndef	mips
	default_arg_1[1] = (boothowto & RB_SINGLE) ? 's' : 'x';
	default_arg_1[2] = (boothowto & RB_ASKNAME) ? 'a' : 'x';
#endif

        /*
         * Temporarily become the process and the calling thread.
         */
	cthread_set_data(cthread_self(), (any_t)target_proc);

	target_proc->p_master_lock = p->p_master_lock;

        /*
         * Prepare for loading the emulator.
         */
        emul_init_process(target_proc);

	do {
		if (init_attempts == 2)
			panic("Can't load emulator (%s)", emulator_name);
		init_attempts++;

		if (error) {
			printf("Load of %s failed, errno %d\n",
					emulator_name, error);
			error = 0;
			boothowto |= RB_INITNAME;
		}

		/*
		 *	Set up argument block for fake call to execve.
		 */

		init_exec_args.fname = emulator_name;
		init_exec_args.argp = 0;   /* Args are set after exec, below */
		init_exec_args.envp = 0;

		ASSERT(p->p_cred != NOCRED);
		error = execve_prim(target_proc, (void *) &init_exec_args,
				    &retval,TRUE);
        } while (error);

        /*
         * Copy out arglist
         */
        {
	    char	  arg_buf[256];
            register char *cp;
            register int  len;

            cp = arg_buf;
            len = strlen(init_program_name) + 1;
            bcopy(init_program_name, cp, len);
            cp += len;

            len = strlen(init_program_args[0]) + 1;
            bcopy(init_program_args[0], cp, len);
            cp += len;

            copy_out_args_and_stack(target_proc,
				    arg_buf,
                                    cp - arg_buf,
                                    2,
                                    0,
                                    (char **)0,
                                    &new_arg_addr);
        }


	/*
	 * Set initial user registers.  Must do all the stuff that
	 * exec in the emulator normally does.
	 */
		set_emulator_state(entry, entry_count, new_arg_addr);

        /*
         * Restore our process ID.
         */
        cthread_set_data(cthread_self(), (any_t)p);

	p->p_master_lock = target_proc->p_master_lock;

	target_proc->p_master_lock = 0;

        unix_release();

	return error;
}
