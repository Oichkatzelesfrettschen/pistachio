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
 * $Log:	emul_mapped.c,v $
 * Revision 2.1  92/04/21  17:10:40  rwd
 * BSDSS
 * 
 *
 */

/*
 * Routines which use mapped area of uarea instead of sending message
 * to server
 */

#ifdef MAP_UAREA
#include <uxkern/import_mach.h>
#include <uxkern/bsd_msg.h>
#include <mach/machine/vm_param.h>
#include <machine/machparam.h>
#include <sys/types.h>
#include <sys/ushared.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <machine/vmparam.h>

int enable_sharing = 1;
int shared_enabled = 0;
struct ushared_ro *shared_base_ro;
struct ushared_rw *shared_base_rw;
char *shared_readwrite;
int readwrite_inuse = 0;
spin_lock_t readwrite_lock = 0;
#ifdef	MAP_UAREA
spin_lock_t in_emulator_lock = 0;
#endif	/* MAP_UAREA */

#define	DEBUG 0

#if	DEBUG
#define	EPRINT(a) e_emulator_error/**/a/**/
#else	DEBUG
#define	EPRINT(a)
#endif	DEBUG

/*
 * Same as bsd/mach_signal.c
 */

extern  mach_port_t	our_bsd_server_port;

emul_mapped_init()
{
	kern_return_t	result;
	vm_address_t	address;
	vm_size_t	size;
	vm_prot_t	prot;
	vm_prot_t	max_prot;
	vm_inherit_t	inherit;
	boolean_t	shared;
	mach_port_t	object_name;
	vm_offset_t	offset;
	vm_statistics_data_t	vm_stat;

	if (!enable_sharing)
		return;

	shared_base_ro = (struct ushared_ro *)(EMULATOR_END - vm_page_size);
	shared_base_rw = (struct ushared_rw *)(EMULATOR_END - 2*vm_page_size);
	shared_readwrite = (char *) (EMULATOR_END - 3*vm_page_size);

	address = (vm_address_t) shared_base_ro;
	result = vm_region(mach_task_self(), &address, &size,
			   &prot, &max_prot, &inherit, &shared,
			   &object_name, &offset);
	if (result != KERN_SUCCESS) {
		e_emulator_error("vm_region ret = %x",result);
		return;
	}
	if (!(prot & VM_PROT_READ)) {
		e_emulator_error("shared region not readable");
		return;
	}
	if (shared_base_ro->us_version != USHARED_VERSION) {
		e_emulator_error("shared region mismatch %x/%x",
			shared_base_ro->us_version, USHARED_VERSION);
		return;
	}

	shared_enabled = 1;
	shared_base_rw->us_inuse = 1;
	shared_base_rw->us_debug = 0;
	readwrite_inuse = 0;
	spin_lock_init(&readwrite_lock);
	if (shared_base_ro->us_mach_nbc)
		shared_base_rw->us_map_files = 1;
	spin_lock_init(&in_emulator_lock);
}

#define E_DECLARE(routine) \
int routine(serv_port, interrupt, syscode, argp, rvalp) \
	mach_port_t	serv_port; \
	boolean_t	*interrupt; \
	int	syscode; \
	int	* argp; \
	int	* rvalp; \
{ \

#define E_IF \
    if (shared_enabled) {

#define E_END(status) \
end:\
	e_checksignals(interrupt); \
	return (status); \
    } else { \
server: \
	return emul_generic(serv_port, interrupt, syscode, argp, rvalp); \
    } \
} \


E_DECLARE(e_obreak)
	vm_offset_t	old, new;
	kern_return_t	ret;
E_IF
	new = round_page(argp[0]);
	if ((int)(new - (vm_offset_t)shared_base_ro->us_data_start) >
	    shared_base_ro->us_rlimit[RLIMIT_DATA].rlim_cur) {
		return ENOMEM;
	}
	old = round_page(shared_base_ro->us_data_start + 
			 ctob(shared_base_rw->us_dsize));
	if (new > old) {
		ret = vm_allocate(mach_task_self(), &old, new - old, FALSE);
		if (ret != KERN_SUCCESS) {
			e_emulator_error("obreak : vm_allocate %x",ret);
			goto server;
		} else
			shared_base_rw->us_dsize += btoc(new - old);
	}
E_END(ESUCCESS)


E_DECLARE(e_getdtablesize)
E_IF
#ifndef	OSF1_SERVER
	rvalp[0] = shared_base_ro->us_nofile;
#else	OSF1_SERVER
	rvalp[0] = shared_base_ro->us_nofile;
#endif	OSF1_SERVER
E_END(ESUCCESS)


E_DECLARE(e_getuid)
E_IF
#ifndef	OSF1_SERVER
	rvalp[0] = shared_base_ro->us_ruid;
	rvalp[1] = shared_base_ro->us_uid;
E_END(ESUCCESS)
#else	OSF1_SERVER
E_END(EINVAL)
#endif	OSF1_SERVER


E_DECLARE(e_getpid)
E_IF
	rvalp[0] = shared_base_ro->us_pid;
	rvalp[1] = shared_base_ro->us_ppid;
E_END(ESUCCESS)


E_DECLARE(e_getgid)
E_IF
#ifndef	OSF1_SERVER
	rvalp[0] = shared_base_ro->us_rgid;
	rvalp[1] = shared_base_ro->us_gid;
E_END(ESUCCESS)
#else	OSF1_SERVER
E_END(EINVAL)
#endif	OSF1_SERVER


E_DECLARE(e_getgroups)
	register gid_t *gp;
	int *groups = (int *)argp[1];
	int size,i;
E_IF
	for (gp = &shared_base_ro->us_groups[NGROUPS];
	     gp > shared_base_ro->us_groups; gp--)
		if (gp[-1] != NOGROUP)
			break;
	size = gp - shared_base_ro->us_groups;
	if (argp[0] < size) return EINVAL;
	for (i=0; i < size; i++)
		groups[i]=shared_base_ro->us_groups[i];
	rvalp[0] = size;
E_END(ESUCCESS)


E_DECLARE(e_getrlimit)
	struct rlimit *rlp = (struct rlimit *)argp[1];
E_IF
	if (argp[0] >= RLIM_NLIMITS) return EINVAL;
	*rlp = shared_base_ro->us_rlimit[argp[0]];
E_END(ESUCCESS)


E_DECLARE(e_umask)
E_IF
	rvalp[0] = shared_base_rw->us_cmask;
	shared_base_rw->us_cmask = argp[0];
E_END(ESUCCESS)


E_DECLARE(e_osigblock)
E_IF
	share_lock(&shared_base_rw->us_siglock);
	rvalp[0] = shared_base_rw->us_sigmask;
	shared_base_rw->us_sigmask |= argp[0] &~ cantmask;
	share_unlock(&shared_base_rw->us_siglock);
E_END(ESUCCESS)

E_DECLARE(e_sigpending)
E_IF
        sigset_t *set = (sigset_t *)argp[0];
        *set = shared_base_rw->us_sig;
E_END(ESUCCESS)

E_DECLARE(e_sigprocmask)
E_IF
        int how = (int) argp[0];
        sigset_t mask = (sigset_t) argp[1];
        share_lock(&shared_base_rw->us_siglock);
        rvalp[0] = shared_base_rw->us_sigmask;
        switch (how) {
	      case SIG_BLOCK:
		shared_base_rw->us_sigmask |= mask &~ sigcantmask;
		break;
	      case SIG_UNBLOCK:
		shared_base_rw->us_sigmask &= ~mask;
		break;
	      case SIG_SETMASK:
		shared_base_rw->us_sigmask = mask &~ sigcantmask;
		break;
	      default:
		share_unlock(&shared_base_rw->us_siglock);
		return EINVAL;
	}
        share_unlock(&shared_base_rw->us_siglock);
E_END(ESUCCESS)

#ifdef	MAP_FILE

E_DECLARE(e_lseek)
	register struct file_info *fd;
E_IF
	if (argp[0] < 0 || argp[0] > shared_base_ro->us_nofile)
	    return (EBADF);
	fd = &shared_base_rw->us_file_info[argp[0]];

	share_lock(&fd->lock);
	if (fd->mapped && fd->open) {
	    get_it(argp[0], interrupt);
	    switch (argp[2]) {

	    case L_INCR:
		fd->offset += argp[1];
		break;

	    case L_XTND:
		fd->offset = argp[1] + fd->map_info.inode_size;
		break;

	    case L_SET:
		fd->offset = argp[1];
		break;

	    default:
		rel_it(argp[0], interrupt);
		return (EINVAL);
	    }
	    rvalp[0] = fd->offset;
	    rel_it(argp[0], interrupt);
	} else {
	    share_unlock(&fd->lock);
	    goto server;
	}
E_END(ESUCCESS)

#endif	MAP_FILE

E_DECLARE(e_read)
	int result;
E_IF
#ifdef	MAP_FILE
	if (!e_maprw(serv_port, interrupt, argp[0], argp[1], argp[2],
			     rvalp, 0, &result)) {
#endif	MAP_FILE
	if (argp[2] > vm_page_size) goto server;
	spin_lock(&readwrite_lock);
	if (readwrite_inuse) {
		spin_unlock(&readwrite_lock);
		goto server;
	}
	readwrite_inuse = 1;
	spin_unlock(&readwrite_lock);
	result = e_readwrite(serv_port, interrupt, argp[0], argp[1], argp[2],
			     rvalp, 0, 1);
#ifdef	MAP_FILE
	}
#endif	MAP_FILE
E_END(result)

E_DECLARE(e_osigsetmask)

E_IF
	share_lock(&shared_base_rw->us_siglock);

	rvalp[0] = shared_base_rw->us_sigmask;
	shared_base_rw->us_sigmask = argp[0] &~ cantmask;

	share_unlock(&shared_base_rw->us_siglock);

E_END(ESUCCESS)

share_lock(x)
register struct shared_lock *x;
{
	int i=0;
	while(!spin_try_lock(&(x)->lock)) {
	    if (++i % 1024 == 0)
		e_emulator_error("share_lock failure %d", i);
	    swtch_pri(0);
	}
	(x)->who = shared_base_ro->us_proc_pointer & ~KERNEL_USER;
}

share_unlock(x)
register struct shared_lock *x;
{
    (x)->who = 0;
    spin_unlock(&(x)->lock);
    if ((x)->need_wakeup)
	bsd_share_wakeup(our_bsd_server_port, (int)x - (int)shared_base_rw);
}

e_shared_sigreturn(proc_port, interrupt, old_on_stack, old_sigmask)
	mach_port_t	proc_port;
	boolean_t	*interrupt;
	int		old_on_stack;
	int		old_sigmask;
{
	share_lock(&shared_base_rw->us_lock);
	shared_base_rw->us_sigstack.ss_onstack = old_on_stack & 01;
	share_unlock(&shared_base_rw->us_lock);

	share_lock(&shared_base_rw->us_siglock);
	shared_base_rw->us_sigmask = old_sigmask & ~cantmask;
	share_unlock(&shared_base_rw->us_siglock);

	e_checksignals(interrupt);
}

e_checksignals(interrupt)
	boolean_t	*interrupt;
{
	if (shared_enabled) {
		/*
		 *	This is really just a hint; so the lack
		 *	of locking isn't important.
		 */

		if (shared_base_ro->us_cursig ||
		    (shared_base_rw->us_sig &~
		     (shared_base_rw->us_sigmask |
		      ((shared_base_ro->us_flag&STRC) ?
		       0 : shared_base_rw->us_sigignore)
#if	VICE
		      | ((shared_base_ro->us_flag&SRMT) ?
		       stopsigmask : 0))))
#else	VICE
		       )))
#endif	VICE
			*interrupt = TRUE;
	}
}

E_DECLARE(e_close)
	struct file_info *fd;
E_IF
#ifndef	OSF1_SERVER
	if (argp[0] < 0 || argp[0] > shared_base_ro->us_lastfile)
#else	OSF1_SERVER

	if (argp[0] < 0 || argp[0] > shared_base_ro->us_file_state.uf_lastfile)
#endif	OSF1_SERVER
	    return (EBADF);
	fd = &shared_base_rw->us_file_info[argp[0]];

	if (shared_base_rw->us_closefile != -1)
	    goto server;
	share_lock(&fd->lock);
	if (fd->mapped && fd->open) {
	    shared_base_rw->us_closefile = argp[0];
	    fd->open = FALSE;
	    share_unlock(&fd->lock);
	    return (ESUCCESS);
	}
	share_unlock(&fd->lock);
	goto server;
E_END(ESUCCESS)

int
e_readwrite(serv_port, interrupt, fileno, data, count, rval, which, copy)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		fileno;
	char		*data;
	unsigned int	count;
	int		*rval;
	int		which;
	int		copy;
{
	extern int bsd_readwrite();
	int result;

	rval[0] = -1;
#ifndef	OSF1_SERVER
	if (fileno < 0 || fileno > shared_base_ro->us_lastfile)
#else	OSF1_SERVER
	if (fileno < 0 || fileno > shared_base_ro->us_file_state.uf_lastfile)
#endif	OSF1_SERVER
	    return (EBADF);
	if (which && copy)
	    bcopy(data, shared_readwrite, count);
	result = bsd_readwrite(serv_port, interrupt, which, fileno,
			       count, rval);
	if (!which && rval[0] > 0 && copy)
	    bcopy(shared_readwrite, data, rval[0]);
	if (copy) {
	    spin_lock(&readwrite_lock);
	    readwrite_inuse = 0;
	    spin_unlock(&readwrite_lock);
	}
	return (result);
}

#ifdef	MAP_FILE

int
e_maprw(serv_port, interrupt, fileno, data, count, rval, which, result)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int		fileno;
	char		*data;
	unsigned int	count;
	int		*rval;
	int		which;
	int		*result;
{
	register struct file_info *fd;
	register struct map_info *mi;
	char *from,*to;	
	char *wdata;
	int size,tsize;

#ifndef	OSF1_SERVER
	if (fileno < 0 || fileno > shared_base_ro->us_lastfile) {
#else	OSF1_SERVER
	if (fileno < 0 || fileno > shared_base_ro->us_file_state.uf_lastfile) {
#endif	OSF1_SERVER
	    EPRINT(("e_maprw:%d badfile",which));
	    *result = EBADF;
	    return TRUE;
	}
	fd = &shared_base_rw->us_file_info[fileno];
	mi = &fd->map_info;

	share_lock(&fd->lock);
	if (fd->mapped && fd->open) {
	  if(which?fd->write:fd->read) {
	    get_it(fileno, interrupt);
	    wdata = data;
	    tsize = size = count;
	    while (tsize > 0 && size > 0) {
		size = tsize;
		if (which & fd->append)
		    fd->offset = mi->inode_size;
		if (fd->offset < mi->offset ||
		    fd->offset >= mi->offset + mi->size)
		    bsd_maprw_remap(serv_port, interrupt, fileno, fd->offset, 
				    size);
		if (fd->offset + size > mi->offset + mi->size)
		    size = mi->offset + mi->size - fd->offset;
		from =(char * )( mi->address + fd->offset - mi->offset);
		if (which) {
		    if (fd->offset + size > mi->inode_size)
			mi->inode_size = fd->offset + size;
		    fd->dirty = TRUE;
		    to = from;
		    from = wdata;
		} else {
		    if (fd->offset + size > mi->inode_size)
			size = mi->inode_size - fd->offset;
		    if (size <= 0) size = 0;
		    to = wdata;
		}
		if (!fd->inuse) goto done;
		if (size > 0) bcopy(from, to, size);
		fd->offset += size;
		tsize-=size;
		wdata += size;
	    }
	    rel_it(fileno, interrupt);
done:
	    rval[0] = count - tsize;
	    *result = ESUCCESS;
	    return TRUE;
	  }
	  share_unlock(&fd->lock);
	  *result = EBADF;
	  e_emulator_error("e_maprw:%d (r/w) %d/%d",which,fd->read,fd->write);
	  return TRUE;
	}
	share_unlock(&fd->lock);
	return FALSE;
}

/* called with share_lock(fd->lock) held */
get_it(fileno, interrupt)
	int fileno;
	int *interrupt;
{
	register struct file_info *fd = &shared_base_rw->us_file_info[fileno];

	if (!fd->control || fd->inuse ) {
	    share_unlock(&fd->lock);
	    bsd_maprw_request_it(our_bsd_server_port, interrupt, fileno);
	    return;
	}
	fd->inuse = TRUE;
	share_unlock(&fd->lock);
}

rel_it(fileno, interrupt)
	int fileno;
	int *interrupt;
{
	register struct file_info *fd = &shared_base_rw->us_file_info[fileno];

	share_lock(&fd->lock);
	if (fd->wants) {
		share_unlock(&fd->lock);
		bsd_maprw_release_it(our_bsd_server_port, interrupt, fileno);
	} else {
		fd->inuse = FALSE;
		share_unlock(&fd->lock);
	}
}

#endif	MAP_FILE
#endif	MAP_UAREA
