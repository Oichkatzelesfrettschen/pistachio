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
 * $Log:	mfs_vnops.c,v $
 * Revision 2.1  92/04/21  17:18:18  rwd
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
 *	@(#)mfs_vnops.c	7.22 (Berkeley) 4/16/91
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/map.h>
#include <sys/vnode.h>

#include <sys/mfsnode.h>
#include <sys/mfsiom.h>

#include <machine/vmparam.h>
#include <machine/mtpr.h>

#if !defined(hp300) && !defined(i386)
static int mfsmap_want;		/* 1 => need kernel I/O resources */
struct map mfsmap[MFS_MAPSIZE];
extern char mfsiobuf[];
#endif

/*
 * mfs vnode operations.
 */
struct vnodeops mfs_vnodeops = {
	mfs_lookup,		/* lookup */
	mfs_create,		/* create */
	mfs_mknod,		/* mknod */
	mfs_open,		/* open */
	mfs_close,		/* close */
	mfs_access,		/* access */
	mfs_getattr,		/* getattr */
	mfs_setattr,		/* setattr */
	mfs_read,		/* read */
	mfs_write,		/* write */
	mfs_ioctl,		/* ioctl */
	mfs_select,		/* select */
	mfs_mmap,		/* mmap */
	mfs_fsync,		/* fsync */
	mfs_seek,		/* seek */
	mfs_remove,		/* remove */
	mfs_link,		/* link */
	mfs_rename,		/* rename */
	mfs_mkdir,		/* mkdir */
	mfs_rmdir,		/* rmdir */
	mfs_symlink,		/* symlink */
	mfs_readdir,		/* readdir */
	mfs_readlink,		/* readlink */
	mfs_abortop,		/* abortop */
	mfs_inactive,		/* inactive */
	mfs_reclaim,		/* reclaim */
	mfs_lock,		/* lock */
	mfs_unlock,		/* unlock */
	mfs_bmap,		/* bmap */
	mfs_strategy,		/* strategy */
	mfs_print,		/* print */
	mfs_islocked,		/* islocked */
	mfs_advlock,		/* advlock */
};

/*
 * Vnode Operations.
 *
 * Open called to allow memory filesystem to initialize and
 * validate before actual IO. Record our process identifier
 * so we can tell when we are doing I/O to ourself.
 */
/* ARGSUSED */
mfs_open(vp, mode, cred, p)
	register struct vnode *vp;
	int mode;
	struct ucred *cred;
	struct proc *p;
{

	if (vp->v_type != VBLK) {
		panic("mfs_ioctl not VBLK");
		/* NOTREACHED */
	}
	return (0);
}

/*
 * Ioctl operation.
 */
/* ARGSUSED */
mfs_ioctl(vp, com, data, fflag, cred, p)
	struct vnode *vp;
	int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
	struct proc *p;
{

	return (-1);
}

/*
 * Pass I/O requests to the memory filesystem process.
 */
mfs_strategy(bp)
	register struct buf *bp;
{
	register struct mfsnode *mfsp;
	struct vnode *vp;
	struct proc *p = curproc;		/* XXX */

	if (vfinddev(bp->b_dev, VBLK, &vp) || vp->v_usecount == 0)
		panic("mfs_strategy: bad dev");
	mfsp = VTOMFS(vp);
	if (mfsp->mfs_pid == p->p_pid) {
		mfs_doio(bp, mfsp->mfs_baseoff);
	} else {
		bp->av_forw = mfsp->mfs_buflist;
		mfsp->mfs_buflist = bp;
		wakeup((caddr_t)vp);
	}
	return (0);
}

#if defined(vax) || defined(tahoe)
/*
 * Memory file system I/O.
 *
 * Essentially play ubasetup() and disk interrupt service routine by
 * doing the copies to or from the memfs process. If doing physio
 * (i.e. pagein), we must map the I/O through the kernel virtual
 * address space.
 */
mfs_doio(bp, base)
	register struct buf *bp;
	caddr_t base;
{
	register struct pte *pte, *ppte;
	register caddr_t vaddr;
	int off, npf, npf2, reg;
	caddr_t kernaddr, offset;

	/*
	 * For phys I/O, map the b_addr into kernel virtual space using
	 * the Mfsiomap pte's.
	 */
	if ((bp->b_flags & B_PHYS) == 0) {
		kernaddr = bp->b_un.b_addr;
	} else {
		if (bp->b_flags & (B_PAGET | B_UAREA | B_DIRTY))
			panic("swap on memfs?");
		off = (int)bp->b_un.b_addr & PGOFSET;
		npf = btoc(bp->b_bcount + off);
		/*
		 * Get some mapping page table entries
		 */
		while ((reg = rmalloc(mfsmap, (long)npf)) == 0) {
			mfsmap_want++;
			sleep((caddr_t)&mfsmap_want, PZERO-1);
		}
		reg--;
		pte = vtopte(bp->b_proc, btop(bp->b_un.b_addr));
		/*
		 * Do vmaccess() but with the Mfsiomap page table.
		 */
		ppte = &Mfsiomap[reg];
		vaddr = &mfsiobuf[reg * NBPG];
		kernaddr = vaddr + off;
		for (npf2 = npf; npf2; npf2--) {
			mapin(ppte, (u_int)vaddr, pte->pg_pfnum,
				(int)(PG_V|PG_KW));
#if defined(tahoe)
			if ((bp->b_flags & B_READ) == 0)
				mtpr(P1DC, vaddr);
#endif
			ppte++;
			pte++;
			vaddr += NBPG;
		}
	}
	offset = base + (bp->b_blkno << DEV_BSHIFT);
	if (bp->b_flags & B_READ)
		bp->b_error = copyin(offset, kernaddr, bp->b_bcount);
	else
		bp->b_error = copyout(kernaddr, offset, bp->b_bcount);
	if (bp->b_error)
		bp->b_flags |= B_ERROR;
	/*
	 * Release pte's used by physical I/O.
	 */
	if (bp->b_flags & B_PHYS) {
		rmfree(mfsmap, (long)npf, (long)++reg);
		if (mfsmap_want) {
			mfsmap_want = 0;
			wakeup((caddr_t)&mfsmap_want);
		}
	}
	biodone(bp);
}
#endif	/* vax || tahoe */

#if defined(hp300) || defined(i386)
/*
 * Memory file system I/O.
 *
 * Trivial on the HP since buffer has already been mapping into KVA space.
 */
mfs_doio(bp, base)
	register struct buf *bp;
	caddr_t base;
{
	base += (bp->b_blkno << DEV_BSHIFT);
	if (bp->b_flags & B_READ)
		bp->b_error = copyin(base, bp->b_un.b_addr, bp->b_bcount);
	else
		bp->b_error = copyout(bp->b_un.b_addr, base, bp->b_bcount);
	if (bp->b_error)
		bp->b_flags |= B_ERROR;
	biodone(bp);
}
#endif

/*
 * This is a noop, simply returning what one has been given.
 */
mfs_bmap(vp, bn, vpp, bnp)
	struct vnode *vp;
	daddr_t bn;
	struct vnode **vpp;
	daddr_t *bnp;
{

	if (vpp != NULL)
		*vpp = vp;
	if (bnp != NULL)
		*bnp = bn;
	return (0);
}

/*
 * Memory filesystem close routine
 */
/* ARGSUSED */
mfs_close(vp, flag, cred, p)
	register struct vnode *vp;
	int flag;
	struct ucred *cred;
	struct proc *p;
{
	register struct mfsnode *mfsp = VTOMFS(vp);
	register struct buf *bp;

	/*
	 * Finish any pending I/O requests.
	 */
	while (bp = mfsp->mfs_buflist) {
		mfsp->mfs_buflist = bp->av_forw;
		mfs_doio(bp, mfsp->mfs_baseoff);
		wakeup((caddr_t)bp);
	}
	/*
	 * On last close of a memory filesystem
	 * we must invalidate any in core blocks, so that
	 * we can, free up its vnode.
	 */
	vflushbuf(vp, 0);
	if (vinvalbuf(vp, 1))
		return (0);
	/*
	 * There should be no way to have any more uses of this
	 * vnode, so if we find any other uses, it is a panic.
	 */
	if (vp->v_usecount > 1)
		printf("mfs_close: ref count %d > 1\n", vp->v_usecount);
	if (vp->v_usecount > 1 || mfsp->mfs_buflist)
		panic("mfs_close");
	/*
	 * Send a request to the filesystem server to exit.
	 */
	mfsp->mfs_buflist = (struct buf *)(-1);
	wakeup((caddr_t)vp);
	return (0);
}

/*
 * Memory filesystem inactive routine
 */
/* ARGSUSED */
mfs_inactive(vp, p)
	struct vnode *vp;
	struct proc *p;
{

	if (VTOMFS(vp)->mfs_buflist != (struct buf *)(-1))
		panic("mfs_inactive: not inactive");
	return (0);
}

/*
 * Print out the contents of an mfsnode.
 */
mfs_print(vp)
	struct vnode *vp;
{
	register struct mfsnode *mfsp = VTOMFS(vp);

	printf("tag VT_MFS, pid %d, base %d, size %d\n", mfsp->mfs_pid,
		mfsp->mfs_baseoff, mfsp->mfs_size);
}

/*
 * Block device bad operation
 */
mfs_badop()
{

	panic("mfs_badop called\n");
	/* NOTREACHED */
}

/*
 * Memory based filesystem initialization.
 */
mfs_init()
{

#if !defined(hp300) && !defined(i386)
	rminit(mfsmap, (long)MFS_MAPREG, (long)1, "mfs mapreg", MFS_MAPSIZE);
#endif
}
