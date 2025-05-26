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
 * $Log:	bsd_user_side.c,v $
 * Revision 2.1  92/04/21  17:10:38  rwd
 * BSDSS
 * 
 * Revision 2.1  92/04/21  17:08:14  rwd
 * BSDSS
 * 
 *
 */

/*
 * Glue routines between traps and MiG interfaces.
 */

#include <mach_init.h>
#include <mach/mig_errors.h>
#include <uxkern/bsd_1.h>
#include <uxkern/bsd_msg.h>
#include <varargs.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/errno.h>
#if VICE
#include <sys/viceioctl.h>
#endif

#ifdef	MAP_UAREA
#include <sys/ushared.h>

extern int shared_enabled;
extern struct ushared_ro *shared_base_ro;
extern struct ushared_rw *shared_base_rw;
extern char *shared_readwrite;
extern int readwrite_inuse;
extern spin_lock_t readwrite_lock;
#endif	MAP_UAREA

extern	mach_port_t	our_bsd_server_port;

#define	DEBUG 0

#if	DEBUG
#define	EPRINT(a) e_emulator_error/**/a/**/
#else	DEBUG
#define	EPRINT(a)
#endif	DEBUG

/*
 * Copy zero-terminated string and return its length,
 * including the trailing zero.  If longer than max_len,
 * return -1.
 */
int
copystr(from, to, max_len)
	register char	*from;
	register char	*to;
	register int	max_len;
{
	register int	count;

	count = 0;
	while (count < max_len) {
	    count++;
	    if ((*to++ = *from++) == 0) {
		return (count);
	    }
	}
	return (-1);
}

int
copy_args(argp, arg_count, arg_addr, arg_size, char_count)
	register char	**argp;
	int		*arg_count;	/* OUT */
	vm_offset_t	*arg_addr;	/* IN/OUT */
	vm_size_t	*arg_size;	/* IN/OUT */
	unsigned int	*char_count;	/* IN/OUT */
{
	register char		*ap;
	register int		len;
	register unsigned int	cc = *char_count;
	register char		*cp = (char *)*arg_addr + cc;
	register int		na = 0;

	while ((ap = *argp++) != 0) {
	    na++;
	    while ((len = copystr(ap, cp, *arg_size - cc)) < 0) {
		/*
		 * Allocate more
		 */
		vm_offset_t	new_arg_addr;

		if (vm_allocate(mach_task_self(),
				&new_arg_addr,
				(*arg_size) * 2,
				TRUE) != KERN_SUCCESS)
		    return (E2BIG);
		(void) vm_copy(mach_task_self(),
				*arg_addr,
				*arg_size,
				new_arg_addr);
		(void) vm_deallocate(mach_task_self(),
				*arg_addr,
				*arg_size);
		*arg_addr = new_arg_addr;
		*arg_size *= 2;

		cp = (char *)*arg_addr + cc;
	    }
	    cc += len;
	    cp += len;
	}
	*arg_count = na;
	*char_count = cc;
	return (0);
}

int
e_exec_call(serv_port, interrupt,
		fname, argp, envp, new_arg_addr, entry, entry_count)
	mach_port_t	serv_port;
	boolean_t	*interrupt;	/* OUT */
	char		*fname;
	char		**argp;
	char		**envp;
	vm_offset_t	*new_arg_addr;	/* OUT */
	vm_offset_t	*entry;		/* pointer to OUT array */
	unsigned int	*entry_count;	/* OUT */
{
	vm_offset_t	arg_addr;
	vm_size_t	arg_size;
	int		arg_count, env_count;
	unsigned int	char_count = 0;
	int		error;
	vm_offset_t	arg_start;
	cfname_t	cfname;
	cfname_t	cfarg;
	path_name_t	save_fname;

	/*
	 * Copy the argument and environment strings into
	 * contiguous memory.  Since most argument lists are
	 * small, we allocate a page to start, and add more
	 * if we need it.
	 */
	arg_size = vm_page_size;
	(void) vm_allocate(mach_task_self(),
			   &arg_addr,
			   arg_size,
			   TRUE);

	if (argp) {
	    if (copy_args(argp, &arg_count,
			&arg_addr, &arg_size, &char_count) != 0)
		return (E2BIG);
	}
	else {
	    arg_count = 0;
	}

	if (envp) {
	    if (copy_args(envp, &env_count,
			&arg_addr, &arg_size, &char_count) != 0)
		return (E2BIG);
	}
	else {
	    env_count = 0;
	}

	/*
	 * Save the file name in case a command file needs it.
	 * (The file name is in the old program address space,
	 * and will disappear if the exec is successful.)
	 */
	strcpy(save_fname, fname);

	/*
	 * Exec the program.  Get back the command file name (if any),
	 * and the entry information (machine-dependent).
	 */
	error = bsd_execve(serv_port,
			interrupt,
			save_fname, strlen(save_fname) + 1,
			cfname,
			cfarg,
			entry,
			&entry_count);
	if (error) {
	    (void) vm_deallocate(mach_task_self(), arg_addr, arg_size);
	    return (error);
	}

	/*
	 * Set up new argument list.  If command file name and argument
	 * have been found, use them instead of argv[0].
	 */
	{
	    register char	**ap;
	    register char	*cp;
	    register char	*argstrings = (char *)arg_addr;
	    register int	total_args;
	    register int	len;
	    char		*cmd_args[4];
	    register char	**xargp = 0;

	    total_args = arg_count + env_count;
	    if (cfname[0] != '\0') {
		/*
		 * argv[0] becomes 'cfname'; skip real argv[0].
		 */
		len = strlen(argstrings) + 1;
		argstrings += len;
		char_count -= len;

		xargp = cmd_args;
		*xargp++ = cfname;
		char_count += (strlen(cfname) + 1);

		if (cfarg[0] != '\0') {
		    *xargp++ = cfarg;
		    char_count += (strlen(cfarg) + 1);
		    total_args++;
		}
		*xargp++ = save_fname;
		char_count += (strlen(save_fname) + 1);
		total_args++;

		*xargp = 0;
		xargp = cmd_args;
	    }
	    char_count = (char_count + NBPW - 1) & ~(NBPW - 1);

	    arg_start = set_arg_addr(total_args*NBPW + 3*NBPW + char_count
					 + NBPW);
	    ap = (char **)arg_start;
	    cp = (char *)arg_start + total_args*NBPW + 3*NBPW;

	    *ap++ = (char *)(total_args - env_count);
	    for (;;) {

		if (total_args == env_count)
		    *ap++ = 0;
		if (--total_args < 0)
		    break;
		*ap++ = cp;
		if (xargp && *xargp)
		    len = copystr(*xargp++, cp, (unsigned)char_count);
		else {
		    len = copystr(argstrings, cp, (unsigned)char_count);
		    argstrings += len;
		}
		cp += len;
		char_count -= len;
	    }
	    *ap = 0;
	}

#ifdef	STACK_GROWTH_UP
	*new_arg_addr = ((vm_offset_t) cp + NBPW - 1) & ~(NBPW - 1);
#else	STACK_GROWTH_UP
	*new_arg_addr = arg_start;
#endif	STACK_GROWTH_UP

	(void) vm_deallocate(mach_task_self(), arg_addr, arg_size);
	return (error);
}

int
e_getrusage(serv_port, interrupt, which, rusage, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;	/* OUT */
	int		which;
	register
	struct rusage	*rusage;
	int		*rval;
{
	register int	error;
	struct thread_basic_info bi;
	unsigned int	bi_count;

	error = bsd_getrusage(serv_port, interrupt, which, rusage);
	if (error || which != RUSAGE_SELF)
	    return (error);

	bi_count = THREAD_BASIC_INFO_COUNT;
	(void) thread_info(mach_thread_self(),
			THREAD_BASIC_INFO,
			(thread_info_t)&bi,
			&bi_count);

	rusage->ru_utime.tv_sec  = bi.user_time.seconds;
	rusage->ru_utime.tv_usec = bi.user_time.microseconds;
	rusage->ru_stime.tv_sec  = bi.system_time.seconds;
	rusage->ru_stime.tv_usec = bi.system_time.microseconds;

	return (0);
}

int
e_write(serv_port, interrupt, fileno, data, count, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;	/* out */
	int		fileno;
	char		*data;
	unsigned int	count;
	int		*rval;
{
	extern int	bsd_write_short();
	extern int	bsd_write_long();

#ifdef	MAP_UAREA
#ifdef	MAP_FILE
	int result = 0;
	if (shared_enabled &&
	    e_maprw(serv_port, interrupt, fileno, data,
		    count, rval, 1, &result))
	    return (result);
	else
#endif	MAP_FILE
	if (shared_enabled && count <= vm_page_size) {
	    spin_lock(&readwrite_lock);
	    if (readwrite_inuse) {
		spin_unlock(&readwrite_lock);
		goto server;
	    }
	    readwrite_inuse = 1;
	    spin_unlock(&readwrite_lock);
	    return e_readwrite(serv_port, interrupt, fileno, data, 
			       count, rval, 1, 1);
	}
server:
	if (result != 0) {
	    EPRINT(("e_write(%d, %x, %x)\n",fileno, data, count));
	    EPRINT(("shared = %x, res = %x, inuse = %x\n",shared_enabled, result, readwrite_inuse));
	}
#endif MAP_UAREA
	return (((count <= SMALL_ARRAY_LIMIT) ? bsd_write_short
					      : bsd_write_long
		)(serv_port,
		  interrupt,
		  fileno,
		  data,
		  count,
		  &rval[0])
	       );
}

#ifdef	COMPAT_43
int
e_send(serv_port, interrupt, fileno, data, count, flags, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		fileno;
	char		*data;
	unsigned int	count;
	int		flags;
	int		*rval;
{
	extern int	bsd_send_short();
	extern int	bsd_send_long();

	return (((count <= SMALL_ARRAY_LIMIT) ? bsd_send_short
					      : bsd_send_long
		)(serv_port,
		  interrupt,
		  fileno,
		  flags,
		  data,
		  count,
		  &rval[0]));
}
#endif	/* COMPAT_43 */

int
e_sendto(serv_port, interrupt, fileno, data, count, flags, to, tolen, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		fileno;
	char		*data;
	unsigned int	count;
	int		flags;
	char		*to;
	int		tolen;
	int		*rval;
{
	extern int	bsd_sendto_short();
	extern int	bsd_sendto_long();

	return (((count <= SMALL_ARRAY_LIMIT) ? bsd_sendto_short
					      : bsd_sendto_long
		)(serv_port,
		  interrupt,
		  fileno,
		  flags,
		  to,
		  tolen,
		  data,
		  count,
		  &rval[0]));
}

#ifdef	COMPAT_43
int
e_recvfrom(serv_port, interrupt,
	   fileno, data, len, flags, from, fromlenaddr, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		fileno;
	char		*data;
	int		len;
	int		flags;
	char		*from;
	int		*fromlenaddr;
	int		*rval;
{
	/*
	 * We receive the address into a temporary buffer,
	 * since the MiG interface always returns the full
	 * amount.
	 */
	register int	error;
	unsigned int	data_count;
	sockarg_t	from_buf;
	int		from_count;

	data_count = len;
	from_count = *fromlenaddr;

	if (len <= SMALL_ARRAY_LIMIT) {
	    error = bsd_recvfrom_short(serv_port,
			interrupt,
			fileno,
			flags,
			len,
			from_buf,
			&from_count,
			data,
			&data_count);
	}
	else {
	    char *		data_addr;

	    error = bsd_recvfrom_long(serv_port,
			interrupt,
			fileno,
			flags,
			len,
			from_buf,
			&from_count,
			&data_addr,
			&data_count);
	    if (error == 0) {
		bcopy(data_addr, data, data_count);
		(void) vm_deallocate(mach_task_self(),
				(vm_offset_t)data_addr,
				data_count);
	    }
	}

	if (error == 0) {
	    rval[0] = data_count;
	    if (from) {
		bcopy(from_buf, from, from_count);
		*fromlenaddr = from_count;
	    }
	}
	return (error);
}
#endif	/* COMPAT_43 */

int
e_stat(serv_port, interrupt, fname, ub, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	char		*fname;
	struct stat	*ub;
	int		*rval;
{
#ifdef	COMPAT_43
	return (e_stat_call(serv_port, interrupt, fname, (caddr_t)ub, TRUE, FALSE));
#else
	return (e_stat_call(serv_port, interrupt, fname, (caddr_t)ub, TRUE));
#endif				
}

int
e_lstat(serv_port, interrupt, fname, ub, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	char		*fname;
	struct stat	*ub;
	int		*rval;
{
#ifdef	COMPAT_43
	return (e_stat_call(serv_port, interrupt, fname, (caddr_t)ub, FALSE, FALSE));
#else
	return (e_stat_call(serv_port, interrupt, fname, (caddr_t)ub, FALSE));
#endif				

}

#ifdef	COMPAT_43
struct  ostat
{
	short   st_dev;                /* ID of device containing a directory*/
					/*   entry for this file.  File serial*/
					/*   no + device ID uniquely identify */
					/*   the file within the system */
	ino_t   st_ino;                /* File serial number */
	u_short st_mode;               /* File mode; see #define's below */
	short   st_nlink;              /* Number of links */
	u_short st_uid;                /* User ID of the file's owner */
	u_short st_gid;                /* Group ID of the file's group */
	short   st_rdev;               /* ID of device */
					/*   This entry is defined only for */
					/*   character or block special files */
	off_t   st_size;               /* File size in bytes */
	time_t  st_atime;              /* Time of last access */
	int     st_spare1;
	time_t  st_mtime;              /* Time of last data modification */
	int     st_spare2;
	time_t  st_ctime;              /* Time of last file status change */
	int     st_spare3;
					/* Time measured in seconds since */
					/*   00:00:00 GMT, Jan. 1, 1970 */
	long    st_blksize;            /* Size of block in file */
	long    st_blocks;             /* blocks allocated for file */
	u_long  st_flags;              /* user defined flags for file */
	u_long  st_gen;                /* file generation number */

};

int
e_ostat(serv_port, interrupt, fname, oub, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	char		*fname;
	struct ostat	*oub;
	int		*rval;
{
	return (e_stat_call(serv_port, interrupt, fname, (caddr_t)oub, TRUE, TRUE));
}

int
e_olstat(serv_port, interrupt, fname, oub, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	char		*fname;
	struct ostat	*oub;
	int		*rval;
{
	return (e_stat_call(serv_port, interrupt, fname, (caddr_t)oub, FALSE, TRUE));
}
#endif	/* COMPAT_43 */

#ifdef	COMPAT_43
int
e_stat_call(serv_port, interrupt, fname, on_ub, follow, compat)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	char		*fname;
	caddr_t		*on_ub;		/* for lint */
	boolean_t	follow;
	boolean_t	compat;
#else
e_stat_call(serv_port, interrupt, fname, on_ub, follow)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	char		*fname;
	caddr_t		*on_ub;		/* for lint */
	boolean_t	follow;
#endif	/* COMPAT_43 */
{
	int		error;
	statb_t		statbuf;

	error = bsd_stat(serv_port,
			interrupt,
			follow,
			fname, strlen(fname) + 1,
			&statbuf);
	if (error)
	    return (error);

	/*
	 * Copy out stat fields
	 */
#ifdef	COMPAT_43
	if (compat) {
		struct ostat	*ob = (struct ostat *) on_ub;

		ob->st_dev	= statbuf.s_dev;
		ob->st_ino	= statbuf.s_ino;
		ob->st_mode	= statbuf.s_mode;
		ob->st_nlink	= statbuf.s_nlink;
		ob->st_uid	= statbuf.s_uid;
		ob->st_gid	= statbuf.s_gid;
		ob->st_rdev	= statbuf.s_rdev;
		ob->st_size	= statbuf.s_size;
		ob->st_atime	= statbuf.s_atime;
		ob->st_mtime	= statbuf.s_mtime;
		ob->st_ctime	= statbuf.s_ctime;
		ob->st_blksize	= statbuf.s_blksize;
		ob->st_blocks	= statbuf.s_blocks;
	} else
#endif	/* COMPAT_43 */
	{
		struct stat	*nb = (struct stat *) on_ub;

		nb->st_dev	= statbuf.s_dev;
		nb->st_ino	= statbuf.s_ino;
		nb->st_mode	= statbuf.s_mode;
		nb->st_nlink	= statbuf.s_nlink;
		nb->st_uid	= statbuf.s_uid;
		nb->st_gid	= statbuf.s_gid;
		nb->st_rdev	= statbuf.s_rdev;
		nb->st_size	= statbuf.s_size;
		nb->st_atime	= statbuf.s_atime;
		nb->st_mtime	= statbuf.s_mtime;
		nb->st_ctime	= statbuf.s_ctime;
		nb->st_blksize	= statbuf.s_blksize;
		nb->st_blocks	= statbuf.s_blocks;
		nb->st_flags	= statbuf.s_flags;
		nb->st_gen	= statbuf.s_gen;
	}
	return (0);
}

int
e_readlink(serv_port, interrupt, name, buf, count, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	char		*name;
	char		*buf;
	int		count;
	int		*rval;
{
	int		buflen;
	register int	error;

	buflen = count;
	error = bsd_readlink(serv_port,
			interrupt,
			name, strlen(name) + 1,
			count,		/* max length */
			buf,
			&buflen);
	if (error == 0)
	    rval[0] = buflen;
	return (error);
}

int
e_sysacct(serv_port, interrupt, fname, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	char		*fname;
	int		*rval;
{
	if (fname) {
	    return (bsd_acct(serv_port,
			interrupt,
			TRUE,
			fname, strlen(fname) + 1));
	}
	else {
	    return (bsd_acct(serv_port,
			interrupt,
			FALSE,
			"", 1));
	}
}

struct timeval	zero_time = { 0, 0 };

int
e_select(serv_port, interrupt, nd, in, ou, ex, tv, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		nd;
	fd_set		*in;
	fd_set		*ou;
	fd_set		*ex;
	timeval_t	*tv;
	int		*rval;
{
	register int	ni_size;
	register int	error;
	fd_set		zeros;
	fd_set		in_set, ou_set, ex_set;

	FD_ZERO(&zeros);

	if (nd > NOFILE)
	    nd = NOFILE;	/* 'forgiving, if slightly wrong' */
	ni_size = howmany(nd, NFDBITS) * sizeof(fd_mask);

	if (in)
	    bcopy((char *)in, (char *)&in_set, (unsigned)ni_size);
	if (ou)
	    bcopy((char *)ou, (char *)&ou_set, (unsigned)ni_size);
	if (ex)
	    bcopy((char *)ex, (char *)&ex_set, (unsigned)ni_size);

	error = bsd_select(serv_port,
			interrupt,
			nd,
			(in) ? &in_set : &zeros,
			(ou) ? &ou_set : &zeros,
			(ex) ? &ex_set : &zeros,
			(in != 0),
			(ou != 0),
			(ex != 0),
			(tv != 0),
			(tv) ? *tv : zero_time,
			&rval[0]);

	if (error)
	    return (error);

	if (in)
	    bcopy((char *)&in_set, (char *)in, (unsigned)ni_size);
	if (ou)
	    bcopy((char *)&ou_set, (char *)ou, (unsigned)ni_size);
	if (ex)
	    bcopy((char *)&ex_set, (char *)ex, (unsigned)ni_size);

	return (0);
}

int
e_htg_syscall(argp, rvalp)
{
}

#ifdef	COMPAT_43
struct sigvec	zero_sv = { 0, 0, 0 };

int
e_sigvec(serv_port, interrupt, sig, nsv, osv, tramp, rval)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register int		sig;
	register struct sigvec	*nsv;
	register struct sigvec	*osv;
	unsigned		tramp;
	int			*rval;
{
	register int	error;
	struct sigvec	old_sig_vec;

#ifdef	MAP_UAREA 
    if (shared_enabled) {
	int bit;

	if (sig <= 0 || sig > NSIG || sig == SIGKILL || sig == SIGSTOP)
		return EINVAL;
	bit = sigmask(sig);
	if (osv) {
#ifndef	OSF1_SERVER
		osv->sv_handler = shared_base_rw->us_signal[sig];
#else	OSF1_SERVER
		osv->sv_handler = (void(*))shared_base_rw->us_signal[sig];
#endif	OSF1_SERVER
		osv->sv_mask = shared_base_rw->us_usigmask[sig];
		bit = sigmask(sig);
		osv->sv_flags = 0;
		if ((shared_base_rw->us_sigonstack & bit) != 0)
			osv->sv_flags |= SV_ONSTACK;
		if ((shared_base_rw->us_sigintr & bit) != 0)
			osv->sv_flags |= SV_INTERRUPT;
	}
	if (nsv) {
		if (sig == SIGCONT && nsv->sv_handler == SIG_IGN)
			return EINVAL;
#ifdef	mips
		/*
		 * check for unaligned pc on sighandler
		 */
		if (nsv->sv_handler != SIG_IGN
		    && ((int)nsv->sv_handler & (sizeof(int)-1)))
			return EINVAL;
#endif	mips
		/*
		 * Set the signal trampoline code address - the
		 * server does not always know where it is.
		 */
#if	defined(balance) || defined(mips)
		shared_base_rw->us_sigtramp = (int (*)())tramp;
#endif	/* defined(balance) || defined(mips) */

		share_lock(&shared_base_rw->us_siglock);
#ifndef	OSF1_SERVER
		shared_base_rw->us_signal[sig] = nsv->sv_handler;
#else	OSF1_SERVER
		shared_base_rw->us_signal[sig] = (sig_t) nsv->sv_handler;
#endif	OSF1_SERVER
		shared_base_rw->us_usigmask[sig] = nsv->sv_mask &~ cantmask;

		if (nsv->sv_flags & SV_INTERRUPT)
			shared_base_rw->us_sigintr |= bit;
		else
			shared_base_rw->us_sigintr &= ~bit;
		if (nsv->sv_flags & SV_ONSTACK)
			shared_base_rw->us_sigonstack |= bit;
		else
			shared_base_rw->us_sigonstack &= ~bit;
		if (nsv->sv_handler == SIG_IGN) {
			shared_base_rw->us_sig &= ~bit;
			shared_base_rw->us_sigignore |= bit;
			shared_base_rw->us_sigcatch &= ~bit;
		} else {
			shared_base_rw->us_sigignore &= ~bit;
			if (nsv->sv_handler == SIG_DFL)
				shared_base_rw->us_sigcatch &= ~bit;
			else
				shared_base_rw->us_sigcatch |= bit;
		}
		share_unlock(&shared_base_rw->us_siglock);
	}
#ifdef	mips
	{
		extern unsigned sigtramp;
		sigtramp = tramp;
	}
#endif	mips
	e_checksignals(interrupt);
	return ESUCCESS;
    } else {
#endif	MAP_UAREA
	error = bsd_osigvec(serv_port,
			interrupt,
			sig,
			nsv != 0,
			(nsv) ? *nsv : zero_sv,
			&old_sig_vec,
			tramp);
	if (error == 0 && osv)
	    *osv = old_sig_vec;
#ifdef	mips
	if (error == 0) {
		extern unsigned sigtramp;
		sigtramp = tramp;
	}
#endif	mips
	return (error);
#ifdef	MAP_UAREA
    }
#endif	MAP_UAREA
}
#endif	/* COMPAT_43 */

#if 0
int
e_signal(serv_port, interrupt, sig, hdlr, tramp, rval)
	mach_port_t		serv_port;
	boolean_t		*interrupt;
	register int		sig;
	register int		(*hdlr)();
	unsigned		tramp;
	int 			*rval;
{
	register int	error;
	int		(*ohdlr)();	/* old handler */

#ifdef	MAP_UAREA 	
	if (shared_enabled && 0) {
		/* no optimization implemented: signal() is obsolete... */
	} else {
#endif	/* MAP_UAREA */
	error = bsd_signal(serv_port,
			interrupt,
			sig,
			hdlr,
			rval,
			tramp);
#ifdef	mips
	if (error == 0) {
		extern unsigned sigtramp;
		sigtramp = tramp;
	}
#endif	mips
	return (error);
#ifdef	MAP_UAREA
    }
#endif	MAP_UAREA
}

struct sigaction	zero_sa = { 0, 0, 0 };
/* defaultignmask copied from server/bsd/kern_sig.c */
#define defaultignmask	(sigmask(SIGCONT)|sigmask(SIGIO)|sigmask(SIGURG)| \
			sigmask(SIGCHLD)|sigmask(SIGWINCH)|sigmask(SIGINFO))

int
e_sigaction(serv_port, interrupt, sig, nsa, osa, tramp, rval)
	mach_port_t			serv_port;
	boolean_t			*interrupt;
	register int			sig;
	register struct sigaction	*nsa;
	register struct sigaction	*osa;
	unsigned			tramp;
	int				*rval;
{
	register int		error;
	struct sigaction	old_sig_act;

#ifdef	MAP_UAREA 
	/* setting SIGCHLD might require changing p->p_flag in RO area */
    if (shared_enabled &&
	!(sig == SIGCHLD && nsa &&
	  (nsa->sa_flags&SA_NOCLDSTOP == 0) != (shared_base_ro->us_flag&SNOCLDSTOP == 0))) {
	int bit;

#if	defined(i386)
	shared_base_rw->us_sigreturn = (int (*)())tramp;
#endif	/* defined(i386) */

	if (sig <= 0 || sig > NSIG)
		return EINVAL;
	bit = sigmask(sig);
	if (osa) {
		osa->sa_handler = (void(*))shared_base_rw->us_signal[sig];
		osa->sa_mask = shared_base_rw->us_usigmask[sig];
		osa->sa_flags = 0;
		if ((shared_base_rw->us_sigonstack & bit) != 0)
			osa->sa_flags |= SA_ONSTACK;
		if ((shared_base_rw->us_sigintr & bit) == 0)
			osa->sa_flags |= SA_RESTART;
		if (shared_base_ro->us_flag & SNOCLDSTOP)
		        osa->sa_flags |= SA_NOCLDSTOP;
	}
	if (nsa) {
		if (nsa->sa_handler != SIG_DFL && (sig == SIGKILL || sig == SIGSTOP))
			return EINVAL;
#ifdef	mips
		/*
		 * check for unaligned pc on sighandler
		 */
		if (nsa->sa_handler != SIG_IGN
		    && ((int)nsa->sa_handler & (sizeof(int)-1)))
			return EINVAL;
#endif	mips
		/*
		 * Set the signal trampoline code address - the
		 * server does not always know where it is.
		 */
#if	defined(balance) || defined(mips)
		shared_base_rw->us_sigtramp = (int (*)())tramp;
#endif	/* defined(balance) || defined(mips) */
		share_lock(&shared_base_rw->us_siglock);
		shared_base_rw->us_signal[sig] = (sig_t) nsa->sa_handler;
		shared_base_rw->us_usigmask[sig] = nsa->sa_mask &~ sigcantmask;

		if ((nsa->sa_flags & SA_RESTART) == 0)
			shared_base_rw->us_sigintr |= bit;
		else
			shared_base_rw->us_sigintr &= ~bit;
		if (nsa->sa_flags & SA_ONSTACK)
			shared_base_rw->us_sigonstack |= bit;
		else
			shared_base_rw->us_sigonstack &= ~bit;
#if	0	/* XXX should let server do it */
		if (sig == SIGCHLD) {
			if (nsa->sa_flags & SA_NOCLDSTOP)
			  shared_base_ro->us_flag |= SNOCLDSTOP;
			else
			  shared_base_ro->us_flag &= ~SNOCLDSTOP;
		}
#endif	/* 0 */
		if (nsa->sa_handler == SIG_IGN ||
		    (bit & defaultignmask && nsa->sa_handler == SIG_DFL)) {
			shared_base_rw->us_sig &= ~bit;
			if (sig != SIGCONT)
			  shared_base_rw->us_sigignore |= bit;
			shared_base_rw->us_sigcatch &= ~bit;
		} else {
			shared_base_rw->us_sigignore &= ~bit;
			if (nsa->sa_handler == SIG_DFL)
				shared_base_rw->us_sigcatch &= ~bit;
			else
				shared_base_rw->us_sigcatch |= bit;
		}
		share_unlock(&shared_base_rw->us_siglock);
	}
#ifdef	mips
	{
		extern unsigned sigtramp;
		sigtramp = tramp;
	}
#endif	mips
	e_checksignals(interrupt);
	return ESUCCESS;
    } else {
#endif	MAP_UAREA
	error = osf1_sigaction(serv_port,
			interrupt,
			sig,
			nsa != 0,
			(nsa) ? *nsa : zero_sa,
			&old_sig_act,
			tramp);
	if (error == 0 && osa)
	    *osa = old_sig_act;
#ifdef	mips
	if (error == 0) {
		extern unsigned sigtramp;
		sigtramp = tramp;
	}
#endif	mips
	return (error);
#ifdef	MAP_UAREA
    }
#endif	MAP_UAREA
}
#endif 0

struct sigstack	zero_ss = { 0, 0 };

int
e_sigstack(serv_port, interrupt, nss, oss)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	struct sigstack	*nss;
	struct sigstack	*oss;
{
	register int	error;
	struct sigstack	old_sig_stack;

#ifdef	MAP_UAREA 
    if (shared_enabled) {
	share_lock(&shared_base_rw->us_lock);
	if (oss)
	    *oss = shared_base_rw->us_sigstack;
	if (nss)
	    shared_base_rw->us_sigstack = *nss;
	share_unlock(&shared_base_rw->us_lock);
	e_checksignals(interrupt);
	return ESUCCESS;
    } else {
#endif	MAP_UAREA
	error = bsd_sigstack(serv_port,
			interrupt,
			(nss != 0),
			(nss) ? *nss : zero_ss,
			&old_sig_stack);
	if (error == 0 && oss)
	    *oss = old_sig_stack;
	return (error);
#ifdef	MAP_UAREA
    }
#endif	MAP_UAREA
}

struct timeval	zero_tv = { 0, 0 };
struct timezone	zero_tz = { 0, 0 };

int
e_settimeofday(serv_port, interrupt, tv, tz)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	struct timeval	*tv;
	struct timezone	*tz;
{
	return (bsd_settimeofday(serv_port,
			interrupt,
			(tv != 0),
			(tv) ? *tv : zero_tv,
			(tz != 0),
			(tz) ? *tz : zero_tz));
}

struct itimerval zero_itv = { 0, 0, 0, 0 };

int e_setitimer(serv_port, interrupt, which, itv, oitv)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		which;
	struct itimerval *itv;
	struct itimerval *oitv;
{
	register int	error;
	struct itimerval old_itimer_val;

	error = bsd_setitimer(serv_port,
			interrupt,
			which,
			(itv != 0),
			(itv) ? *itv : zero_itv,
			&old_itimer_val);
	if (error == 0 && oitv)
	    *oitv = old_itimer_val;
	return (error);
}

#ifdef	COMPAT_43
int
e_accept(serv_port, interrupt, s, name, anamelen, rvalp)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		s;
	char *		name;
	int		*anamelen;
	int		*rvalp;
{
	register int	error;
	int		namelen = sizeof(sockarg_t);
	int		new_s;

	sockarg_t	out_name;

	error = bsd_oaccept(serv_port,
			interrupt,
			s,
			out_name,
			&namelen,
			&new_s);
	if (error)
	    return (error);

	if (name) {
	    if (namelen > *anamelen)
		namelen = *anamelen;
	    bcopy(out_name, name, namelen);
	    *anamelen = namelen;
	}
	*rvalp = new_s;
	return (error);
}
#endif	/* COMPAT_43 */

int
e_setsockopt(serv_port, interrupt, s, level, name, val, valsize)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		s;
	int		level;
	int		name;
	char *		val;
	int		valsize;
{
	return (bsd_setsockopt(serv_port,
			interrupt,
			s,
			level,
			name,
			val,
			(val) ? valsize : 0));
}

int
e_getsockopt(serv_port, interrupt, s, level, name, val, avalsize)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		s;
	int		level;
	int		name;
	char *		val;
	int		*avalsize;
{
	register int	error;
	int		valsize = sizeof(sockarg_t);
	sockarg_t	val_buf;

	error = bsd_getsockopt(serv_port,
			interrupt,
			s,
			level,
			name,
			val_buf,
			&valsize);
	if (error)
	    return (error);

	if (val) {
	    if (valsize > *avalsize)
		valsize = *avalsize;
	    bcopy(val_buf, val, valsize);
	    *avalsize = valsize;
	}
	return (error);
}

#ifdef	COMPAT_43
int
e_getsockname(serv_port, interrupt, fdes, asa, alen)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		fdes;
	char *		asa;
	int		*alen;
{
	register int	error;
	int		buflen = sizeof(sockarg_t);
	sockarg_t	asa_buf;

	error = bsd_ogetsockname(serv_port,
			interrupt,
			fdes,
			asa_buf,
			&buflen);
	if (error)
	    return (error);

	if (buflen > *alen)
	    buflen = *alen;
	bcopy(asa_buf, asa, buflen);
	*alen = buflen;

	return (error);
}

int
e_getpeername(serv_port, interrupt, fdes, asa, alen)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		fdes;
	char *		asa;
	int		*alen;
{
	register int	error;
	int		buflen = sizeof(sockarg_t);
	sockarg_t	asa_buf;

	error = bsd_ogetpeername(serv_port,
			interrupt,
			fdes,
			asa_buf,
			&buflen);
	if (error)
	    return (error);

	if (buflen > *alen)
	    buflen = *alen;
	bcopy(asa_buf, asa, buflen);
	*alen = buflen;

	return (error);
}
#endif	COMPAT_43

#if CMUCS
int
e_table(serv_port, interrupt, id, index, addr, nel, lel, rval)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		id;
	int		index;
	char *		addr;
	int		nel;	/* >0 ==> get, <0 ==> set */
	unsigned int	lel;
	int		*rval;
{
	register int	error;
	int		nel_done;

	if (nel < 0) {
	    /*
	     * Set.
	     */
	    error = bsd_table_set(serv_port, interrupt,
			id, index, lel, nel,
			addr, -nel*lel,
			&nel_done);
	}
	else {
	    char *		out_addr;
	    unsigned int	out_count;

	    error = bsd_table_get(serv_port, interrupt,
			id, index, lel, nel,
			&out_addr, &out_count,
			&nel_done);

	    if (error == 0) {
		/*
		 * Copy table to addr
		 */
		bcopy(out_addr, addr, lel * nel_done);
		(void) vm_deallocate(mach_task_self(),
				     (vm_offset_t)out_addr,
				     (vm_size_t) out_count);
	    }
	}
	if (error == 0)
	    *rval = nel_done;
	return (error);
}
#endif

int
e_writev(proc_port, interrupt, fdes, iov, iovcnt, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* out */
	int		fdes;
	struct iovec	*iov;
	unsigned	iovcnt;
	int		*rval;		/* out */
{
	register int	i;
	register int	len;
	register char	*cp;
	register struct iovec *iovp;
	unsigned int	count;
	int		result;
	char *		bufptr;
	struct iovec	aiov[16];
	char		buf[SMALL_ARRAY_LIMIT];
#ifdef	MAP_UAREA
	int		shared = 0;
#ifdef	MAP_FILE
	int		nocopy = 0;
	register struct file_info *fd = &shared_base_rw->us_file_info[fdes];

	if (fdes < 0 ||
#ifndef	OSF1_SERVER
	   (shared_enabled && fdes > shared_base_ro->us_lastfile)) {
#else	OSF1_SERVER
	   (shared_enabled && fdes > shared_base_ro->us_file_state.uf_lastfile)) {
#endif	OSF1_SERVER
	    EPRINT(("e_writev badfile"));
	    rval[0] = -1;
	    return EBADF;
	}
#endif	MAP_FILE
#endif	MAP_UAREA

	rval[0] = -1;
	if (iovcnt > sizeof(aiov)/sizeof(aiov[0])) {
	    EPRINT(("e_writev too many vectors"));
	    return (EINVAL);
	}

	bcopy((char *)iov, (char *)aiov, iovcnt * sizeof(struct iovec));

	count = 0;
	for (i = 0, iovp = aiov; i < iovcnt; i++, iovp++) {
	    len = iovp->iov_len;
	    if (len < 0) {
		EPRINT(("e_writev invalid vector length %x", len));
		return (EINVAL);
	    }
	    count += len;
	    if (count < 0) {
		EPRINT(("e_writev count < 0"));
		return (EINVAL);
	    }
	}
#ifdef	MAP_UAREA
#ifdef	MAP_FILE
	share_lock(&fd->lock);
	if (fd->mapped && fd->open) {
	    share_unlock(&fd->lock);
	    nocopy = 1;
	} else {
	    share_unlock(&fd->lock);
#endif	MAP_FILE
	    spin_lock(&readwrite_lock);
	    if (count <= vm_page_size && readwrite_inuse == 0) {
		bufptr = shared_readwrite;
		readwrite_inuse = shared = 1;
		spin_unlock(&readwrite_lock);
	    } else {
		spin_unlock(&readwrite_lock);
#endif	MAP_UAREA
		if (count <= SMALL_ARRAY_LIMIT) {
		    /*
		     * Short write.  Copy into buffer.
		     */
		    bufptr = buf;
		}
		else {
		    /*
		     * Long write.  Allocate memory to fill.
		     * (Hope that no one uses this to write large
		     *  amounts of data; we`ll lose on the copying.)
		     */
		    (void) vm_allocate(mach_task_self(),
				(vm_offset_t *)&bufptr,
				count,
				TRUE);
		}
#ifdef	MAP_UAREA
	    }
#ifdef	MAP_FILE
	}
#endif	MAP_FILE
#endif	MAP_UAREA
	cp = bufptr;
	for (i = 0, iovp = aiov; i < iovcnt; i++, iovp++) {
	    len = iovp->iov_len;
#if	defined(MAP_UAREA) && defined(MAP_FILE)
	    if (nocopy) {
		if (!e_maprw(proc_port, interrupt, fdes, iovp->iov_base, len,
		             rval, 1, &result) || result != ESUCCESS) {
		    EPRINT(("e_writev e_maprw %d",result));
		    return (result);
		}
	    } else
#endif	defined(MAP_UAREA) && defined(MAP_FILE)
	    bcopy(iovp->iov_base, cp, len);
	    cp += len;
	}

#ifdef	MAP_UAREA
#ifdef	MAP_FILE
	if (nocopy) {
	    rval[0] = count;
	    return ESUCCESS;
	}
#endif	MAP_FILE
	if (shared) {
	    result = e_readwrite(proc_port, interrupt, fdes, 0, count, 
				 rval, 1, 0);
	    spin_lock(&readwrite_lock);
	    readwrite_inuse = 0;
	    spin_unlock(&readwrite_lock);
	} else {
#endif	MAP_UAREA
	    result = ((count <= SMALL_ARRAY_LIMIT) ? bsd_write_short
						   : bsd_write_long)
			(proc_port,
			interrupt,
			fdes,
			bufptr,
			count,
			rval);

	    if (result != 0) {
		EPRINT(("e_writev bsd_write_* %d",result));
	    }
	    if (count > SMALL_ARRAY_LIMIT)
		(void) vm_deallocate(mach_task_self(), (vm_offset_t)bufptr, count);

#ifdef	MAP_UAREA
	}
#endif	MAP_UAREA
	return (result);
}

int
e_readv(proc_port, interrupt, fdes, iov, iovcnt, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;	/* out */
	int		fdes;
	struct iovec	*iov;
	unsigned	iovcnt;
	int		*rval;		/* out */
{
	register int	i;
	register int	len;
	register struct iovec *iovp;
	unsigned int	count;
	int		result;
	int		arg[3];
	struct iovec	aiov[16];
	char		buf[SMALL_ARRAY_LIMIT];
#ifdef	MAP_UAREA
	int		shared = 0;
#ifdef	MAP_FILE
	int		nocopy = 0;
	register struct file_info *fd = &shared_base_rw->us_file_info[fdes];

	if (fdes < 0 ||
#ifndef	OSF1_SERVER
	   (shared_enabled && fdes > shared_base_ro->us_lastfile)) {
#else	OSF1_SERVER
	   (shared_enabled && fdes > shared_base_ro->us_file_state.uf_lastfile)) {
#endif	OSF1_SERVER
	    EPRINT(("e_writev badfile"));
	    rval[0] = -1;
	    return EBADF;
	}
#endif	MAP_FILE
#endif	MAP_UAREA

	rval[0] = -1;
	if (iovcnt > sizeof(aiov)/sizeof(aiov[0])) {
	    EPRINT(("e_readv too many vectors"));
	    return (EINVAL);
	}

	bcopy((char *)iov, (char *)aiov, iovcnt * sizeof(struct iovec));

	count = 0;
	for (i = 0, iovp = aiov; i < iovcnt; i++, iovp++) {
	    len = iovp->iov_len;
	    if (len < 0) {
		EPRINT(("e_readv invalid vector length %x", len));
		EPRINT(("        count = %d  vector =%d", count, i));
		EPRINT(("        iovcnt = %d", iovcnt));
		return (EINVAL);
	    }
	    count += len;
	    if (count < 0) {
		EPRINT(("e_readv count < 0"));
		return (EINVAL);
	    }
	}

	arg[0] = fdes;
	arg[2] = (int)count;

#ifdef	MAP_UAREA
#ifdef	MAP_FILE
	share_lock(&fd->lock);
	if (fd->mapped && fd->open) {
	    share_unlock(&fd->lock);
	    nocopy = 1;
	} else {
	    share_unlock(&fd->lock);
#endif	MAP_FILE
	    spin_lock(&readwrite_lock);
	    if (count <= vm_page_size && readwrite_inuse == 0) {
		arg[1] = (int) shared_readwrite;
		readwrite_inuse = shared = 1;
		spin_unlock(&readwrite_lock);
	    } else {
		spin_unlock(&readwrite_lock);
#endif	MAP_UAREA
		if (count <= SMALL_ARRAY_LIMIT) {
		    /*
		     * Short read.  Copy into buffer.
		     */
		    arg[1] = (int)buf;
		}
		else {
		    /*
		     * Long read.  Allocate memory to fill.
		     * (Hope that no one uses this to read large
		     *  amounts of data; we`ll lose on the copying.)
		     */
		    (void) vm_allocate(mach_task_self(),
				(vm_offset_t *)&arg[1],
				count,
				TRUE);
		}
#ifdef	MAP_UAREA
	    }
#ifdef	MAP_FILE
	}

	if (nocopy) {
	    result = 0;
	} else
#endif	MAP_FILE
	    if (shared) {
		result = e_readwrite(proc_port, interrupt, fdes, 0,
				     count, rval, 0, 0);
		if (rval[0]>count) {
		   EPRINT(("e_readv e_readwrite ask %d got %d",count,rval[0]));
		}
		count = rval[0];
	    } else
#endif	MAP_UAREA
	    {
		result = emul_generic(proc_port, interrupt,
				      SYS_read, arg, rval);
		if (rval[0]>count) {
		    EPRINT(("e_readv SYS_read ask %d got %d",count,rval[0]));
		}
		count = rval[0];
		if (result != 0) {
			EPRINT(("e_readv SYS_read %d",result));
		}
	    }

	if (result == 0) {
	    register char	*cp;

	    cp = (char *)arg[1];

	    for (i = 0, iovp = aiov; i < iovcnt; i++, iovp++) {
		len = iovp->iov_len;
#if	defined(MAP_UAREA) && defined(MAP_FILE)
		if (nocopy) {
		    if (!e_maprw(proc_port, interrupt, fdes, iovp->iov_base,
				 len, rval, 0, &result) || result != ESUCCESS){
			EPRINT(("e_readv e_maprw %d",result));
			rval[0] = -1;
			return (result);
		    }
		    if (rval[0] == 0) {
			rval[0] = (int)(cp - arg[1]);
			if (rval[0] > arg[2]) {
			    EPRINT(("r_readv ask %d got %d",arg[2],rval[0]));
			}
			return (ESUCCESS);
		    }
		} else
#endif	defined(MAP_UAREA) && defined(MAP_FILE)
		    bcopy(cp, iovp->iov_base, len);
		cp += len;
	    }
	}

#ifdef	MAP_UAREA
	if (shared) {
	    spin_lock(&readwrite_lock);
	    readwrite_inuse = 0;
	    spin_unlock(&readwrite_lock);
	}
#endif	MAP_UAREA

	if (count > SMALL_ARRAY_LIMIT)
#ifdef	MAP_UAREA
	    if (!shared)
#ifdef	MAP_FILE
		if(!nocopy)
#endif	MAP_FILE
#endif	MAP_UAREA
		    (void) vm_deallocate(mach_task_self(), (vm_offset_t)arg[1],
					 count);

	rval[0] = count;
	if (rval[0] > arg[2]) {
	    EPRINT(("r_readv(2) ask %d got %d",arg[2],rval[0]));
	}
	return (result);

}

#if VICE
int
e_pioctl(proc_port, interrupt, path, com, data, follow)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*path;
	int		com;
	struct ViceIoctl *data;
	int		follow;
{
	register int	result;
	unsigned int	in_count;
	unsigned int	out_count;

	if (data->in == 0)
	    in_count = 0;
	else
	    in_count = data->in_size;
	if (in_count > SMALL_ARRAY_LIMIT)
	    in_count = SMALL_ARRAY_LIMIT;

	if (data->out == 0)
	    out_count = 0;
	else    
	    out_count = data->out_size;
	if (out_count > SMALL_ARRAY_LIMIT)
	    out_count = SMALL_ARRAY_LIMIT;

	result = bsd_pioctl(proc_port,
			interrupt,
			path, strlen(path) + 1,
			com,
			follow,
			data->in,
			in_count,
			out_count,	/* amount wanted */
			data->out,
			&out_count);	/* amount returned */
	if (result == MIG_ARRAY_TOO_LARGE)
	    result = EINVAL;
	if (result == MACH_RCV_TOO_LARGE)
	    result = EINVAL;
	return (result);
}
#else VICE
int
e_pioctl(proc_port, interrupt, path, com, data, follow)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*path;
	int		com;
	int		*data;
	int		follow;
{
	return EINVAL;
}
#endif VICE

#define MAXPRINT 128
#define putchar(x) if (pos<MAXPRINT) error[pos++] = (x); else goto done
int
e_emulator_error(fmt, va_alist)
	char *fmt;
	va_dcl
{
	va_list adx;
	register int b, c, i;
	char *s;
	u_long	value;
	char prbuf[11];
	register char *cp;
	char error[MAXPRINT];
	int pos = 0;

	va_start(adx);
loop:
	while ((c = *fmt++) != '%') {
	    if (c == '\0') {
		goto done;
	    }
	    putchar(c);
	}
	c = *fmt++;
	switch (c) {
	    case 'x': case 'X':
		b = 16;
		goto number;
	    case 'd': case 'D':
	    case 'u':
		b = 10;
		goto number;
	    case 'o': case 'O':
		b = 8;
number:
		value = va_arg(adx, u_long);
		if (b == 10 && (long)value < 0) {
		    putchar('-');
		    value = -value;
		}
		cp = prbuf;
		do {
		    *cp++ = "0123456789abcdef"[value%b];
		    value /= b;
		} while (value);
		do {
		    putchar(*--cp);
		} while (cp > prbuf);
		break;
	    case 'c':
		value = va_arg(adx, u_long);
		for (i = 24; i >= 0; i -= 8)
			if (c = (value >> i) & 0x7f)
				putchar(c);
		break;
	    case 's':
		s = va_arg(adx, char *);
		i = 0;
		while (c = *s++) {
		    putchar(c);
		}
		break;
	    case '%':
		putchar('%');
		break;
	}
	goto loop;
done:
	error[pos]='\0';
	va_end(adx);
	return bsd_emulator_error(our_bsd_server_port, error, pos+1);
}
int
e_chdir(proc_port, interrupt, fname)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
{
	return bsd_chdir(proc_port,
			interrupt,
			fname, strlen(fname) + 1);
}

int
e_chroot(proc_port, interrupt, fname)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
{
	return bsd_chroot(proc_port,
			interrupt,
			fname, strlen(fname) + 1);
}

int
e_open(proc_port, interrupt, fname, mode, crtmode, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	int		mode;
	int		crtmode;
	int		*rval;
{
	return bsd_open(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			mode,
			crtmode,
			rval);
}

#ifdef	COMPAT_43
int
e_creat(proc_port, interrupt, fname, fmode, rval)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	int		fmode;
	int		*rval;
{
	return bsd_ocreat(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			fmode,
			rval);
}
#endif	/* COMPAT_43 */

int
e_mknod(proc_port, interrupt, fname, fmode, dev)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	int		fmode;
	int		dev;
{
	return bsd_mknod(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			fmode,
			dev);
}

int
e_link(proc_port, interrupt, target, linkname)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*target;
	char		*linkname;
{
	return bsd_link(proc_port,
			interrupt,
			target, strlen(target) + 1,
			linkname, strlen(linkname) + 1);
}

int
e_symlink(proc_port, interrupt, target, linkname)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*target;
	char		*linkname;
{
	return bsd_symlink(proc_port,
			interrupt,
			target, strlen(target) + 1,
			linkname, strlen(linkname) + 1);
}

int
e_unlink(proc_port, interrupt, fname)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
{
	return bsd_unlink(proc_port,
			interrupt,
			fname, strlen(fname) + 1);
}

int
e_saccess(proc_port, interrupt, fname, fmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	int		fmode;
{
	return bsd_access(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			fmode);
}

int
e_chmod(proc_port, interrupt, fname, fmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	int		fmode;
{
	return bsd_chmod(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			fmode);
}

int
e_chown(proc_port, interrupt, fname, uid, gid)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	int		uid;
	int		gid;
{
	return bsd_chown(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			uid,
			gid);
}

int
e_utimes(proc_port, interrupt, fname, times)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	timeval_2_t	times;
{
	return bsd_utimes(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			times);
}

int
e_truncate(proc_port, interrupt, fname, length)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	int		length;
{
	return bsd_truncate(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			length);
}

int
e_rename(proc_port, interrupt, from_name, to_name)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*from_name;
	char		*to_name;
{
	return bsd_rename(proc_port,
			interrupt,
			from_name, strlen(from_name) + 1,
			to_name, strlen(to_name) + 1);
}

int
e_mkdir(proc_port, interrupt, fname, dmode)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	int		dmode;
{
	return bsd_mkdir(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			dmode);
}

int
e_rmdir(proc_port, interrupt, fname)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
{
	return bsd_rmdir(proc_port,
			interrupt,
			fname, strlen(fname) + 1);
}

#if 0/*CMUCS*/
int
e_xutimes(proc_port, interrupt, fname, times)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fname;
	timeval_3_t	times;
{
	return bsd_xutimes(proc_port,
			interrupt,
			fname, strlen(fname) + 1,
			times);
}
#endif CMUCS

#if 0
int
e_mount(proc_port, interrupt, fspec, freg, ronly)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fspec;
	char		*freg;
	int		ronly;
{
	return bsd_mount(proc_port,
			interrupt,
			fspec, strlen(fspec) + 1,
			freg, strlen(freg) + 1,
			ronly);
}

int
e_umount(proc_port, interrupt, fspec)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	char		*fspec;
{
	return bsd_umount(proc_port,
			interrupt,
			fspec, strlen(fspec) + 1);
}
#endif 0
