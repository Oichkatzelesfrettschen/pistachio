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
 * $Log:	bsd_server_side.c,v $
 * Revision 2.1  92/04/21  17:11:19  rwd
 * BSDSS
 * 
 *
 */

/*
 * Glue modified for OSF1_SERVER system calls.
 */

#include <uxkern/syscall_subr.h>
#include <uxkern/import_mach.h>
#include <uxkern/bsd_types.h>
#include <uxkern/syscalltrace.h>
#include <ufs/quota.h>
#include <ufs/inode.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <uxkern/proc_to_task.h>
#include <uxkern/syscalltrace.h>

/*
 * in sys_generic
 */
int
bsd_write_short(proc_port, interrupt, fileno, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	char *		data;
	unsigned int	data_count;
	int		*amount_written;	/* out */
{
	START_SERVER(SYS_write, 3)

	arg[0] = fileno;
	arg[1] = (int)data;
	arg[2] = (int)data_count;
	error = write((struct proc *)proc_port, arg, amount_written);

	END_SERVER(SYS_write)
}

int
bsd_write_long(proc_port, interrupt, fileno, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	char *		data;
	unsigned int	data_count;
	int		*amount_written;	/* out */
{
	START_SERVER(SYS_write, 3)

	arg[0] = fileno;
	arg[1] = (int)data;
	arg[2] = (int)data_count;
	error = write((struct proc *)proc_port, arg, amount_written);

	END_SERVER_DEALLOC(SYS_write, data, data_count)
}

int
bsd_select(proc_port, interrupt, nd, in_set, ou_set, ex_set,
	   in_valid, ou_valid, ex_valid, do_timeout, tv, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		nd;
	fd_set		*in_set;	/* in/out */
	fd_set		*ou_set;	/* in/out */
	fd_set		*ex_set;	/* in/out */
	boolean_t	in_valid;
	boolean_t	ou_valid;
	boolean_t	ex_valid;
	boolean_t	do_timeout;
	timeval_t	tv;
	int		*rval;		/* out */
{
	START_SERVER(SYS_select, 5)

	arg[0] = nd;
	arg[1] = (in_valid) ? (int)in_set : 0;
	arg[2] = (ou_valid) ? (int)ou_set : 0;
	arg[3] = (ex_valid) ? (int)ex_set : 0;
	arg[4] = (do_timeout) ? (int)&tv : 0;
	error = select((struct proc *)proc_port, arg, rval);

	END_SERVER(SYS_select)
}

#ifdef	COMPAT_43
/*
 * in uipc_syscalls
 */
int
bsd_send_short(proc_port, interrupt,
		fileno, flags, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	char		*data;
	unsigned int	data_count;
	int		*amount_written;	/* OUT */
{
	START_SERVER(SYS_osend, 4)

	arg[0] = fileno;
	arg[1] = (int)data;
	arg[2] = (int)data_count;
	arg[3] = flags;
	error = osend((struct proc *)proc_port, arg, amount_written);

	END_SERVER(SYS_osend)
}

int
bsd_send_long(proc_port, interrupt,
		fileno, flags, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	char		*data;
	unsigned int	data_count;
	int		*amount_written;	/* OUT */
{
	START_SERVER(SYS_osend, 4)

	arg[0] = fileno;
	arg[1] = (int)data;
	arg[2] = (int)data_count;
	arg[3] = flags;
	error = osend((struct proc *)proc_port, arg, amount_written);

	END_SERVER_DEALLOC(SYS_osend, data, data_count)
}
#endif	/* COMPAT_43 */

int
bsd_sendto_short(proc_port, interrupt, fileno, flags,
		to, tolen, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	char		*to;
	int		tolen;
	char		*data;
	unsigned int	data_count;
	int		*amount_written;	/* OUT */
{
	START_SERVER(SYS_sendmsg, 6)

	arg[0] = fileno;
	arg[1] = (int)data;
	arg[2] = (int)data_count;
	arg[3] = flags;
	arg[4] = (int)to;
	arg[5] = tolen;
	error = sendto((struct proc *)proc_port, arg, amount_written);

	END_SERVER(SYS_sendmsg)
}

int
bsd_sendto_long(proc_port, interrupt, fileno, flags,
		to, tolen, data, data_count, amount_written)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	char		*to;
	int		tolen;
	char		*data;
	unsigned int	data_count;
	int		*amount_written;	/* OUT */
{
	START_SERVER(SYS_sendmsg, 6)

	arg[0] = fileno;
	arg[1] = (int)data;
	arg[2] = (int)data_count;
	arg[3] = flags;
	arg[4] = (int)to;
	arg[5] = tolen;
	error = sendto((struct proc *)proc_port, arg, amount_written);

	END_SERVER_DEALLOC(SYS_sendmsg, data, data_count)
}

#ifdef	COMPAT_43
int
bsd_recvfrom_short(proc_port, interrupt, fileno, flags, len,
		from, fromlen, data, data_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	int		len;
	char		*from;		/* pointer to OUT array */
	int		*fromlen;	/* in/out */
	char		*data;		/* pointer to OUT array */
	unsigned int	*data_count;	/* out */
{
	START_SERVER(SYS_orecvfrom, 6)

	arg[0] = fileno;
	arg[1] = (int)data;
	arg[2] = len;
	arg[3] = flags;
	arg[4] = (int)from;
	arg[5] = (int)fromlen;

	error = orecvfrom((struct proc *)proc_port, arg, data_count);

	END_SERVER(SYS_orecvfrom)
}

int
bsd_recvfrom_long(proc_port, interrupt, fileno, flags, len,
		from, fromlen, data, data_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fileno;
	int		flags;
	int		len;
	char		*from;		/* pointer to OUT array */
	int		*fromlen;	/* in/out */
	char		**data;		/* out */
	unsigned int	*data_count;	/* out */
{
	START_SERVER(SYS_orecvfrom, 6)

	if (vm_allocate(mach_task_self(),
			(vm_offset_t *)data,
			len,
			TRUE)
	    != KERN_SUCCESS)
	{
	    error = ENOBUFS;
	} else {
	    arg[0] = fileno;
	    arg[1] = (int)*data;
	    arg[2] = len;
	    arg[3] = flags;
	    arg[4] = (int)from;
	    arg[5] = (int)fromlen;

	    error = orecvfrom((struct proc *)proc_port, arg, data_count);
	}

	/*
	 * Special end code to deallocate data if any error
	 */
        }
	
        unix_release();
        error = end_server_op(p, error, interrupt);
	if (error) {
	    (void) vm_deallocate(mach_task_self(),
				(vm_offset_t) *data,
				len);
	}
	return (error);
}
#endif	/* COMPAT_43 */

/*
 * in ufs_syscalls
 */
int
bsd_chdir(proc_port, interrupt, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
{
	START_SERVER(SYS_chdir, 1)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	error = chdir(proc_port, arg, 0);

	END_SERVER(SYS_chdir)
}

int
bsd_chroot(proc_port, interrupt, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
{
	START_SERVER(SYS_chroot, 1)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	error = chroot(proc_port, arg, 0);

	END_SERVER(SYS_chroot)
}

int
bsd_open(proc_port, interrupt, fname, fname_count, mode, crtmode, fileno)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	int		mode;
	int		crtmode;
	int		*fileno;	/* OUT */
{
	START_SERVER(SYS_open, 3)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	arg[1] = mode;
	arg[2] = crtmode;
	error = open((struct proc *)proc_port, arg, fileno);

	END_SERVER(SYS_open)
}

#ifdef	COMPAT_43
int
bsd_ocreat(proc_port, interrupt, fname, fname_count, fmode, fileno)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	int		fmode;
	int		*fileno;	/* OUT */
{
	START_SERVER(SYS_ocreat, 2)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	arg[1] = fmode;
	error = ocreat((struct proc *)proc_port, arg, fileno);
	END_SERVER(SYS_ocreat)
}
#endif	/* COMPAT_43 */

int
bsd_mknod(proc_port, interrupt, fname, fname_count, fmode, dev)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	int		fmode;
	int		dev;
{
	START_SERVER(SYS_mknod, 3)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	arg[1] = fmode;
	arg[2] = dev;
	error = mknod((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_mknod)
}

int
bsd_link(proc_port, interrupt, target, target_count, linkname, linkname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*target;
	unsigned int	target_count;
	char		*linkname;
	unsigned int	linkname_count;
{
	START_SERVER(SYS_link, 2)

	TRACE(("(%s,%s)",target,linkname));
	arg[0] = (int)target;
	arg[1] = (int)linkname;
	error = link((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_link)
}

int
bsd_symlink(proc_port, interrupt, target, target_count, linkname, linkname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*target;
	unsigned int	target_count;
	char		*linkname;
	unsigned int	linkname_count;
{
	START_SERVER(SYS_symlink, 2)

	TRACE(("(%s,%s)",target,linkname));
	arg[0] = (int)target;
	arg[1] = (int)linkname;
	error = symlink((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_symlink)
}

int
bsd_unlink(proc_port, interrupt, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
{
	START_SERVER(SYS_unlink, 1)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	error = unlink((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_unlink)
}

int
bsd_access(proc_port, interrupt, fname, fname_count, fmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	int		fmode;
{
	START_SERVER(SYS_access, 2)

	TRACE(("(%s,%o)",fname,fmode));
	arg[0] = (int)fname;
	arg[1] = fmode;
	error = saccess((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_access)
}

int
bsd_stat(proc_port, interrupt, follow, name, name_count, statr)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	follow;
	char		*name;
	unsigned int	name_count;
	register statb_t *statr;	/* out */
{
	struct stat	sb;
	int stat(),lstat();

	START_SERVER(follow ? SYS_stat : SYS_lstat, 2);

	TRACE(("(%s)",name));
	arg[0] = (int)name;
	arg[1] = (int)&sb;
	error = (follow?stat:lstat)(p, arg, 0);

	/* copy out stat buffer... */
	statr->s_dev	= sb.st_dev;
	statr->s_ino	= sb.st_ino;
	statr->s_mode	= sb.st_mode;
	statr->s_nlink	= sb.st_nlink;
	statr->s_uid	= sb.st_uid;
	statr->s_gid	= sb.st_gid;
	statr->s_rdev	= sb.st_rdev;
	statr->s_size	= sb.st_size;
	statr->s_atime	= sb.st_atime;
	statr->s_mtime	= sb.st_mtime;
	statr->s_ctime	= sb.st_ctime;
	statr->s_blksize	= sb.st_blksize;
	statr->s_blocks	= sb.st_blocks;
	statr->s_flags	= sb.st_flags;
	statr->s_gen	= sb.st_gen;

	END_SERVER(follow ? SYS_stat : SYS_lstat)
}

int
bsd_readlink(proc_port, interrupt, name, name_count, count, buf, bufCount)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*name;
	unsigned int	name_count;
	int		count;
	char		*buf;		/* pointer to OUT array */
	int		*bufCount;	/* out */
{
	START_SERVER(SYS_readlink, 3)

	TRACE(("(%s)",name));
	arg[0] = (int)name;
	arg[1] = (int)buf;
	arg[2] = count;
	error =	readlink((struct proc *)proc_port, arg, bufCount);

	END_SERVER(SYS_readlink)
}

int
bsd_chmod(proc_port, interrupt, fname, fname_count, fmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	int		fmode;
{
	START_SERVER(SYS_chmod, 2)

	TRACE(("(%s,%o)",fname,fmode));
	arg[0] = (int)fname;
	arg[1] = fmode;
	error =	chmod((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_chmod)
}

int
bsd_chown(proc_port, interrupt, fname, fname_count, uid, gid)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	int		uid;
	int		gid;
{
	START_SERVER(SYS_chown, 3)

	TRACE(("(%s,%d,%d)",fname,uid,gid));
	arg[0] = (int)fname;
	arg[1] = uid;
	arg[2] = gid;
	error =	chown((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_chown)
}

int
bsd_utimes(proc_port, interrupt, fname, fname_count, times)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	struct timeval	*times;
{
	START_SERVER(SYS_utimes, 2)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	arg[1] = (int)times;
	error = utimes((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_utimes)
}

int
bsd_truncate(proc_port, interrupt, fname, fname_count, length)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	int		length;
{
	START_SERVER(SYS_truncate, 2)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	arg[1] = length;
	error = truncate((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_truncate)
}

int
bsd_rename(proc_port, interrupt, from, from_count, to, to_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*from;
	unsigned int	from_count;
	char		*to;
	unsigned int	to_count;
{
	START_SERVER(SYS_rename, 2)

	TRACE(("(%s,%s)",from,to));
	arg[0] = (int)from;
	arg[1] = (int)to;
	error = rename((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_rename)
}

int
bsd_mkdir(proc_port, interrupt, name, name_count, dmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*name;
	unsigned int	name_count;
	int		dmode;
{
	START_SERVER(SYS_mkdir, 2)

	TRACE(("(%s,%o)",name,dmode));
	arg[0] = (int)name;
	arg[1] = dmode;
	error = mkdir((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_mkdir)
}

int
bsd_rmdir(proc_port, interrupt, name, name_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*name;
	unsigned int	name_count;
{
	START_SERVER(SYS_rmdir, 1)

	TRACE(("(%s)",name));
	arg[0] = (int)name;
	error = rmdir((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_rmdir)
}

int
bsd_fork(proc_port, interrupt, new_state, new_state_count, child_pid)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	thread_state_t	new_state;
	unsigned int	new_state_count;
	int		*child_pid;	/* OUT */
{
	START_SERVER(SYS_fork, 2)

	TRACE(("(%s)","fork"));
	arg[0] = (int)new_state;
	arg[1] = new_state_count;
	error = fork1((struct proc *)proc_port, arg, child_pid, 0);

	END_SERVER(SYS_fork)
}
int
bsd_vfork(proc_port, interrupt, new_state, new_state_count, child_pid)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	thread_state_t	new_state;
	unsigned int	new_state_count;
	int		*child_pid;	/* OUT */
{
	START_SERVER(SYS_fork, 2)

	TRACE(("(%s)","fork"));
	arg[0] = (int)new_state;
	arg[1] = new_state_count;
	error = fork1((struct proc *)proc_port, arg, child_pid, 1);

	END_SERVER(SYS_fork)
}

#if 0
/*
 * In cmu_syscalls
 */
int
bsd_xutimes(proc_port, interrupt, fname, fname_count, times)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	struct timeval	*times;
{
	START_SERVER(SYS_xutimes, 2)

	TRACE(("(%s)",fname));
	arg[0] = (int)fname;
	arg[1] = (int)times;
	xutimes();

	END_SERVER(SYS_xutimes)
}
#endif

#if 0
/*
 * in vfs_syscalls
 *
 * OSF1_SERVER: the (u)mount system calls go through the emul_generic interface
 * bsd_mount only supports local mounts and is kept here just in case...
 */
int
bsd_mount(proc_port, interrupt, type, dir, dir_count, flags, fspec, fspec_count, exflags, exroot)
											
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*dir;
	unsigned int	dir_count;
	int		flags;
	char		*fspec;
	unsigned int	fspec_count;
	int		exflags;
	uid_t		exroot;
{
	struct	ufs_args ua;

	START_SERVER(SYS_mount, 4)

	TRACE(("(%s,%s)", fspec, dir));
	arg[0] = (int)type;
	arg[1] = (int)dir;
	arg[2] = flags;
	arg[3] = &ua;

	ua.fspec = fspec;
	ua.exflags = exflags;
	ua.exroot = exroot;

	error = mount((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_mount)
}

int
bsd_umount(proc_port, interrupt, path, path_count, flags)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*path;
	unsigned int	path_count;
	int		flags;
{
	START_SERVER(SYS_unmount, 2)

	TRACE(("(%s)", path));
	arg[0] = (int)path;
	arg[1] = flags;
	error = unmount((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_unmount)
}

#endif 0
/*
 * in kern_acct
 */

int
bsd_acct(proc_port, interrupt, acct_on, fname, fname_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	acct_on;
	char		*fname;
	unsigned int	fname_count;
{
	START_SERVER(SYS_acct, 1)

	arg[0] = (acct_on) ? (int)fname : 0;
	error = sysacct((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_acct)
}

/*
 * More glue
 */
int
bsd_setgroups(proc_port, interrupt, gidsetsize, gidset)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	unsigned int	gidsetsize;
	int		*gidset;
{
	START_SERVER(SYS_setgroups, 2)

	arg[0] = gidsetsize;
	arg[1] = (int)gidset;

	error = setgroups((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_setgroups)
}

int
bsd_setrlimit(proc_port, interrupt, which, lim)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		which;
	struct rlimit	*lim;
{
	START_SERVER(SYS_setrlimit, 2)

	arg[0] = which;
	arg[1] = (int)lim;

	error = setrlimit((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_setrlimit)
}

#ifdef	COMPAT_43
int
bsd_osigvec(proc_port, interrupt, signo, have_nsv, nsv, osv, tramp)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* OUT */
	int		signo;
	boolean_t	have_nsv;
	struct sigvec	nsv;
	struct sigvec	*osv;		/* OUT */
	int		tramp;
{
	START_SERVER(SYS_osigvec, 4)

	arg[0] = signo;
	arg[1] = (have_nsv) ? (int)&nsv : 0;
	arg[2] = (int)osv;
	arg[3] = tramp;

	{ 
		int retval;
		error = osigvec((struct proc *)proc_port, arg, &retval);
	}

	END_SERVER(SYS_osigvec)
}
#endif	/* COMPAT_43 */

#if 0
int
bsd_signal(proc_port, interrupt, signo, handler, ohandler, tramp)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* OUT */
	int		signo;
	int		handler;
	int		*ohandler;	/* OUT */
	int		tramp;
{
	START_SERVER(SYS_signal, 3);

	arg[0] = signo;
	arg[1] = handler;
	arg[2] = tramp;

	error = ssig((struct proc *)proc_port, arg, ohandler);

	END_SERVER(SYS_signal)
}	

int
osf1_sigaction(proc_port, interrupt, signo, have_nsa, nsa, osa, tramp)
	mach_port_t		proc_port;
	boolean_t		*interrupt;	/* OUT */
	int			signo;
	boolean_t		have_nsa;
	struct sigaction	nsa;
	struct sigaction	*osa;	/* OUT */
	int			tramp;
{
	START_SERVER(SYS_sigaction, 4)

	arg[0] = signo;
	arg[1] = (have_nsa) ? (int)&nsa : 0;
	arg[2] = (int)osa;
	arg[3] = tramp;

	{
		int retval;
		error = sigaction((struct proc *)proc_port, arg, &retval);
	}

	END_SERVER(SYS_sigaction)
}

#endif 0
int
bsd_sigstack(proc_port, interrupt, have_nss, nss, oss)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	have_nss;
	struct sigstack	nss;
	struct sigstack	*oss;		/* OUT */
{
	START_SERVER(SYS_sigstack, 2)

	arg[0] = (have_nss) ? (int)&nss : 0;
	arg[1] = (int)oss;

	error = sigstack((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_sigstack)
}

int
bsd_settimeofday(proc_port, interrupt, have_tv, tv, have_tz, tz)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	have_tv;
	struct timeval	tv;
	boolean_t	have_tz;
	struct timezone	tz;
{
	START_SERVER(SYS_settimeofday, 2)

	arg[0] = (have_tv) ? (int)&tv : 0;
	arg[1] = (have_tz) ? (int)&tz : 0;

	error = settimeofday((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_settimeofday)
}

int
bsd_adjtime(proc_port, interrupt, delta, olddelta)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	struct timeval	delta;
	struct timeval	*olddelta;
{
	START_SERVER(SYS_adjtime, 2)

	arg[0] = (int)&delta;
	arg[1] = (int)olddelta;

	error = adjtime((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_adjtime)
}

int
bsd_setitimer(proc_port, interrupt, which, have_itv, itv, oitv)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		which;
	boolean_t	have_itv;
	struct itimerval itv;
	struct itimerval *oitv;		/* OUT */
{
	START_SERVER(SYS_setitimer, 3)

	arg[0] = which;
	arg[1] = (have_itv) ? (int)&itv : 0;
	arg[2] = (int)oitv;

	error = setitimer((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_setitimer)
}

int
bsd_sethostname(proc_port, interrupt, hostname, len)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*hostname;
	int		len;
{
	START_SERVER(SYS_sethostname, 2)

	arg[0] = (int)hostname;
	arg[1] = len;

	error = sethostname((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_sethostname)
}

int
bsd_bind(proc_port, interrupt, s, name, namelen)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	char		*name;
	int		namelen;
{
	START_SERVER(SYS_bind, 3)

	arg[0] = s;
	arg[1] = (int) name;
	arg[2] = namelen;

	error = bind((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_bind)
}

#ifdef	COMPAT_43
int
bsd_oaccept(proc_port, interrupt, s, name, anamelen, new_s)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	char		*name;		/* OUT */
	int		*anamelen;	/* OUT */
	int		*new_s;		/* OUT */
{
	START_SERVER(SYS_oaccept, 3)

	*anamelen = sizeof(sockarg_t);

	arg[0] = s;
	arg[1] = (int) name;
	arg[2] = (int) anamelen;

	error = oaccept((struct proc *)proc_port, arg, new_s);

	END_SERVER(SYS_oaccept)
}
#endif	/* COMPAT_43 */

int
bsd_connect(proc_port, interrupt, s, name, namelen)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	char		*name;
	int		namelen;
{
	START_SERVER(SYS_connect, 3)

	arg[0] = s;
	arg[1] = (int) name;
	arg[2] = namelen;

	error = connect((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_connect)
}

int
bsd_setsockopt(proc_port, interrupt, s, level, name, val, valsize)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	int		level;
	int		name;
	char		*val;
	int		valsize;
{
	START_SERVER(SYS_setsockopt, 5)

	arg[0] = s;
	arg[1] = level;
	arg[2] = name;
	arg[3] = (valsize > 0) ? (int)val : 0;
	arg[4] = valsize;

	error = setsockopt((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_setsockopt)
}

int
bsd_getsockopt(proc_port, interrupt, s, level, name, val, avalsize)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		s;
	int		level;
	int		name;
	char		*val;		/* OUT */
	int		*avalsize;	/* IN-OUT */
{
	START_SERVER(SYS_getsockopt, 5)

	*avalsize = sizeof(sockarg_t);

	arg[0] = s;
	arg[1] = level;
	arg[2] = name;
	arg[3] = (int)val;
	arg[4] = (int)avalsize;

	error = getsockopt((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_getsockopt)
}

#ifdef	COMPAT_43
int
bsd_ogetsockname(proc_port, interrupt, fdes, asa, alen)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fdes;
	char		*asa;		/* OUT */
	int		*alen;		/* OUT */
{
	START_SERVER(SYS_ogetsockname, 3)

	*alen = sizeof(sockarg_t);

	arg[0] = fdes;
	arg[1] = (int)asa;
	arg[2] = (int)alen;

	error = ogetsockname((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_ogetsockname)
}

int
bsd_ogetpeername(proc_port, interrupt, fdes, asa, alen)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		fdes;
	char		*asa;		/* OUT */
	int		*alen;		/* OUT */
{
	START_SERVER(SYS_ogetpeername, 3)

	*alen = sizeof(sockarg_t);

	arg[0] = fdes;
	arg[1] = (int)asa;
	arg[2] = (int)alen;

	error = ogetpeername((struct proc *)proc_port, arg, 0);

	END_SERVER(SYS_ogetpeername)
}
#endif	COMPAT_43

int
bsd_table_set(proc_port, interrupt, id, index, lel, nel,
		addr, count, nel_done)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* out */
	int		id;
	int		index;
	int		lel;
	int		nel;
	char		*addr;
	unsigned int	count;
	int		*nel_done;	/* out */
{
	START_SERVER(SYS_table, 5)

	arg[0] = id;
	arg[1] = index;
	arg[2] = (int)addr;
	arg[3] = nel;
	arg[4] = lel;

	error = table((struct proc *)proc_port, arg, nel_done);

	END_SERVER(SYS_table)
}

int
bsd_table_get(proc_port, interrupt, id, index, lel, nel,
		addr, count, nel_done)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* out */
	int		id;
	int		index;
	int		lel;
	int		nel;
	char		**addr;		/* out */
	unsigned int	*count;		/* out */
	int		*nel_done;	/* out */
{
	START_SERVER(SYS_table, 5)

	*count = nel * lel;
	if (vm_allocate(mach_task_self(),
			(vm_offset_t *)addr,
			*count,
			TRUE)
	    != KERN_SUCCESS)
	{
	    error = ENOBUFS;
	}
	else {
	    arg[0] = id;
	    arg[1] = index;
	    arg[2] = (int)*addr;
	    arg[3] = nel;
	    arg[4] = lel;

	    error = table((struct proc *)proc_port, arg, nel_done);
    	}


	/*
	 * Special end code to deallocate data if any error
	 */
	/* { for electric-c */
	}
        unix_release();
	error = end_server_op(p, error, interrupt);
	if (error) {
	    (void) vm_deallocate(mach_task_self(),
				 (vm_offset_t) *addr,
				 *count);
	    *count = 0;
	}

	return (error);
}

int
bsd_emulator_error(proc_port, error_message, size)
	mach_port_t	proc_port;
	char *		error_message;
	int		size;
{
	printf("emulator [%x] %s\n",proc_port, error_message);
	return KERN_SUCCESS;
}
int
bsd_readwrite(proc_port, interrupt, which, fileno, size, amount)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	boolean_t	which;
	int		fileno;
	int		size;
	int		*amount;
{
#if	MAP_UAREA
	if (which != 0 && which != 1)
		return (EINVAL);
	else {
		START_SERVER(which?SYS_write:SYS_read, 3)

		arg[0] = fileno;
		arg[1] = (int)uth->uu_procp->p_readwrite;
		arg[2] = (int)size;
		if (which)
			error = write((struct proc *)proc_port, arg, amount);
		else
			error = read((struct proc *)proc_port, arg, amount);

		END_SERVER(which?SYS_write:SYS_read)
	}
#else	MAP_UAREA
	return (EINVAL);
#endif	MAP_UAREA
}

int
bsd_share_wakeup(proc_port, offset)
	mach_port_t	proc_port;
	int		offset;
{
#if	MAP_UAREA
	register struct proc	*p;

	if ((p = port_to_proc_lookup(proc_port)) == (struct proc *)0)
		return (ESRCH);
#if	SYSCALLTRACE
	if (syscalltrace &&
		(syscalltrace == p->p_pid || syscalltrace < 0)) {
	    printf("[%d]bsd_share_wakeup(%x, %x)", p->p_pid, (int)p, offset);
	}
#endif
	mutex_lock(&master_mutex);
	wakeup((int)(p->p_shared_rw) + offset);
	mutex_unlock(&master_mutex);
	return (0);
#else MAP_UAREA
	return (EINVAL);
#endif MAP_UAREA
}

int bsd_maprw_request_it(proc_port, interrupt, fileno)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	register int	fileno;
{
#if	MACH_NBC && MAP_UAREA
	register struct inode *ip;

	START_SERVER_PARALLEL(1005, 1)
	ip = VTOI((struct vnode *)u.u_ofile[fileno]->f_data);
	user_request_it(ip, fileno, 0);
	END_SERVER_PARALLEL
#else	MACH_NBC && MAP_UAREA
	return (EINVAL);
#endif	MACH_NBC && MAP_UAREA
}

int bsd_maprw_release_it(proc_port, interrupt, fileno)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	register int	fileno;
{
#if	MACH_NBC && MAP_UAREA
	register struct inode *ip;
	START_SERVER_PARALLEL(1006, 1)
	ip = VTOI((struct vnode *)u.u_ofile[fileno]->f_data);
	user_release_it(ip);
	END_SERVER_PARALLEL
#else	MACH_NBC && MAP_UAREA
	return (EINVAL);
#endif	MACH_NBC && MAP_UAREA
}

int bsd_maprw_remap(proc_port, interrupt, fileno, offset, size)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	register int	fileno;
	int	offset;
	int	size;
{
#if	MACH_NBC && MAP_UAREA
	register struct inode *ip;
	START_SERVER_PARALLEL(1007, 3)
	ip = VTOI((struct vnode *)u.u_ofile[fileno]->f_data);
	user_remap_inode(ip, offset, size);
	END_SERVER_PARALLEL
#else	MACH_NBC && MAP_UAREA
	return (EINVAL);
#endif	MACH_NBC && MAP_UAREA
}

int
bsd_execve(proc_port, interrupt,
	   fname, fname_count,
	   cfname,
	   cfarg,
	   entry,
	   entry_count)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	unsigned int	fname_count;
	char		*cfname;	/* OUT */
	char		*cfarg;		/* OUT */
	int		*entry;		/* pointer to OUT array */
	unsigned int	*entry_count;	/* OUT */
{
	struct execr {				/* exec return arguments */
		char		*cfname;	/* shell file name */
		char		*cfarg;		/* shell args */
		int		*entry;		/* pointer to pc entry points */
		unsigned int	*entry_count;	/* number of entries */
	} rtv;

	START_SERVER(SYS_execve, 3)

	TRACE(("(%s)",fname));

	arg[0] = (int)fname;		/* file name for exec */
	arg[1] = 0;			/* handled in the emulator */
	arg[2] = 0;			/* handled in the emulator */

	rtv.cfname = cfname;
	rtv.cfarg = cfarg;
	rtv.entry = entry;
	rtv.entry_count = entry_count;

	error = execve((struct proc *)proc_port, arg, &rtv);

	END_SERVER(SYS_execve)
}
