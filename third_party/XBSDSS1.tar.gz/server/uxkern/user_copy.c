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
 * $Log:	user_copy.c,v $
 * Revision 2.3  92/07/08  16:21:58  mrt
 * 	Changed routines to match the prototypes in systm.h
 * 
 * Revision 2.2  92/06/25  17:31:46  mrt
 * 	Removed the rcsci.
 * 	[92/06/25            mrt]
 * 
 * Revision 2.1  92/04/21  17:10:43  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1982, 1986, 1991 Regents of the University of California.
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
 *	@(#)kern_subr.c	7.7 (Berkeley) 4/15/91
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <ufs/dir.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/parallel.h>

#include <uxkern/import_mach.h>

extern char *extend_reply_msg();
extern void extend_current_output();

uiomove(cp, n, uio)
	register caddr_t cp;
	register int n;
	register struct uio *uio;
{
	register enum uio_rw rw = uio->uio_rw;
	register struct iovec *iov;
	u_int cnt;
	int error = 0;
	register struct proc *p = (struct proc *)cthread_data(cthread_self());

	while (n > 0 && uio->uio_resid) {
		iov = uio->uio_iov;
		cnt = iov->iov_len;
		if (cnt == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			continue;
		}
		if (cnt > n)
			cnt = n;
		if (uio->uio_segflg == UIO_SYSSPACE || p->p_reply_msg == 0) {
		    /*UIO_SYSSPACE*/
			if (rw == UIO_READ)
				bcopy(cp, iov->iov_base, cnt);
			else {
				/*
				 * Unlock master lock to touch user data.
				 */
				if (p->p_master_lock)
				    master_unlock();

				bcopy(iov->iov_base, cp, cnt);

				/*
				 * Re-lock master lock.
				 */
				if (p->p_master_lock)
				    master_lock();
			}
		}
		else {
		    /*UIO_USERSPACE*/
			if (rw == UIO_READ) {
				register caddr_t user_addr;

				user_addr = extend_reply_msg(iov->iov_base,
							     iov->iov_len);
				bcopy(cp, user_addr, cnt);
				extend_current_output(cnt);
			}
			else {
				error = copyin(iov->iov_base, cp, cnt);
				if (error)
					return (error);
			}
		}
		iov->iov_base += cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		uio->uio_offset += cnt;
		cp += cnt;
		n -= cnt;
	}
	return (error);
}

/*
 * Give next character to user as result of read.
 */
ureadc(c, uio)
	register int c;
	register struct uio *uio;
{
	register struct iovec *iov;
	register struct proc *p = (struct proc *)cthread_data(cthread_self());

again:
	if (uio->uio_iovcnt == 0)
		panic("ureadc");
	iov = uio->uio_iov;
	if (iov->iov_len <= 0 || uio->uio_resid <= 0) {
		uio->uio_iovcnt--;
		uio->uio_iov++;
		goto again;
	}
	if (uio->uio_segflg == UIO_SYSSPACE || p->p_reply_msg == 0) {
	    /*UIO_SYSSPACE*/
		*iov->iov_base = c;
	}
	else {
	    /*UIO_USERSPACE*/
		register char *		user_addr;

		user_addr = extend_reply_msg(iov->iov_base, iov->iov_len);
		*user_addr = c;
		extend_current_output(1);
	}
	iov->iov_base++;
	iov->iov_len--;
	uio->uio_resid--;
	uio->uio_offset++;
	return (0);
}
int
copyout(from, to, len)
	void	*from, *to;
	u_int	len;

{
	register struct proc *p = (struct proc *)cthread_data(cthread_self());
	if (p->p_reply_msg == 0) {
	    /*UIO_SYSSPACE*/
		bcopy(from, to, len);
	}
	else {
	    /*UIO_USERSPACE*/
		register caddr_t	user_addr;

		user_addr = extend_reply_msg(to, len);
		bcopy(from, user_addr, len);
		extend_current_output(len);
	}
	return (0);
}
int
subyte(addr, byte)
	void *addr;
	int byte;
{
	return (copyout( &byte, addr, sizeof(char)) == 0 ? 0 : -1);
}

/*----------------*/

/*
 * Get next character written in by user from uio.
 */
uwritec(uio)
	struct uio *uio;
{
	register struct iovec *iov;
	register int c;
	register struct proc *p = (struct proc *)cthread_data(cthread_self());

	if (uio->uio_resid <= 0)
		return (-1);
again:
	if (uio->uio_iovcnt <= 0)
		panic("uwritec");
	iov = uio->uio_iov;
	if (iov->iov_len == 0) {
		uio->uio_iov++;
		if (--uio->uio_iovcnt == 0)
			return (-1);
		goto again;
	}
	if (uio->uio_segflg == UIO_SYSSPACE || p->p_reply_msg == 0) {
	    /*UIO_SYSSPACE*/
		c = *iov->iov_base & 0377;
	}
	else {
	    /*UIO_USERSPACE*/
		c = fubyte(iov->iov_base);
	}
	if (c < 0)
		return (-1);
	iov->iov_base++;
	iov->iov_len--;
	uio->uio_resid--;
	uio->uio_offset++;
	return (c & 0377);
}

int copyin(from, to, len)
	void *	from;
	void *	to;
	u_int	len;
{
	vm_offset_t	start, end;
	char		*data;
	vm_size_t	data_len;
	kern_return_t	result;
	register struct proc *p = (struct proc *)cthread_data(cthread_self());

	if (p->p_reply_msg == 0) {
	    /*UIO_SYSSPACE*/
		bcopy(from, to, len);
		return (0);
	}
	    /*UIO_USERSPACE*/

	start = trunc_page((vm_offset_t)from);
	end   = round_page((vm_offset_t)from + len);

	/*
	 * Unlock master lock to touch user data.
	 */
	if (p->p_master_lock)
	    master_unlock();

	result = vm_read(p->p_task,
			 start,
			 (vm_size_t)(end - start),
			 (pointer_t *)  &data,
			 &data_len);
	if (result == KERN_SUCCESS) {
	    bcopy(data + ((vm_offset_t)from - start),
		  to,
		  len);
	    (void) vm_deallocate(mach_task_self(),
				(vm_offset_t)data,
				data_len);
	}
	else {
	    result = EFAULT;
	}

	/*
	 * Re-lock master lock.
	 */
	if (p->p_master_lock)
	    master_lock();

	return (result);
}

int fuword(addr)
	void	*addr;
{
	int	word;

	if (copyin(addr, &word, sizeof(word)))
	    return (-1);
	return (word);
}

int fubyte(addr)
	void	*addr;
{
	char	byte;

	if (copyin(addr, &byte, sizeof(byte)))
	    return (-1);
	return ((int)byte & 0xff);
}

copyinstr(from, to, max_len, len_copied)
	void	*from;
	register void *to;
	u_int	max_len;
	u_int	*len_copied;
{
	vm_offset_t	start, end;
	char		* data;
	vm_size_t	data_len;
	kern_return_t	result;

	register int	count, cur_max;
	register char	* cp;
	register struct proc *p = (struct proc *)cthread_data(cthread_self());

	if (p->p_reply_msg == 0) {
	    /*UIO_SYSSPACE*/
		return (copystr(from, to, max_len, len_copied));
	}
	    /*UIO_USERSPACE*/

	/*
	 * Unlock master lock to touch user data.
	 */
	if (p->p_master_lock)
	    master_unlock();

	count = 0;

	while (count < max_len) {
	    start = trunc_page((vm_offset_t)from);
	    end   = start + vm_page_size;

	    result = vm_read(p->p_task,
			     start,
			     vm_page_size,
			     (pointer_t *) &data,
			     &data_len);
	    if (result != KERN_SUCCESS) {
		/*
		 * Re-lock master lock.
		 */
		if (p->p_master_lock)
		    master_lock();

		return (EFAULT);
	    }

	    cur_max = end - (vm_offset_t)from;
	    if (cur_max > max_len)
		cur_max = max_len;

	    cp = data + ((vm_offset_t)from - start);
	    while (count < cur_max) {
		count++;
		if ((*((char *)to)++ = *cp++) == 0) {
		    goto done;
		}
	    }
	    (void) vm_deallocate(mach_task_self(),
				 (vm_offset_t)data,
				 data_len);
	    from = (void *)end;
	}

	/*
	 * Re-lock master lock.
	 */
	if (p->p_master_lock)
	    master_lock();

	return (ENOENT);

    done:
	if (len_copied)
	    *len_copied = count;

	(void) vm_deallocate(mach_task_self(),
			     (vm_offset_t)data,
			     data_len);
	/*
	 * Re-lock master lock.
	 */
	if (p->p_master_lock)
	    master_lock();

	return (0);
}

copystr(from, to, max_len, len_copied)
	register void *from, *to;
	u_int		max_len;
	u_int		*len_copied;
{
	register int	count;

	count = 0;
	while (count < max_len) {
	    count++;
	    if ((*((char *)to)++ = *((char *)from)++) == 0) {
		goto done;
	    }
	}
	return (ENOENT);
    done:
	if (len_copied)
	    *len_copied = count;
	return (0);
}

/*
 * Check address.
 * Given virtual address, byte count, and flag (nonzero for READ).
 * Returns 0 on no access.
 */
useracc(user_addr, len, direction)
	caddr_t	user_addr;
	u_int	len;
	int	direction;
{
	/*
	 * We take the 'hit' in the emulator (SIGSEGV) instead of the
	 * UX server (EFAULT).  If we fault, the system call will have
	 * been completed: this is a change to the existing semantics,
	 * but only brain-damaged programs should be able to tell
	 * the difference.
	 */
	return (1);	/* always */
}
