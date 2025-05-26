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
 * $Log:	isofs_node.h,v $
 * Revision 2.2  92/07/08  16:17:31  mrt
 * 	BSDSS
 * 	[92/06/25  18:06:55  rwd]
 * 
 *
 */

struct iso_node {
	struct	iso_node *i_chain[2]; /* hash chain, MUST be first */
	struct	vnode *i_vnode;	/* vnode associated with this inode */
	struct	vnode *i_devvp;	/* vnode for block I/O */
	u_long	i_flag;		/* see below */
	dev_t	i_dev;		/* device where inode resides */
	ino_t	i_number;	/* the identity of the inode */
	struct	iso_mnt *i_mnt;	/* filesystem associated with this inode */
	struct	lockf *i_lockf;	/* head of byte-level lock list */
	long	i_diroff;	/* offset in dir, where we found last entry */
	off_t	i_endoff;	/* end of useful stuff in directory */
	long	i_spare0;
	long	i_spare1;


	int iso_reclen;
	int iso_extlen;
	int iso_extent;
	int i_size;
	unsigned char iso_time[7];
	int iso_flags;
	int iso_unit_size;
	int iso_interleave_gap;
	int iso_volume_seq;
	int iso_namelen;
};

#define	i_forw		i_chain[0]
#define	i_back		i_chain[1]

/* flags */
#define	ILOCKED		0x0001		/* inode is locked */
#define	IWANT		0x0002		/* some process waiting on lock */
#define	IACC		0x0020		/* inode access time to be updated */

#define VTOI(vp) ((struct iso_node *)(vp)->v_data)
#define ITOV(ip) ((ip)->i_vnode)

#define ISO_ILOCK(ip)	iso_ilock(ip)
#define ISO_IUNLOCK(ip)	iso_iunlock(ip)

#define VT_ISOFS (VT_MFS+1)

/*
 * Prototypes for ISOFS vnode operations
 */
int isofs_lookup __P((struct vnode *vp, struct nameidata *ndp, struct proc *p));
int isofs_open __P((struct vnode *vp, int mode, struct ucred *cred,
	struct proc *p));
int isofs_close __P((struct vnode *vp, int fflag, struct ucred *cred,
	struct proc *p));
int isofs_access __P((struct vnode *vp, int mode, struct ucred *cred,
	struct proc *p));
int isofs_getattr __P((struct vnode *vp, struct vattr *vap, struct ucred *cred,
	struct proc *p));
int isofs_read __P((struct vnode *vp, struct uio *uio, int ioflag,
	struct ucred *cred));
int isofs_ioctl __P((struct vnode *vp, int command, caddr_t data, int fflag,
	struct ucred *cred, struct proc *p));
int isofs_select __P((struct vnode *vp, int which, int fflags, struct ucred *cred,
	struct proc *p));
int isofs_mmap __P((struct vnode *vp, int fflags, struct ucred *cred,
	struct proc *p));
int isofs_seek __P((struct vnode *vp, off_t oldoff, off_t newoff,
	struct ucred *cred));
int isofs_readdir __P((struct vnode *vp, struct uio *uio, struct ucred *cred,
	int *eofflagp));
int isofs_abortop __P((struct nameidata *ndp));
int isofs_inactive __P((struct vnode *vp, struct proc *p));
int isofs_reclaim __P((struct vnode *vp));
int isofs_lock __P((struct vnode *vp));
int isofs_unlock __P((struct vnode *vp));
int isofs_strategy __P((struct buf *bp));
int isofs_print __P((struct vnode *vp));
int isofs_islocked __P((struct vnode *vp));

