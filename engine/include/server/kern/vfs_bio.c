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
 * $Log:	vfs_bio.c,v $
 * Revision 2.3  92/07/08  16:19:50  mrt
 * 	Added fourth arg, TRUE, to vm_allocate call in getnewbuf.
 * 	[92/07/02            mrt]
 * 
 * Revision 2.2  92/06/25  17:25:40  mrt
 * 	Clear b_resid on release.  No one cares once released.
 * 	[92/06/25            rwd]
 * 	Set b_rcred before VOP_STRATEGY calls. [from Jolitz]
 * 
 * Revision 2.1  92/04/21  17:12:36  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *	from: @(#)vfs_bio.c	7.40 (Berkeley) 5/8/91
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/trace.h>
#include <sys/resourcevar.h>
#include <sys/synch.h>

int debug_bio = 0;
int freebufspace = 0;
int allocbufspace = 0;
extern int bufpages;

extern kern_return_t	bio_read_reply();
extern kern_return_t	bio_write_reply();

struct buf *allocbuf();

/*
 * Initialize buffers and hash links for buffers.
 */
bufinit()
{
    struct bufhd *bh;
    struct buf *bp;

    freebufspace = bufpages * NBPG;

    /* first, make a null hash table */
    for(bh = bufhash; bh < bufhash + BUFHSZ; bh++) {
	bh->b_flags = 0;
	bh->b_forw = (struct buf *)bh;
	bh->b_back = (struct buf *)bh;
    }

    /* next, make a null set of free lists */
    for(bp = bfreelist; bp < bfreelist + BQUEUES; bp++) {
	bp->b_flags = 0;
	bp->av_forw = bp;
	bp->av_back = bp;
	bp->b_forw = bp;
	bp->b_back = bp;

    }

    /* finally, initialize each buffer header and stick on empty q */
    for(bp = buf; bp < buf + nbuf ; bp++) {
	bp->b_flags = B_HEAD | B_INVAL;	/* we're just an empty header */
	bp->b_dev = NODEV;
	bp->b_vp = 0;
	/* allocate one reply port per buffer (the Accent way...) */
	bp->b_reply_port = mach_reply_port();
	reply_hash_enter(bp->b_reply_port,
			 (char *)bp,
			 bio_read_reply,
			 bio_write_reply);
	binstailfree(bp, bfreelist + BQ_EMPTY);
	binshash(bp, bfreelist + BQ_EMPTY);
    }
}


/*
 * Find the block in the buffer pool.
 * If the buffer is not present, allocate a new buffer and load
 * its contents according to the filesystem fill routine.
 */
bread(vp, blkno, size, cred, bpp)
	struct vnode *vp;
	daddr_t blkno;
	int size;
	struct ucred *cred;
	struct buf **bpp;
{
	struct buf *bp;
	int rv = 0;

	bp = getblk (vp, blkno, size);

	/* if not found in cache, do some I/O */
	if ((bp->b_flags & B_CACHE) == 0 || (bp->b_flags & B_INVAL) != 0) {
		bp->b_flags |= B_READ;
		bp->b_flags &= ~(B_DONE|B_ERROR|B_INVAL);
		bp->b_rcred = cred;
		VOP_STRATEGY(bp);
		rv = biowait (bp);
	}
	*bpp = bp;
	
	return (rv);
}

/*
 * Operates like bread, but also starts I/O on the specified
 * read-ahead block. [See page 55 of Bach's Book]
 */
breada(vp, blkno, size, rablkno, rabsize, cred, bpp)
	struct vnode *vp;
	daddr_t blkno; int size;
	daddr_t rablkno; int rabsize;
	struct ucred *cred;
	struct buf **bpp;
{
	struct buf *bp, *rabp;
	int rv = 0, needwait = 0;

	bp = getblk (vp, blkno, size);

	/* if not found in cache, do some I/O */
	if ((bp->b_flags & B_CACHE) == 0 || (bp->b_flags & B_INVAL) != 0) {
		bp->b_flags |= B_READ;
		bp->b_flags &= ~(B_DONE|B_ERROR|B_INVAL);
		bp->b_rcred = cred;
		VOP_STRATEGY(bp);
		needwait++;
	}
	
	rabp = getblk (vp, rablkno, rabsize);

	/* if not found in cache, do some I/O (overlapped with first) */
	if ((rabp->b_flags & B_CACHE) == 0 || (rabp->b_flags & B_INVAL) != 0) {
		rabp->b_flags |= B_READ | B_ASYNC;
		rabp->b_flags &= ~(B_DONE|B_ERROR|B_INVAL);
		bp->b_rcred = cred;
		VOP_STRATEGY(rabp);
	} else
		brelse(rabp);
	
	/* wait for original I/O */
	if (needwait)
		rv = biowait (bp);

	*bpp = bp;
	return (rv);
}

/*
 * Synchronous write.
 * Release buffer on completion.
 */
bwrite(bp)
	register struct buf *bp;
{
	int rv;

	if(bp->b_flags & B_INVAL) {
		brelse(bp);
		return (0);
	} else {
		int wasdelayed;

		if(!(bp->b_flags & B_BUSY))panic("bwrite: not busy");
		wasdelayed = bp->b_flags & B_DELWRI;
		bp->b_flags &= ~(B_READ|B_DONE|B_ERROR|B_ASYNC|B_DELWRI);
		if(wasdelayed) reassignbuf(bp, bp->b_vp);
		bp->b_flags |= B_DIRTY;
		VOP_STRATEGY(bp);
		rv = biowait(bp);
		if (!rv)
			bp->b_flags &= ~B_DIRTY;
		brelse(bp);
		return (rv);
	}
}

/*
 * Delayed write.
 *
 * The buffer is marked dirty, but is not queued for I/O.
 * This routine should be used when the buffer is expected
 * to be modified again soon, typically a small write that
 * partially fills a buffer.
 *
 * NB: magnetic tapes cannot be delayed; they must be
 * written in the order that the writes are requested.
 */
void bdwrite(bp)
	register struct buf *bp;
{

	if(!(bp->b_flags & B_BUSY))panic("bdwrite: not busy");
	if(bp->b_flags & B_INVAL) {
		brelse(bp);
	}
	if(bp->b_flags & B_TAPE) {
		bwrite(bp);
		return;
	}
	bp->b_flags &= ~(B_READ|B_DONE);
	bp->b_flags |= B_DIRTY|B_DELWRI;
	reassignbuf(bp, bp->b_vp);
	brelse(bp);
	return;
}

/*
 * Asynchronous write.
 * Start I/O on a buffer, but do not wait for it to complete.
 * The buffer is released when the I/O completes.
 */
bawrite(bp)
	register struct buf *bp;
{

	if(!(bp->b_flags & B_BUSY))panic("bawrite: not busy");
	if(bp->b_flags & B_INVAL)
		brelse(bp);
	else {
		int wasdelayed;

		wasdelayed = bp->b_flags & B_DELWRI;
		bp->b_flags &= ~(B_READ|B_DONE|B_ERROR|B_DELWRI);
		if(wasdelayed) reassignbuf(bp, bp->b_vp);

		bp->b_flags |= B_DIRTY | B_ASYNC;
		VOP_STRATEGY(bp);
	}
}

/*
 * Release a buffer.
 * Even if the buffer is dirty, no I/O is started.
 */
brelse(bp)
	register struct buf *bp;
{
	int x;

	/* anyone need a "free" block? */
	x=splbio();
	if ((bfreelist + BQ_AGE)->b_flags & B_WANTED) {
		(bfreelist + BQ_AGE) ->b_flags &= ~B_WANTED;
		wakeup(bfreelist);
	}
	/* anyone need this very block? */
	if (bp->b_flags & B_WANTED) {
		bp->b_flags &= ~B_WANTED;
		wakeup(bp);
	}

	if (bp->b_flags & (B_INVAL|B_ERROR)) {
		bp->b_flags |= B_INVAL;
		bp->b_flags &= ~(B_DELWRI|B_CACHE);
		if(bp->b_vp)
			brelvp(bp);
	}

	/* enqueue */
	/* just an empty buffer head ... */
	/*if(bp->b_flags & B_HEAD)
		binsheadfree(bp, bfreelist + BQ_EMPTY)*/
	/* buffers with junk contents */
	/*else*/ if(bp->b_flags & (B_ERROR|B_INVAL|B_NOCACHE))
		binsheadfree(bp, bfreelist + BQ_AGE)
	/* buffers with stale but valid contents */
	else if(bp->b_flags & B_AGE)
		binstailfree(bp, bfreelist + BQ_AGE)
	/* buffers with valid and quite potentially reuseable contents */
	else
		binstailfree(bp, bfreelist + BQ_LRU)

	/* unlock */
	bp->b_flags &= ~B_BUSY;
	bp->b_resid = 0;
	splx(x);

	return;
}

/*
 * Find a buffer which is available for use.
 * If free memory for buffer space and an empty header from the empty list,
 * use that. Otherwise, select something from a free list.
 * Preference is to AGE list, then LRU list.
 */
struct buf *
getnewbuf(sz)
    int sz;
{
	struct buf *bp;
	int x;

	x = splbio();
start:
	/* can we constitute a new buffer? */
	if (freebufspace > sz
		&& bfreelist[BQ_EMPTY].av_forw != (struct buf *)bfreelist+BQ_EMPTY) {
		caddr_t addr;
		kern_return_t kr;

		sz = round_page(sz);

		if ((kr = vm_allocate(mach_task_self(), (vm_address_t *)  &addr, (vm_size_t) sz, TRUE))
		    != KERN_SUCCESS) goto tryfree;
		freebufspace -= sz;
		allocbufspace += sz;

		bp = bfreelist[BQ_EMPTY].av_forw;
		if (debug_bio) printf("   allocate bp: %x addr: %x size: %x\n",bp, addr, sz);
		bp->b_flags = B_BUSY | B_INVAL;
		bremfree(bp);
		bp->b_un.b_addr = (caddr_t) addr;
		bp->b_bufsize = round_page(sz);
		goto fillin;
	}

tryfree:
	if (bfreelist[BQ_AGE].av_forw != (struct buf *)bfreelist+BQ_AGE) {
		bp = bfreelist[BQ_AGE].av_forw;
		bremfree(bp);
	} else if (bfreelist[BQ_LRU].av_forw != (struct buf *)bfreelist+BQ_LRU) {
		bp = bfreelist[BQ_LRU].av_forw;
		bremfree(bp);
	} else	{
		/* wait for a free buffer of any kind */
		(bfreelist + BQ_AGE)->b_flags |= B_WANTED;
		sleep(bfreelist, PRIBIO);
		splx(x);
		return (0);
	}

	/* if we are a delayed write, convert to an async write! */
	if (bp->b_flags & B_DELWRI) {
		/*bp->b_flags &= ~B_DELWRI;*/
		bp->b_flags |= B_BUSY;
		bawrite (bp);
		goto start;
	}

	/*if (bp->b_flags & (B_INVAL|B_ERROR) == 0) {
		bremhash(bp);
	}*/

	if(bp->b_vp)
		brelvp(bp);

	/* we are not free, nor do we contain interesting data */
	bp->b_flags = B_BUSY;
fillin:
	bremhash(bp);
	splx(x);
	bp->b_dev = NODEV;
	bp->b_vp = NULL;
	bp->b_blkno = bp->b_lblkno = 0;
	bp->b_iodone = 0;
	bp->b_error = 0;
	bp->b_resid = 0;
	bp->b_wcred = bp->b_rcred = NOCRED;
	if (bp->b_bufsize != round_page(sz)) allocbuf(bp, sz);
	bp->b_bcount = sz;
	bp->b_dirtyoff = bp->b_dirtyend = 0;
	return (bp);
}

/*
 * Check to see if a block is currently memory resident.
 */
struct buf *incore(vp, blkno)
	struct vnode *vp;
	daddr_t blkno;
{
	struct buf *bh;
	struct buf *bp;

	bh = BUFHASH(vp, blkno);

	/* Search hash chain */
	bp = bh->b_forw;
	while (bp != (struct buf *) bh) {
		/* hit */
		if (bp->b_lblkno == blkno && bp->b_vp == vp
			&& (bp->b_flags & B_INVAL) == 0)
			return (bp);
		bp = bp->b_forw;
	}
	
	return(0);
}

/*
 * Get a block of requested size that is associated with
 * a given vnode and block offset. If it is found in the
 * block cache, mark it as having been found, make it busy
 * and return it. Otherwise, return an empty block of the
 * correct size. It is up to the caller to insure that the
 * cached blocks be of the correct size.
 */
struct buf *
getblk(vp, blkno, size)
	register struct vnode *vp;
	daddr_t blkno;
	int size;
{
	struct buf *bp, *bh;
	int x;

	for (;;) {
		if (bp = incore(vp, blkno)) {
			x = splbio();
			if (bp->b_flags & B_BUSY) {
				bp->b_flags |= B_WANTED;
				sleep (bp, PRIBIO);
				splx(x);
				continue;
			}
			bp->b_flags |= B_BUSY | B_CACHE;
			bremfree(bp);
			if (size > bp->b_bufsize)
				panic("now what do we do?");
			/* if (bp->b_bufsize != size) allocbuf(bp, size); */
		} else {

			if((bp = getnewbuf(size)) == 0) continue;
			bp->b_blkno = bp->b_lblkno = blkno;
			bgetvp(vp, bp);
			x = splbio();
			bh = BUFHASH(vp, blkno);
			binshash(bp, bh);
			bp->b_flags = B_BUSY;
		}
		splx(x);
		return (bp);
	}
}

/*
 * Get an empty, disassociated buffer of given size.
 */
struct buf *
geteblk(size)
	int size;
{
	struct buf *bp;
	int x;

	while ((bp = getnewbuf(size)) == 0)
		;
	x = splbio();
	binshash(bp, bfreelist + BQ_AGE);
	splx(x);

	return (bp);
}

/*
 * Exchange a buffer's underlying buffer storage for one of different
 * size, taking care to maintain contents appropriately. When buffer
 * increases in size, caller is responsible for filling out additional
 * contents. When buffer shrinks in size, data is lost, so caller must
 * first return it to backing store before shrinking the buffer, as
 * no implied I/O will be done.
 *
 * Expanded buffer is returned as value.
 */
struct buf *
allocbuf(bp, size)
	register struct buf *bp;
	int size;
{
	vm_size_t	current_size, desired_size;
	vm_offset_t	new_start;
	kern_return_t	kr;

	current_size = bp->b_bufsize;
	desired_size = round_page(size);

	if (current_size < desired_size) {
	    /*
	     * Buffer is growing.
	     * If buffer already has data, allocate new area and copy
	     * old data to it.
	     */
	    kr = vm_allocate(mach_task_self(),
			     &new_start,
			     desired_size,
			     TRUE);
	    if (kr != KERN_SUCCESS)
		panic("allocbuf: allocate",kr);
	    if (debug_bio) printf(" reallocate bp: %x addr: %x size: %x\n",bp, new_start, desired_size);
	    if (current_size) {
		bcopy(bp->b_un.b_addr,
		      (caddr_t) new_start,
		      bp->b_bufsize);
		kr = vm_deallocate(mach_task_self(),
				   (vm_offset_t)bp->b_un.b_addr,
				   current_size);
		if (kr != KERN_SUCCESS)
		    panic("allocbuf: deallocate",kr);
		if (debug_bio) printf(" deallocate bp: %x addr: %x size: %x\n",bp, bp->b_un.b_addr,bp->b_bufsize);
	    }
	    bp->b_un.b_addr = (char *)new_start;
	    bp->b_bufsize = desired_size;
	}
	/* adjust buffer cache's idea of memory allocated to buffer contents */
	freebufspace -= desired_size - bp->b_bufsize;
	allocbufspace += desired_size - bp->b_bufsize;

	bp->b_bcount = size;
	return (bp);
}

/*
 * Patiently await operations to complete on this buffer.
 * When they do, extract error value and return it.
 * Extract and return any errors associated with the I/O.
 * If an invalid block, force it off the lookup hash chains.
 */
biowait(bp)
	register struct buf *bp;
{
	int x;

	x = splbio();
	while ((bp->b_flags & B_DONE) == 0)
		sleep((caddr_t)bp, PRIBIO);
	if((bp->b_flags & B_ERROR) || bp->b_error) {
		if ((bp->b_flags & B_INVAL) == 0) {
			bp->b_flags |= B_INVAL;
			bremhash(bp);
			binshash(bp, bfreelist + BQ_AGE);
		}
		if (!bp->b_error)
			bp->b_error = EIO;
		else
			bp->b_flags |= B_ERROR;
		splx(x);
		return (bp->b_error);
	} else {
		splx(x);
		return (0);
	}
}

/*
 * Finish up operations on a buffer, calling an optional
 * function (if requested), and releasing the buffer if
 * marked asynchronous. Then mark this buffer done so that
 * others biowait()'ing for it will notice when they are
 * woken up from sleep().
 */
biodone(bp)
	register struct buf *bp;
{
	int x;

	x = splbio();
	if (bp->b_flags & B_CALL) (*bp->b_iodone)(bp);
	bp->b_flags &=  ~B_CALL;
	if (bp->b_flags & B_ASYNC) brelse(bp);
	bp->b_flags &=  ~B_ASYNC;
	bp->b_flags |= B_DONE;
	wakeup(bp);
	splx(x);
}
