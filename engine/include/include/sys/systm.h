#pragma once
/*-
 * Copyright (c) 1982, 1988, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)systm.h	8.4 (Berkeley) 2/23/94
 */

/*
 * The `securelevel' variable controls the security level of the system.
 * It can only be decreased by process 1 (/sbin/init).
 *
 * Security levels are as follows:
 *   -1	permannently insecure mode - always run system in level 0 mode.
 *    0	insecure mode - immutable and append-only flags make be turned off.
 *	All devices may be read or written subject to permission modes.
 *    1	secure mode - immutable and append-only flags may not be changed;
 *	raw disks of mounted filesystems, /dev/mem, and /dev/kmem are
 *	read-only.
 *    2	highly secure mode - same as (1) plus raw disks are always
 *	read-only whether mounted or not. This level precludes tampering 
 *	with filesystems by unmounting them, but also inhibits running
 *	newfs while the system is secured.
 *
 * In normal operation, the system runs in level 0 mode while single user
 * and in level 1 mode while multiuser. If level 2 mode is desired while
 * running multiuser, it can be set in the multiuser startup script
 * (/etc/rc.local) using sysctl(1). If it is desired to run the system
 * in level 0 mode while multiuser, initialize the variable securelevel
 * in /sys/kern/kern_sysctl.c to -1. Note that it is NOT initialized to
 * zero as that would allow the vmunix binary to be patched to -1.
 * Without initialization, securelevel loads in the BSS area which only
 * comes into existence when the kernel is loaded and hence cannot be
 * patched by a stalking hacker.
 */
extern int securelevel;		/* system security level */
extern const char *panicstr;	/* panic message */
extern char version[];		/* system version */
extern char copyright[];	/* system copyright */

extern int nblkdev;		/* number of entries in bdevsw */
extern int nchrdev;		/* number of entries in cdevsw */
extern int nswdev;		/* number of swap devices */
extern int nswap;		/* size of swap space */

extern int selwait;		/* select timeout address */

extern u_char curpriority;	/* priority of current process */

extern int maxmem;		/* max memory per process */
extern int physmem;		/* physical memory */

extern dev_t dumpdev;		/* dump device */
extern long dumplo;		/* offset into dumpdev */

extern dev_t rootdev;		/* root device */
extern struct vnode *rootvp;	/* vnode equivalent to above */

extern dev_t swapdev;		/* swapping device */
extern struct vnode *swapdev_vp;/* vnode equivalent to above */

extern struct sysent {		/* system call table */
	int	sy_narg;	/* number of arguments */
	int	(*sy_call)();	/* implementing function */
} sysent[];

extern int boothowto;		/* reboot flags, from console subsystem */

/* casts to keep lint happy */
#define	insque(q,p)	_insque((caddr_t)q,(caddr_t)p)
#define	remque(q)	_remque((caddr_t)q)

/*
 * General function declarations.
 */
int	nullop (void);
int	enodev (void);
int	enoioctl (void);
int	enxio (void);
int	eopnotsupp (void);
int	seltrue (dev_t dev, int which, struct proc *p);
void	*hashinit (int count, int type, u_long *hashmask);

void	panic (const char *, ...);
void	tablefull (const char *);
void	addlog (const char *, ...);
void	log (int, const char *, ...);
void	printf (const char *, ...);
int	sprintf (char *buf, const char *, ...);
void	ttyprintf (struct tty *, const char *, ...);

void	ovbcopy (const void *from, void *to, u_int len);

int	copystr (void *kfaddr, void *kdaddr, u_int len, u_int *done);
int	copyinstr (void *udaddr, void *kaddr, u_int len, u_int *done);
int	copyoutstr (void *kaddr, void *udaddr, u_int len, u_int *done);
int	copyin (void *udaddr, void *kaddr, u_int len);
int	copyout (void *kaddr, void *udaddr, u_int len);

int	fubyte (void *base);
#ifdef notdef
int	fuibyte (void *base);
#endif
int	subyte (void *base, int byte);
int	suibyte (void *base, int byte);
int	fuword (void *base);
int	fuiword (void *base);
int	suword (void *base, int word);
int	suiword (void *base, int word);

int	hzto (struct timeval *tv);
void	timeout (void (*func)(void *), void *arg, int ticks);
void	untimeout (void (*func)(void *), void *arg);
void	realitexpire (void *);

struct clockframe;
void	hardclock (struct clockframe *frame);
void	softclock (void);
void	statclock (struct clockframe *frame);

void	initclocks (void);

void	startprofclock (struct proc *);
void	stopprofclock (struct proc *);
void	setstatclockrate (int hzrate);

#include <libkern/libkern.h>
#include <string.h>
