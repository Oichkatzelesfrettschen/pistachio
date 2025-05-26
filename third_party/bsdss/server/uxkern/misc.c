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
 * $Log:	misc.c,v $
 * Revision 2.3  92/05/25  14:46:29  rwd
 * 	Added syscalltrace().
 * 	[92/05/25            rwd]
 * 
 * Revision 2.2  92/04/22  14:01:15  rwd
 * 	Remove some obsolete code.  Fix includes.
 * 	[92/04/22            rwd]
 * 
 * Revision 2.1  92/04/21  17:10:55  rwd
 * BSDSS
 * 
 *
 */
#include <map_time.h>
#include <second_server.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/buf.h>
#include <sys/mount.h>
#include <ufs/fs.h>
#include <sys/time.h>
#include <sys/reboot.h>

#include <uxkern/device.h>
#include <uxkern/device_utils.h>
#include <uxkern/import_mach.h>
#include <uxkern/syscalltrace.h>

extern mach_port_t	privileged_host_port;
extern mach_port_t	default_processor_set;

#if	SECOND_SERVER
extern int	second_server;
#endif	/* SECOND_SERVER */

swapon(){
    printf("swapon called");
}

/*
 * New kernel interfaces.
 */

#if MAP_TIME
time_value_t *mtime = NULL;

init_mapped_time()
{
	kern_return_t rc;
	mach_port_t device_port, pager = MACH_PORT_NULL;

	rc = device_open(device_server_port,0,"time",&device_port);
	if (rc != D_SUCCESS) panic("unable to open device time");

	rc = device_map(device_port, VM_PROT_READ,
			0, sizeof(time_value_t), &pager, 0);
	if (rc != D_SUCCESS) panic("unable to map device time");
	if (pager == MACH_PORT_NULL) panic("unable to map device time");
	
	rc = vm_map(mach_task_self(), &mtime, sizeof(time_value_t), 0, TRUE,
		    pager, 0, 0, VM_PROT_READ, 
		    VM_PROT_READ, VM_INHERIT_NONE);
	if (rc != D_SUCCESS) panic("unable to vm_map device time");

	rc = mach_port_deallocate(mach_task_self(), pager);
	if (rc != KERN_SUCCESS) panic("unable to deallocate pager");
}

microtime(tvp)
	struct timeval *tvp;
{
	*tvp = *(struct timeval *)mtime;
}

#else MAP_TIME
init_mapped_time(){}

get_time(tvp)
	struct timeval *tvp;
{
	time_value_t	time_value;

	kern_timestamp(&time_value);
	tvp->tv_sec = time_value.seconds;
	tvp->tv_usec = time_value.microseconds;
}

microtime(tvp)
	struct timeval *tvp;
{
	get_time(tvp);
}

#endif MAP_TIME

set_time(tvp)
	struct timeval *tvp;
{
	time_value_t	time_value;

	time_value.seconds = tvp->tv_sec;
	time_value.microseconds = tvp->tv_usec;

	(void) host_set_time(privileged_host_port, time_value);
}

int	waittime = -1;

Debugger()
{
#if	SECOND_SERVER
	if (second_server) {
		printf("Debugger\n");
#ifdef	i386
		asm("int3");
#else	i386
		task_suspend(mach_task_self());
#endif	i386
		return;
	}
#endif	/* SECOND_SERVER */

	boot(0, RB_NOSYNC | RB_DEBUGGER);

}

boot(paniced, flags)
	int	paniced;
	int	flags;
{
#if 0
	if (((flags & RB_NOSYNC) == 0) &&
	    (waittime < 0) &&
	    (bfreelist[0].b_forw != 0) &&
	    (mount[0].m_bufp != 0)) {
	    /*
	     * Force an accurate time into the root file system superblock.
	     */
	    mount[0].m_bufp->b_un.b_fs->fs_fmod = 1;

	    waittime = 0;
	    (void) splnet();
	    printf("syncing disks... ");

	    update();

	    {
		register struct buf *bp;
		int	iter, nbusy, obusy;

		obusy = 0;

		for (iter = 0; iter < 20; iter++) {
		    nbusy = 0;
		    for (bp = &buf[nbuf]; --bp >= buf; )
			if ((bp->b_flags & (B_BUSY | B_INVAL)) == B_BUSY)
			    nbusy++;
		    if (nbusy == 0)
			break;
		    printf("%d ", nbusy);

		    if (nbusy != obusy)
			iter = 0;
		    obusy = nbusy;

		    DELAY(40000 * iter);
		}
	    }
	    printf("done\n");

	    /*
	     * resettodr()?
	     */
	}
#else	0
	printf("WARNING: No update\n");
#endif	0

#if	SECOND_SERVER
	if (second_server) {
		second__exit();
		return;
	}
#endif	/* SECOND_SERVER */
	(void) host_reboot(privileged_host_port, flags);
}

thread_read_times(thread, utv, stv)
	thread_t	thread;
	time_value_t	*utv;
	time_value_t	*stv;
{
	struct thread_basic_info	bi;
	unsigned int			bi_count;

	bi_count = THREAD_BASIC_INFO_COUNT;
	(void) thread_info(thread,
			   THREAD_BASIC_INFO,
			   (thread_info_t)&bi,
			   &bi_count);

	*utv = bi.user_time;
	*stv = bi.system_time;
}

/*
 *	Priorities run from 0 (high) to 31 (low).
 *	The user base priority is 12.
 *	priority = 12 + nice / 2.
 */

set_thread_priority(thread, pri)
	thread_t	thread;
	int		pri;
{
	(void) thread_max_priority(thread, default_processor_set, pri);
	(void) thread_priority(thread, pri, FALSE);
}

#if MAP_UAREA
#include <sys/user.h>
#include <kern/parallel.h>
#define BACKOFF_SECS 5
#define SHARED_PRIORITY PSPECL

extern int hz;

boolean_t share_try_lock(p, lock)
	register struct proc *p;
	register struct shared_lock *lock;
{
	if (spin_try_lock(&lock->lock)) {
		if (p->p_shared_rw->us_inuse)
			lock->who = (int)p | KERNEL_USER;
		u.uu_share_lock_count++;
		return TRUE;
	} else
		return FALSE;
}

void share_lock(x, p)
register struct shared_lock *x;
register struct proc *p;
{
    if (p->p_shared_rw->us_inuse) {
      while (!spin_try_lock(&(x)->lock) && !share_lock_solid((x)));
      (x)->who = (int)(p) | KERNEL_USER;
    } else {
      spin_lock(&(x)->lock);
    }
    u.uu_share_lock_count++;
}

void share_unlock(x, p)
register struct shared_lock *x;
register struct proc *p;
{
    if (p->p_shared_rw->us_inuse) {
      (x)->who = 0;
      spin_unlock(&(x)->lock);
      if ((x)->need_wakeup) {
	unix_master();
	wakeup(x);
	unix_release();
      }
    } else {
      spin_unlock(&(x)->lock);
    }
    if (u.uu_share_lock_count-- == 0) {
	panic("share_unlock < 0");
	u.uu_share_lock_count = 0;
    }
}

int share_lock_solid(x)
	register struct shared_lock *x;
{
	unix_master();
	x->need_wakeup++;
	if (spin_try_lock(&x->lock)) {
		x->need_wakeup--;
		unix_release();
		return 1;
	}
/*	printf("[%x]Share_lock_solid %x\n",(int)x, x->who);*/
	if (tsleep(x, SHARED_PRIORITY,"share lock solid", BACKOFF_SECS * hz) ==
								 EWOULDBLOCK) {
		if ((x->who & ~KERNEL_USER >= (int)proc) &&
		    (x->who & ~KERNEL_USER <= (int)procNPROC)) {
		    printf("[%x]Unable to get lock from %x\n",(int)x, x->who);
		    panic("share_lock_solid");
		} else {
		    printf("[%x]Taking scribbled share_lock\n",(int)x);
		    share_lock_init(x);
		    x->need_wakeup++;
		}
	}
	(x)->need_wakeup--; /* This protected by the master lock */
/*	printf("[%x]Share_lock_solid\n",(int)x);*/
	unix_release();
	return 0;
}

#endif MAP_UAREA

sysctrace(p, uap, retval)
    register struct proc *p;
    register struct args {
	int pid;
    } *uap;
    int *retval;
{
#if SYSCALLTRCAE
    syscalltrace = uap->pid;
#endif
}
