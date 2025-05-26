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
 * $Log:	init_main.c,v $
 * Revision 2.3  92/07/08  15:48:43  mrt
 * 	Recast arguements in vm_allocate calls to eliminate warnings.
 * 	[92/07/06            mrt]
 * 
 * Revision 2.2  92/06/25  17:24:58  mrt
 * 	Change stack size to 80 pages until I get a chance to fix
 * 	make the grow code work.
 * 	[92/06/15            rwd]
 * 
 * Revision 2.1  92/04/21  17:13:01  rwd
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
 *	@(#)init_main.c	7.41 (Berkeley) 5/15/91
 */

#include <sysvshm.h>
#include <gprof.h>
#include <second_server.h>

#include <uxkern/syscalltrace.h>
#include <sys/param.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/mount.h>
#include <sys/map.h>
#include <sys/proc.h>
#include <sys/resourcevar.h>
#include <sys/signalvar.h>
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/seg.h>
#include <sys/conf.h>
#include <sys/buf.h>
#include <sys/clist.h>
#include <sys/malloc.h>
#include <sys/protosw.h>
#include <sys/reboot.h>
#include <sys/synch.h>
#include <sys/user.h>
#include <sys/parallel.h>
#include <sys/fcntl.h>
#include <sys/mbuf.h>
#include <sys/msgbuf.h>

#include <ufs/quota.h>

#include <uxkern/import_mach.h>

/*
 * Components of process 0;
 * never freed.
 */
struct	session session0;
struct	pgrp pgrp0;
struct	proc proc0;
struct	pcred cred0;
struct	filedesc0 filedesc0;
struct	plimit limit0;
struct  vmspace vmspace0;
struct	proc *initproc, *pageproc;

int	cmask = CMASK;
struct user proc0user;
struct user *proc0paddr = & proc0user;
extern	int (*mountroot)();
struct mutex allproc_lock = MUTEX_INITIALIZER;
int system_procs = 2; /*XXX governed by allproc_lock also.  Start at -3
		        XXX because of task_py_pid garbage*/

struct	vnode *rootvp;
int	boothowto = RB_KDB;
dev_t rootdev;

extern char sigcode;
extern int szsigcode;
char *trampoline_page;
extern int msgbufmapped;

void cinit();

extern struct proc *newproc();

extern any_t	ux_create_thread();

cthread_fn_t	system_setup();	/* forward */

long		avenrun[3] = {0, 0, 0};	/* XXX */

mach_port_t	privileged_host_port;
mach_port_t	host_port;
mach_port_t	device_server_port;
mach_port_t	default_processor_set;
mach_port_t	default_processor_set_name;
struct condition kill_main = CONDITION_INITIALIZER;
struct mutex kill_lock = MUTEX_INITIALIZER;
int debugger_thread = 0;

/*
 * System startup; initialize the world, create process 0,
 * mount root filesystem, and fork to create init and pagedaemon.
 * Most of the hard work is done in the lower-level initialization
 * routines including startup(), which does memory initialization
 * and autoconfiguration.
 */
main(argc,argv)
    int argc;
    char **argv;
{
	allproc = &proc0;
	proc0.p_prev = &allproc;

	/*
	 * Wire down this thread until it becomes a server thread
	 */

	cthread_wire();

	/*
	 * Initialize msgbuf for logging
	 */

	msgbuf_init();

	/*
	 * Get initial ports and arguments.
	 */
	get_config_info(argc, argv);

	/*
	 * Get a port to talk to the world.
	 */
	console_init();

	/*
	 * Setup mappable time
	 */
	init_mapped_time();

	/*
	 * Initialize SPL emulator
	 */
	spl_init();

	/*
	 * Allocate tables - from wired-down memory
	 */
	alloc_tables();

	zone_init();

	/*
	 * Initialize mock wait queues
	 */
	rqinit();

	/*
	 * Start callout thread
	 */
	callout_init();

	/*
	 * Start device reply server
	 */
	dev_utils_init();
	device_reply_hdlr();

	ux_server_init();
	/*
	 * Turn into a U*X thread, to do the rest of the
	 * initialization
	 */
	(void) ux_create_thread(system_setup);

	/*
	 * Unwire now
	 */
	cthread_unwire();

	/*
	 * This should never return from this condition wait
	 */
	if (debugger_thread) {
	    mach_port_t		bogus_port;
	    mach_msg_header_t	bogus_msg;
	    int			limit;

	    cthread_wire();
	    limit = cthread_kernel_limit();
	    if (limit != 0)
		cthread_set_kernel_limit(limit + 1);
	    (void) mach_port_allocate(mach_task_self(),
				      MACH_PORT_RIGHT_RECEIVE, &bogus_port);
	    while (1) {
		(void) mach_msg(&bogus_msg, MACH_RCV_MSG|MACH_RCV_TIMEOUT,
				0, 0, bogus_port, 5000, MACH_PORT_NULL);
	    }
	} else {
	    mutex_lock(&kill_lock);
	    condition_wait(&kill_main,&kill_lock);
	}
}

cthread_fn_t system_setup()
{
	register int i;
	register struct proc *p;
	register struct filedesc0 *fdp;
	int s, rval[2];
	mach_port_t ux_exception_port;
	int error;
	kern_return_t kr;

	p = &proc0;

	p->p_task = mach_task_self();
	p->p_thread = mach_thread_self();
	p->p_ipl = 0;
	p->p_master_lock = 0;
	p->p_servers = 0;
	condition_init(&p->p_condition);
	mutex_init(&p->p_lock);
	p->p_pid = 0;

	server_thread_register(p);

	kmeminit();

	p->p_pgrp = &pgrp0;
	pgrphash[0] = &pgrp0;
	pgrp0.pg_mem = p;
	pgrp0.pg_session = &session0;
	session0.s_count = 1;
	session0.s_leader = p;

	p->p_flag = SLOAD|SSYS;
	p->p_stat = SRUN;
	p->p_nice = NZERO;

	/*
	 * Setup credentials
	 */
	cred0.p_refcnt = 1;
	p->p_cred = &cred0;
	p->p_ucred = crget();
	p->p_ucred->cr_ngroups = 1;	/* group 0 */

	/*
	 * Create the file descriptor table for process 0.
	 */
	fdp = &filedesc0;
	p->p_fd = &fdp->fd_fd;
	fdp->fd_fd.fd_refcnt = 1;
	fdp->fd_fd.fd_cmask = cmask;
	fdp->fd_fd.fd_ofiles = fdp->fd_dfiles;
	fdp->fd_fd.fd_ofileflags = fdp->fd_dfileflags;
	fdp->fd_fd.fd_nfiles = NDFILE;
	fdp->fd_fd.fd_lastfile = 0;
	fdp->fd_fd.fd_freefile = 0;

	/*
	 * Set initial limits
	 */
	p->p_limit = &limit0;
	for (i = 0; i < sizeof(p->p_rlimit)/sizeof(p->p_rlimit[0]); i++)
		limit0.pl_rlimit[i].rlim_cur =
		    limit0.pl_rlimit[i].rlim_max = RLIM_INFINITY;
	limit0.pl_rlimit[RLIMIT_OFILE].rlim_cur = NOFILE;
	limit0.pl_rlimit[RLIMIT_NPROC].rlim_cur = MAXUPRC;
	limit0.p_refcnt = 1;

	p->p_vmspace = &vmspace0;
	vmspace0.vm_ssize = 80;				/* XXX */
	vmspace0.vm_refcnt = 1;
	p->p_addr = proc0paddr;				/* XXX */

	/*
	 * We continue to place resource usage info
	 * and signal actions in the user struct so they're pageable.
	 */
	p->p_stats = &p->p_addr->u_stats;
	p->p_sigacts = &p->p_addr->u_sigacts;

	bufinit();

	unix_master();

	/*
	 * Initialize the file systems.
	 *
	 * Get vnodes for rootdev.
	 */
	vfsinit();
	if (bdevvp(rootdev, &rootvp))
	    panic("can't setup bdevvp's");


	/*
	 * Initialize tables, protocols, and set up well-known inodes.
	 */
	mbinit();
	cinit();
#if SYSVSHM
	shminit();
#endif
#include <sl.h>
#if NSL > 0
	slattach();			/* XXX */
#endif
#include <loop.h>
#if NLOOP > 0
	loattach();			/* XXX */
#endif
	/*
	 * Start network server thread.
	 */
	netisr_init();

	/*
	 * Block reception of incoming packets
	 * until protocols have been initialized.
	 */
	s = splimp();
	ifinit();
	domaininit();
	splx(s);

#if GPROF
	kmstartup();
#endif

	task_to_proc_init();

	/*
	 * Open the console.
	 */
	(void) cons_open(makedev(0,0), FREAD|FWRITE);

	printf("%s\n",version);

	vnode_pager_init();

	/*
	 * Set up the root file system and vnode.
	 */
	if (error =(*mountroot)())
		panic("cannot mount root", error);
	/*
	 * Get vnode for '/'.
	 * Setup rootdir and fdp->fd_fd.fd_cdir to point to it.
	 */
	if (VFS_ROOT(rootfs, &rootdir))
		panic("cannot find root vnode");
	fdp->fd_fd.fd_cdir = rootdir;
	VREF(fdp->fd_fd.fd_cdir);
	VOP_UNLOCK(rootdir);
	fdp->fd_fd.fd_rdir = NULL;

	/*
	 * Now can look at time, having had a chance
	 * to verify the time from the file system.
	 */
	get_time(&boottime);
	p->p_stats->p_start = boottime;

	/*
	 *XXX
	 *XXX Make page for trampoline code to copy into each task
	 *XXX
	 */

	kr = vm_allocate(mach_task_self(), (vm_address_t *) &trampoline_page, 
			NBPG, TRUE);
	if (kr != KERN_SUCCESS)
	    panic("allocate trampoline_page",kr);

	bcopy(&sigcode, trampoline_page+NBPG-TRAMPOLINE_MAX_SIZE, szsigcode);

	/*
	 * make init process
	 */
	siginit(p);

	initproc = newproc(p, TRUE, FALSE);

	/* exception handler */
	ux_exception_port = ux_handler_setup();
	kr = task_set_exception_port(initproc->p_task,
				     ux_exception_port);
	if (kr != KERN_SUCCESS)
	    panic("system_setup: can't set exception port");
	
	/* LOAD THE EMULATION LIBRARY */
	
	/*
	 * Load the emulator library
	 */
	error = load_emulator(initproc);
	if (error)
	    panic("load_emulator x%x", error);
	/*
	 * Start the first task!
	 */
	(void) thread_resume(initproc->p_thread);

	/*
	 * Release the master lock...
	 */
	unix_release();

	/*
	 * Let ps know what we are
	 */
	strncpy(p->p_comm, "BSD4.4 server", sizeof(p->p_comm));

	/*
	 * Become the first server thread
	 */
	server_thread_deregister(p);
	ux_server_loop();
	/*NOTREACHED*/
}

int	nbuf = 0;
int	nsbuf = 0;
int	bufpages = 0;
vm_size_t	memory_size;

alloc_tables()
{
	register vm_offset_t firstaddr, v;
	vm_offset_t	alloc_addr;
	vm_size_t	size;
	kern_return_t kr;

	{
	    vm_statistics_data_t vm_stat;
	    vm_statistics(mach_task_self(), &vm_stat);
	    /*
	     *	Consider wired down memory not available
	     */
	    memory_size = vm_stat.pagesize * (vm_stat.free_count +
					      vm_stat.active_count +
					      vm_stat.inactive_count);

	    bufpages = memory_size / 10 / vm_stat.pagesize;
	}

	/*
	 *	Use 5% of memory for buffers.
	 *	Since these pages are virtual-size pages (larger
	 *	than physical page size), use only one page
	 *	per buffer.
	 */

	if (nbuf == 0) {
		nbuf = bufpages / 2;
		if (nbuf < 16)
			nbuf = 16;
	}
	if (nswbuf == 0) {
		nswbuf = (nbuf / 2) &~ 1;	/* force even */
		if (nswbuf > 256)
			nswbuf = 256;		/* sanity */
	}
#define	valloc(name, type, num) \
	    (name) = (type *)v; v = (vm_offset_t)((name)+(num))
#define	valloclim(name, type, num, lim) \
	    (name) = (type *)v; v = (vm_offset_t)((lim) = ((name)+(num)))

	/*
	 *	We make two passes over the table allocation code.
	 *	The first pass merely calculates the size needed
	 *	for the various data structures.  The second pass
	 *	really allocates memory and then sets the actual
	 *	addresses.  The code must not change any of the
	 *	allocated sizes between the two passes.
	 */
	firstaddr = 0;
	for (;;) {
	    v = firstaddr;	    
	    valloc(cfree, struct cblock, nclist);
#if SYSVSHM
	    valloc(shmsegs, struct shmid_ds, shminfo.shmmni);
#endif
	    valloc(swbuf, struct buf, nswbuf);
	    valloc(buf, struct buf, nbuf);
	    if (firstaddr == 0) {
		/*
		 *	Size has been calculated; allocate memory.
		 */
		size = (vm_size_t)(v - firstaddr);
		if (vm_allocate(mach_task_self(),
				&alloc_addr,
				size,
				TRUE)
			!= KERN_SUCCESS)
		    panic("startup: no room for tables");
		firstaddr = alloc_addr;
	    }
	    else {
		/*
		 *	Memory has been allocated.  Make sure that
		 *	table size has not changed.
		 */
		if ((vm_size_t)(v - firstaddr) != size)
		    panic("startup: table size inconsistent");
		break;
	    }
	}
	/*
	 * Finally, allocate mbuf pool.  Since mclrefcnt is an off-size
	 * we use the more space efficient malloc in place of kmem_alloc.
	 */
	kr = vm_allocate(mach_task_self(), (vm_address_t *)  &mclrefcnt, 
			NMBCLUSTERS+CLBYTES/MCLBYTES, TRUE);
	if (kr != KERN_SUCCESS)
	    panic("allocating mclrefcnt",kr);
	kr = vm_allocate(mach_task_self(), (vm_address_t *) &mbutl,
			(NMBCLUSTERS*MCLBYTES),TRUE);
	if (kr != KERN_SUCCESS)
	    panic("allocating mbutl",kr);
}
extern dev_t	parse_root_device();
extern char *emulator_name;
extern char *init_program_name;
extern char **init_program_args;
#if	SECOND_SERVER
int  second_server = 0;		/* Run under another server ? */
#endif	/* SECOND_SERVER */

get_config_info(argc, argv)
	int	argc;
	char	**argv;
{
	mach_port_t	bootstrap_port;
	mach_port_t	reply_port;
	kern_return_t	result;

	struct imsg {
	    mach_msg_header_t	hdr;
	    mach_msg_type_t	port_desc_1;
	    mach_port_t		port_1;
	    mach_msg_type_t	port_desc_2;
	    mach_port_t		port_2;
	} imsg;

#if	SECOND_SERVER
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == '2') {
		second_server = 1; /* run as a unix process of another server */
	}
	if (second_server) {
		if ((privileged_host_port = second_task_by_pid(-1)) == MACH_PORT_NULL)
			panic("failed to get privileged host port");	
		if ((device_server_port = second_task_by_pid(-2)) == MACH_PORT_NULL)
			panic("failed to get device server port");
	} else {
#endif	/* SECOND_SERVER */
	/*
	 * Get our bootstrap port
	 */
	result = task_get_bootstrap_port(mach_task_self(), &bootstrap_port);
	if (result != KERN_SUCCESS)
	    panic("get bootstrap port %d", result);

	/*
	 * Allocate a reply port
	 */
	reply_port = mach_reply_port();
	if (reply_port == MACH_PORT_NULL)
	    panic("allocate reply port");

	/*
	 * Send a message to it, asking for the host and device ports
	 */
	imsg.hdr.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,
					    MACH_MSG_TYPE_MAKE_SEND_ONCE);
	imsg.hdr.msgh_size = 0;
	imsg.hdr.msgh_remote_port = bootstrap_port;
	imsg.hdr.msgh_local_port = reply_port;
	imsg.hdr.msgh_kind = MACH_MSGH_KIND_NORMAL;
	imsg.hdr.msgh_id = 999999;

	result = mach_msg(&imsg.hdr, MACH_SEND_MSG|MACH_RCV_MSG,
			  sizeof imsg.hdr, sizeof imsg, reply_port,
			  MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	if (result != MACH_MSG_SUCCESS)
	    panic("mach_msg");

	privileged_host_port = imsg.port_1;
	device_server_port = imsg.port_2;
#if	SECOND_SERVER
	}
#endif	/* SECOND_SERVER */

	host_port = mach_host_self();

	/*
	 * Lookup our default processor-set name/control ports.
	 */

	(void) processor_set_default(mach_host_self(),
				     &default_processor_set_name);
	(void) host_processor_set_priv(privileged_host_port,
				       default_processor_set_name,
				       &default_processor_set);

	/*
	 * Parse the arguments.
	 */

	/*
	 * Arg 0 is program name
	 */
	argv++, argc--;

	/*
	 * Arg 1 should be flags
	 */
	if (argc == 0)
	    return;

	while (argv[0][0] == '-') {
	    register char *cp = argv[0];
	    register char c;

	    while ((c = *cp++) != '\0') {
		switch (c) {
		    case 'a':
			boothowto |= RB_ASKNAME;
			break;
		    case 's':
			boothowto |= RB_SINGLE;
			break;
		    case 'd':
			boothowto |= RB_KDB;
			break;
		    case 'n':
			boothowto |= RB_INITNAME;
			break;
#if	SECOND_SERVER
		    case '2':	 
			if (second_server != 1) {
				second_server = 1;
				printf("error: '-2' must be the first argument.\n");
			}
			break;
		    case 'h':
			if (second_server) {
			    boothowto |= RB_KDB;
			    printf("Suspended and ready to continue.\n");
			    task_suspend(mach_task_self());
			}
			break;
		    case 'e':
			/* Allow non-default emulator file name: */
			emulator_name = argv[1];
			/* Skip over the file name argument: */
			argv++, argc--;
			break;
		    case 'i':
			/* Allow non-default init program file name: */
			init_program_name = argv[1];
			/* Skip over the file name argument: */
			argv++, argc--;
			break;
#endif	/* SECOND_SERVER */
		    case 'v':
			/* Turn on syscall tracing: */
			syscalltrace = -1;
			break;
		}
	    }
	    argv++, argc--;
	}
	/*
	 * Arg 2 should be root name
	 */
	if (argc == 0)
	    return;

	rootdev = parse_root_device(argv[0]);
}

msgbuf_init()
{
    kern_return_t kr;

    kr = vm_allocate(mach_task_self(), (vm_address_t *) &msgbufp, 
			sizeof(struct msgbuf),
		     TRUE);
    if (kr != KERN_SUCCESS)
	panic("allocating msgbuf",kr);
    msgbufmapped = 1;
}

void system_proc(np, name)
    struct proc **np;
    char *name;
{
	struct proc **hash;
	struct proc *p;

	proc_allocate(np);
	p = *np;
	p->p_ipl = 0;
	p->p_master_lock = 0;
	condition_init(&p->p_condition);
	mutex_init(&p->p_lock);
	p->p_task = mach_task_self();
	p->p_thread = mach_thread_self();

	cthread_set_data(cthread_self(),(any_t)p);
	cthread_set_name(cthread_self(), name);

	strcpy(p->p_comm, name);

	mutex_lock(&allproc_lock);
	p->p_pid = -(++system_procs);
	mutex_unlock(&allproc_lock);
	hash = &pidhash[PIDHASH(p->p_pid)];
	p->p_hash = *hash;
	*hash = p;

	p->p_pgrp = &pgrp0;
	p->p_flag = SLOAD|SSYS;
	p->p_stat = SRUN;
	p->p_nice = NZERO;
	p->p_cred = &cred0;
	p->p_ucred = crget();
	p->p_ucred->cr_ngroups = 1;	/* group 0 */
	p->p_limit = &limit0;
	p->p_vmspace = &vmspace0;
	p->p_addr = proc0paddr;				/* XXX */
	p->p_stats = &p->p_addr->u_stats;
	p->p_sigacts = &p->p_addr->u_sigacts;
}
