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
 * $Log:	nfsmount.h,v $
 * Revision 2.1  92/04/21  17:15:22  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
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
 *	@(#)nfsmount.h	7.7 (Berkeley) 4/16/91
 */

/*
 * Mount structure.
 * One allocated on every NFS mount.
 * Holds NFS specific information for mount.
 */
struct	nfsmount {
	int	nm_flag;		/* Flags for soft/hard... */
	struct	mount *nm_mountp;	/* Vfs structure for this filesystem */
	nfsv2fh_t nm_fh;		/* File handle of root dir */
	struct	socket *nm_so;		/* Rpc socket */
	int	nm_sotype;		/* Type of socket */
	int	nm_soproto;		/* and protocol */
	int	nm_soflags;		/* pr_flags for socket protocol */
	struct	mbuf *nm_nam;		/* Addr of server */
	short	nm_retry;		/* Max retry count */
	short	nm_rexmit;		/* Rexmit on previous request */
	short	nm_rtt;			/* Round trip timer ticks @ NFS_HZ */
	short	nm_rto;			/* Current timeout */
	short	nm_srtt;		/* Smoothed round trip time */
	short	nm_rttvar;		/* RTT variance */
	short	nm_currto;		/* Current rto of any nfsmount */
	short	nm_currexmit;		/* Max rexmit count of nfsmounts */
	short	nm_sent;		/* Request send count */
	short	nm_window;		/* Request send window (max) */
	short	nm_winext;		/* Window incremental value */
	short	nm_ssthresh;		/* Slowstart threshold */
	short	nm_salen;		/* Actual length of nm_sockaddr */
	int	nm_rsize;		/* Max size of read rpc */
	int	nm_wsize;		/* Max size of write rpc */
};

#ifdef KERNEL
/*
 * Convert mount ptr to nfsmount ptr.
 */
#define VFSTONFS(mp)	((struct nfsmount *)((mp)->mnt_data))
#endif /* KERNEL */

/*
 * Prototypes for NFS mount operations
 */
int	nfs_mount __P((
		struct mount *mp,
		char *path,
		caddr_t data,
		struct nameidata *ndp,
		struct proc *p));
int	nfs_start __P((
		struct mount *mp,
		int flags,
		struct proc *p));
int	nfs_unmount __P((
		struct mount *mp,
		int mntflags,
		struct proc *p));
int	nfs_root __P((
		struct mount *mp,
		struct vnode **vpp));
int	nfs_quotactl __P((
		struct mount *mp,
		int cmds,
		int uid,	/* should be uid_t */
		caddr_t arg,
		struct proc *p));
int	nfs_statfs __P((
		struct mount *mp,
		struct statfs *sbp,
		struct proc *p));
int	nfs_sync __P((
		struct mount *mp,
		int waitfor));
int	nfs_fhtovp __P((
		struct mount *mp,
		struct fid *fhp,
		struct vnode **vpp));
int	nfs_vptofh __P((
		struct vnode *vp,
		struct fid *fhp));
int	nfs_init __P(());
