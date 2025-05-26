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
 * $Log:	vfs_conf.c,v $
 * Revision 2.2  92/07/08  16:19:56  mrt
 * 	Added isofs.
 * 	[92/06/25            rwd]
 * 
 * Revision 2.1  92/04/21  17:13:11  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1989 The Regents of the University of California.
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
 *	@(#)vfs_conf.c	7.3 (Berkeley) 6/28/90
 */

#include <nfs.h>
#include <isofs.h>

#include <sys/param.h>
#include <sys/mount.h>

/*
 * This specifies the filesystem used to mount the root.
 * This specification should be done by /etc/config.
 */
extern int ufs_mountroot();
int (*mountroot)() = ufs_mountroot;

/*
 * These define the root filesystem and device.
 */
struct mount *rootfs;
struct vnode *rootdir;

/*
 * Set up the filesystem operations for vnodes.
 * The types are defined in mount.h.
 */
extern	struct vfsops ufs_vfsops;

#if NFS
extern	struct vfsops nfs_vfsops;
#endif

#if MFS
extern	struct vfsops mfs_vfsops;
#endif

#if ISOFS
extern	struct vfsops isofs_vfsops;
#endif

struct vfsops *vfssw[] = {
	(struct vfsops *)0,	/* 0 = MOUNT_NONE */
	&ufs_vfsops,		/* 1 = MOUNT_UFS */
#if NFS
	&nfs_vfsops,		/* 2 = MOUNT_NFS */
#else
	(struct vfsops *)0,
#endif
#if MFS
	&mfs_vfsops,		/* 3 = MOUNT_MFS */
#else
	(struct vfsops *)0,
#endif
	(struct vfsops *)0,	/* 4 = MOUNT_PC */
#if ISOFS
	&isofs_vfsops,		/* 5 = MOUNT_ISOFS */
#else
	(struct vfsops *)0,
#endif
};
