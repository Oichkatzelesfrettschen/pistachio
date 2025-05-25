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
 * $Log:	sys_generic.c,v $
 * Revision 2.2  92/07/08  16:19:35  mrt
 * 	Added the process pointer argument to the call to fo_select
 * 	and added the process pointer parameter to seltrue to match the
 * 	prototype declaration in sys/systm.h. Added new style parameter
 * 	declatation for seltrue so it would compile with an ANSI compiler
 * 	[92/06/26            mrt]
 * 
 * Revision 2.1  92/04/21  17:12:20  rwd
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
 *	@(#)sys_generic.c	7.30 (Berkeley) 5/30/91
 */

#include <ktrace.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/socketvar.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/stat.h>
#include <sys/malloc.h>
#include <sys/synch.h>
#if KTRACE
#include <sys/ktrace.h>
#endif

/*
 * Read system call.
 */
/* ARGSUSED */
read(p, uap, retval)
	struct proc *p;
	register struct args {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;
	int *retval;
{
	register struct file *fp;
	register struct filedesc *fdp = p->p_fd;
	struct uio auio;
	struct iovec aiov;
	long cnt, error = 0;
#if KTRACE
	struct iovec ktriov;
#endif

	if (((unsigned)uap->fdes) >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[uap->fdes]) == NULL ||
	    (fp->f_flag & FREAD) == 0)
		return (EBADF);
	aiov.iov_base = (caddr_t)uap->cbuf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_resid = uap->count;
	auio.uio_rw = UIO_READ;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_procp = p;
#if KTRACE
	/*
	 * if tracing, save a copy of iovec
	 */
	if (KTRPOINT(p, KTR_GENIO))
		ktriov = aiov;
#endif
	cnt = uap->count;
	if (error = (*fp->f_ops->fo_read)(fp, &auio, fp->f_cred))
		if (auio.uio_resid != cnt && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
	cnt -= auio.uio_resid;
#if KTRACE
	if (KTRPOINT(p, KTR_GENIO) && error == 0)
		ktrgenio(p->p_tracep, uap->fdes, UIO_READ, &ktriov, cnt, error);
#endif
	*retval = cnt;
	return (error);
}

/*
 * Scatter read system call.
 */
/* ARGSUSED */
readv(p, uap, retval)
	struct proc *p;
	register struct args {
		int	fdes;
		struct	iovec *iovp;
		unsigned iovcnt;
	} *uap;
	int *retval;
{
	register struct file *fp;
	register struct filedesc *fdp = p->p_fd;
	struct uio auio;
	register struct iovec *iov;
	struct iovec *saveiov;
	struct iovec aiov[UIO_SMALLIOV];
	long i, cnt, error = 0;
	unsigned iovlen;
#if KTRACE
	struct iovec *ktriov = NULL;
#endif

	if (((unsigned)uap->fdes) >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[uap->fdes]) == NULL ||
	    (fp->f_flag & FREAD) == 0)
		return (EBADF);
	/* note: can't use iovlen until iovcnt is validated */
	iovlen = uap->iovcnt * sizeof (struct iovec);
	if (uap->iovcnt > UIO_SMALLIOV) {
		if (uap->iovcnt > UIO_MAXIOV)
			return (EINVAL);
		MALLOC(iov, struct iovec *, iovlen, M_IOV, M_WAITOK);
		saveiov = iov;
	} else
		iov = aiov;
	auio.uio_iov = iov;
	auio.uio_iovcnt = uap->iovcnt;
	auio.uio_rw = UIO_READ;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_procp = p;
	if (error = copyin((caddr_t)uap->iovp, (caddr_t)iov, iovlen))
		goto done;
	auio.uio_resid = 0;
	for (i = 0; i < uap->iovcnt; i++) {
		if (iov->iov_len < 0) {
			error = EINVAL;
			goto done;
		}
		auio.uio_resid += iov->iov_len;
		if (auio.uio_resid < 0) {
			error = EINVAL;
			goto done;
		}
		iov++;
	}
#if KTRACE
	/*
	 * if tracing, save a copy of iovec
	 */
	if (KTRPOINT(p, KTR_GENIO))  {
		MALLOC(ktriov, struct iovec *, iovlen, M_TEMP, M_WAITOK);
		bcopy((caddr_t)auio.uio_iov, (caddr_t)ktriov, iovlen);
	}
#endif
	cnt = auio.uio_resid;
	if (error = (*fp->f_ops->fo_read)(fp, &auio, fp->f_cred))
		if (auio.uio_resid != cnt && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
	cnt -= auio.uio_resid;
#if KTRACE
	if (ktriov != NULL) {
		if (error == 0)
			ktrgenio(p->p_tracep, uap->fdes, UIO_READ, ktriov,
			    cnt, error);
		FREE(ktriov, M_TEMP);
	}
#endif
	*retval = cnt;
done:
	if (uap->iovcnt > UIO_SMALLIOV)
		FREE(saveiov, M_IOV);
	return (error);
}

/*
 * Write system call
 */
write(p, uap, retval)
	struct proc *p;
	register struct args {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;
	int *retval;
{
	register struct file *fp;
	register struct filedesc *fdp = p->p_fd;
	struct uio auio;
	struct iovec aiov;
	long cnt, error = 0;
#if KTRACE
	struct iovec ktriov;
#endif

	if (((unsigned)uap->fdes) >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[uap->fdes]) == NULL ||
	    (fp->f_flag & FWRITE) == 0)
		return (EBADF);
	aiov.iov_base = (caddr_t)uap->cbuf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_resid = uap->count;
	auio.uio_rw = UIO_WRITE;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_procp = p;
#if KTRACE
	/*
	 * if tracing, save a copy of iovec
	 */
	if (KTRPOINT(p, KTR_GENIO))
		ktriov = aiov;
#endif
	cnt = uap->count;
	if (error = (*fp->f_ops->fo_write)(fp, &auio, fp->f_cred)) {
		if (auio.uio_resid != cnt && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
		if (error == EPIPE)
			psignal(p, SIGPIPE);
	}
	cnt -= auio.uio_resid;
#if KTRACE
	if (KTRPOINT(p, KTR_GENIO) && error == 0)
		ktrgenio(p->p_tracep, uap->fdes, UIO_WRITE,
		    &ktriov, cnt, error);
#endif
	*retval = cnt;
	return (error);
}

/*
 * Gather write system call
 */
writev(p, uap, retval)
	struct proc *p;
	register struct args {
		int	fdes;
		struct	iovec *iovp;
		unsigned iovcnt;
	} *uap;
	int *retval;
{
	register struct file *fp;
	register struct filedesc *fdp = p->p_fd;
	struct uio auio;
	register struct iovec *iov;
	struct iovec *saveiov;
	struct iovec aiov[UIO_SMALLIOV];
	long i, cnt, error = 0;
	unsigned iovlen;
#if KTRACE
	struct iovec *ktriov = NULL;
#endif

	if (((unsigned)uap->fdes) >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[uap->fdes]) == NULL ||
	    (fp->f_flag & FWRITE) == 0)
		return (EBADF);
	/* note: can't use iovlen until iovcnt is validated */
	iovlen = uap->iovcnt * sizeof (struct iovec);
	if (uap->iovcnt > UIO_SMALLIOV) {
		if (uap->iovcnt > UIO_MAXIOV)
			return (EINVAL);
		MALLOC(iov, struct iovec *, iovlen, M_IOV, M_WAITOK);
		saveiov = iov;
	} else
		iov = aiov;
	auio.uio_iov = iov;
	auio.uio_iovcnt = uap->iovcnt;
	auio.uio_rw = UIO_WRITE;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_procp = p;
	if (error = copyin((caddr_t)uap->iovp, (caddr_t)iov, iovlen))
		goto done;
	auio.uio_resid = 0;
	for (i = 0; i < uap->iovcnt; i++) {
		if (iov->iov_len < 0) {
			error = EINVAL;
			goto done;
		}
		auio.uio_resid += iov->iov_len;
		if (auio.uio_resid < 0) {
			error = EINVAL;
			goto done;
		}
		iov++;
	}
#if KTRACE
	/*
	 * if tracing, save a copy of iovec
	 */
	if (KTRPOINT(p, KTR_GENIO))  {
		MALLOC(ktriov, struct iovec *, iovlen, M_TEMP, M_WAITOK);
		bcopy((caddr_t)auio.uio_iov, (caddr_t)ktriov, iovlen);
	}
#endif
	cnt = auio.uio_resid;
	if (error = (*fp->f_ops->fo_write)(fp, &auio, fp->f_cred)) {
		if (auio.uio_resid != cnt && (error == ERESTART ||
		    error == EINTR || error == EWOULDBLOCK))
			error = 0;
		if (error == EPIPE)
			psignal(p, SIGPIPE);
	}
	cnt -= auio.uio_resid;
#if KTRACE
	if (ktriov != NULL) {
		if (error == 0)
			ktrgenio(p->p_tracep, uap->fdes, UIO_WRITE,
				ktriov, cnt, error);
		FREE(ktriov, M_TEMP);
	}
#endif
	*retval = cnt;
done:
	if (uap->iovcnt > UIO_SMALLIOV)
		FREE(saveiov, M_IOV);
	return (error);
}

/*
 * Ioctl system call
 */
/* ARGSUSED */
ioctl(p, uap, retval)
	struct proc *p;
	register struct args {
		int	fdes;
		int	cmd;
		caddr_t	cmarg;
	} *uap;
	int *retval;
{
	register struct file *fp;
	register struct filedesc *fdp = p->p_fd;
	register int com, error;
	register u_int size;
	caddr_t memp = 0;
#define STK_PARAMS	128
	char stkbuf[STK_PARAMS];
	caddr_t data = stkbuf;
	int tmp;

	if ((unsigned)uap->fdes >= fdp->fd_nfiles ||
	    (fp = fdp->fd_ofiles[uap->fdes]) == NULL)
		return (EBADF);
	if ((fp->f_flag & (FREAD|FWRITE)) == 0)
		return (EBADF);
	com = uap->cmd;

	if (com == FIOCLEX) {
		fdp->fd_ofileflags[uap->fdes] |= UF_EXCLOSE;
		return (0);
	}
	if (com == FIONCLEX) {
		fdp->fd_ofileflags[uap->fdes] &= ~UF_EXCLOSE;
		return (0);
	}

	/*
	 * Interpret high order word to find
	 * amount of data to be copied to/from the
	 * user's address space.
	 */
	size = IOCPARM_LEN(com);
	if (size > IOCPARM_MAX)
		return (ENOTTY);
	if (size > sizeof (stkbuf)) {
	        MALLOC(memp, caddr_t, size, M_IOCTLOPS, M_WAITOK);
		data = memp;
	}
	if (com&IOC_IN) {
		if (size) {
			error = copyin(uap->cmarg, data, (u_int)size);
			if (error) {
				if (memp)
					FREE(memp, M_IOCTLOPS);
				return (error);
			}
		} else
			*(caddr_t *)data = uap->cmarg;
	} else if ((com&IOC_OUT) && size)
		/*
		 * Zero the buffer so the user always
		 * gets back something deterministic.
		 */
		bzero(data, size);
	else if (com&IOC_VOID)
		*(caddr_t *)data = uap->cmarg;

	switch (com) {

	case FIONBIO:
		if (tmp = *(int *)data)
			fp->f_flag |= FNONBLOCK;
		else
			fp->f_flag &= ~FNONBLOCK;
		error = (*fp->f_ops->fo_ioctl)(fp, FIONBIO, (caddr_t)&tmp, p);
		break;

	case FIOASYNC:
		if (tmp = *(int *)data)
			fp->f_flag |= FASYNC;
		else
			fp->f_flag &= ~FASYNC;
		error = (*fp->f_ops->fo_ioctl)(fp, FIOASYNC, (caddr_t)&tmp, p);
		break;

	case FIOSETOWN:
		tmp = *(int *)data;
		if (fp->f_type == DTYPE_SOCKET) {
			((struct socket *)fp->f_data)->so_pgid = tmp;
			error = 0;
			break;
		}
		if (tmp <= 0) {
			tmp = -tmp;
		} else {
			struct proc *p1 = pfind(tmp);
			if (p1 == 0) {
				error = ESRCH;
				break;
			}
			tmp = p1->p_pgrp->pg_id;
		}
		error = (*fp->f_ops->fo_ioctl)
			(fp, (int)TIOCSPGRP, (caddr_t)&tmp, p);
		break;

	case FIOGETOWN:
		if (fp->f_type == DTYPE_SOCKET) {
			error = 0;
			*(int *)data = ((struct socket *)fp->f_data)->so_pgid;
			break;
		}
		error = (*fp->f_ops->fo_ioctl)(fp, (int)TIOCGPGRP, data, p);
		*(int *)data = -*(int *)data;
		break;

	default:
		error = (*fp->f_ops->fo_ioctl)(fp, com, data, p);
		/*
		 * Copy any data to user, size was
		 * already set and checked above.
		 */
		if (error == 0 && (com&IOC_OUT) && size)
			error = copyout(data, uap->cmarg, (u_int)size);
		break;
	}
	if (memp)
		FREE(memp, M_IOCTLOPS);
	return (error);
}


struct selbuf {
	int	s_state;	/* state of select buffer: */
#define	SB_CLEAR	0		/* wait condition cleared */
#define	SB_WILLWAIT	1		/* will wait if not cleared */
#define	SB_WAITING	2		/* waiting - wake up */
	int	s_count;	/* number of items in select list */
	struct selitem {
		struct	selitem *si_next;	/* next selbuf in item chain */
		struct	selitem	**si_prev;	/* back pointer */
		struct	selbuf	*si_selbuf;	/* 'thread' waiting */
	} s_item[NOFILE];
};

struct mutex select_lock = MUTEX_INITIALIZER;

/*
 * Select system call.
 */
select(p, uap, retval)
	register struct proc *p;
	register struct args {
		int	nd;
		fd_set	*in, *ou, *ex;
		struct	timeval *tv;
	} *uap;
	int *retval;
{
	fd_set obits[3];
	struct timeval atv;
	int error = 0;

	if (uap->nd > p->p_fd->fd_nfiles)
		uap->nd = p->p_fd->fd_nfiles;	/* forgiving; slightly wrong */

	/*
	 *	Calculate the absolute time until which we will block.
	 */

	if (uap->tv) {
	    struct timeval time;

	    bcopy((caddr_t)uap->tv, (caddr_t)&atv, sizeof (atv));

	    if (itimerfix(&atv)) {
		    error = EINVAL;
		    goto done;
	    }

	    get_time(&time);
	    timevaladd(&atv, &time);
	}

	/*
	 *	We use selscan twice each time around the loop.
	 *	The first time, we make a "fast" check for a true condition.
	 *	selenter does nothing and we don't need to use unselect.
	 *	The second time, we actually put ourself on device chains.
	 *	We only need to zero obits once because if selscan
	 *	turns on a bit it returns non-zero and we stop.
	 */

	bzero((caddr_t)obits, sizeof(obits));
	while (TRUE) {
	    struct selbuf selbuf;
	    int timeout_hzto;

	    /*
	     *	We make a first pass to check for conditions
	     *	which are already true.  Clearing uu_sb
	     *	will keep selenter from doing anything.
	     */

	    p->p_sb = 0;

	    error = selscan(p, uap->in, uap->ou, uap->ex,
				      obits, uap->nd, retval);
	    if (error || *retval)
		break;

	    /*
	     *	Calculate the number of ticks until atv.
	     *	Zero means we shouldn't block, because
	     *	the timeout interval as already expired.
	     */

	    if (uap->tv) {
		timeout_hzto = hzto(&atv);
		if (timeout_hzto == 0)
		    break;	/* r_val1 and u_error are zero */
	    } else {
		/* sleep forever below */

		timeout_hzto = 0;
	    }

	    /*
	     *	Now we get serious about blocking.
	     *	selenter will put us on device chains.
	     *	We don't need select_lock here because
	     *	no other thread can get at selbuf yet.
	     */

	    selbuf.s_state = SB_WILLWAIT;
	    selbuf.s_count = 0;
	    p->p_sb = &selbuf;

	    error = selscan(p, uap->in, uap->ou, uap->ex,
				      obits, uap->nd, retval);
	    if (error || *retval) {
		mutex_lock(&select_lock);
		unselect(&selbuf);
		mutex_unlock(&select_lock);
		break;
	    }

	    /*
	     *	If the state is already SB_CLEAR, then a selwakeup
	     *	slipped in after the selscan.
	     */

	    mutex_lock(&select_lock);
	    if (selbuf.s_state == SB_CLEAR) {
		unselect(&selbuf);
		mutex_unlock(&select_lock);
		continue;
	    }

	    selbuf.s_state = SB_WAITING;
	    tsleep_enter((caddr_t)&selbuf, PSOCK|PCATCH, "select", timeout_hzto);
	    mutex_unlock(&select_lock);
	    error = tsleep_main((caddr_t)&selbuf, PSOCK|PCATCH, timeout_hzto);

	    mutex_lock(&select_lock);
	    unselect(&selbuf);
	    mutex_unlock(&select_lock);

	    if (error == ERESTART) {
		error = EINTR;
		break;
	    }

	    if (error == EWOULDBLOCK)
		error = 0;
	}

    done:

	p->p_sb = 0;

#define	putbits(name, x) 						\
	if (uap->name) {						\
		bcopy((caddr_t)&obits[x], (caddr_t)uap->name,		\
		    (unsigned)(sizeof(fd_set)));			\
	}

	if (error == 0) {
	    putbits(in, 0);
	    putbits(ou, 1);
	    putbits(ex, 2);
	}
#undef putbits
	return (error);
}

selscan(p, in, ou, ex, obits, nfd, retval)
        struct proc *p;
        fd_set *in, *ou, *ex, *obits;
        int *retval;

{
	register int which, i, j;
	register fd_mask bits;
	register fd_set *cbits;
	int flag;
	struct file *fp;
	int n = 0;
	int error = 0;

	for (which = 0; which < 3; which++) {
		switch (which) {

		case 0:
			flag = FREAD;
			cbits = in;
			break;
		case 1:
			flag = FWRITE;
			cbits = ou;
			break;
		case 2:
			flag = 0;
			cbits = ex;
			break;
		}
		if (cbits != 0) {
		  for (i = 0; i < nfd; i += NFDBITS) {
			bits = cbits->fds_bits[i/NFDBITS];
			while ((j = ffs(bits)) && i + --j < nfd) {
				bits &= ~(1 << j);
				fp = p->p_fd->fd_ofiles[i + j];
				if (fp == NULL) {
					error = EBADF;
					break;
				}
				if ((*fp->f_ops->fo_select)(fp, flag, p)) {
					FD_SET(i + j, &obits[which]);
					n++;
				}
			}
		  }
		}
	}
	*retval = n;
	return (error);
}

/*ARGSUSED*/
int
#ifdef	__STDC__
seltrue(dev_t dev, int flag, struct proc *p)
#else	/* __STDC__ */
seltrue(dev, flag, p)
	dev_t dev;
	int flag;
	struct proc *p;
#endif	/* __STDC__ */
{
	return (1);
}

/*
 * Add a select event to the current select buffer.
 * Chain all select buffers that are waiting for
 * the same event.
 */
selenter(p, hdr)
	caddr_t	hdr;
        struct proc *p;
{
	register struct selbuf *sb = p->p_sb;

	if (sb) {
	    register struct selitem **sip = (struct selitem **)hdr;
	    register struct selitem *si;

	    mutex_lock(&select_lock);

	    /* initialize this select item */
	    si = &sb->s_item[sb->s_count++];
	    si->si_selbuf = sb;

	    /* and add it to the device chain */
	    if (si->si_next = *sip)
		si->si_next->si_prev = &si->si_next;
	    *(si->si_prev = sip) = si;

	    mutex_unlock(&select_lock);
	}
}

unselect(sb)
	register struct selbuf *sb;
{
	int i;

	/*
	 *	We unchain the select items.
	 *	This assumes select_lock is held.
	 */

	for (i = 0; i < sb->s_count; i++) {
	    register struct selitem *si = &sb->s_item[i];

	    /*
	     *	We remove this item from its device chain.
	     *	If selwakeup already got to it, then this is a nop.
	     */

	    *si->si_prev = si->si_next;
	}
}

/*
 * Wakeup all threads waiting on the same select event.
 * Do not clear the other select events that each thread
 * is waiting on; thread will do that itself.
 */
selwakeup(hdr)
	caddr_t	hdr;
{
	register struct selitem **sip;
	register struct selitem *si;

	mutex_lock(&select_lock);

	for (sip = (struct selitem **) hdr;
	     (si = *sip) != 0;
	     sip = &si->si_next) {
	    register struct selbuf *sb;
	    int old_state;

	    /* for when unselect tries to dequeue this item */
	    si->si_prev = &si->si_next;

	    /* check for wakeup */
	    sb = si->si_selbuf;
	    old_state = sb->s_state;
	    sb->s_state = SB_CLEAR;
	    if (old_state == SB_WAITING)
		wakeup((caddr_t)sb);
	}

	/* clear the device chain */
	* (struct selitem **) hdr = 0;
	mutex_unlock(&select_lock);
}
