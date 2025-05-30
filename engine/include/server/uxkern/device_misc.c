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
 * $Log:	device_misc.c,v $
 * Revision 2.1  92/04/21  17:11:09  rwd
 * BSDSS
 * 
 *
 */

/*
 * Device interface for out-of-kernel UX kernel.
 */
#include <uxkern/import_mach.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/synch.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/synch.h>

#include <uxkern/device.h>
#include <uxkern/device_utils.h>
#include <uxkern/device_reply_hdlr.h>
#include <mach/kern_return.h>

/*
 * Character device support.
 */
struct char_device {
	mach_port_t	c_device_port;	/* port to device */
	mach_port_t	c_select_port;	/* reply port for select */
	int             c_read_select;	/* anchor for read select list */
	int		c_read_sel_state;
					/* message sent/recvd for read-select */
#define CD_SEL_MSENT 1
#define CD_SEL_MRCVD 2
	int             c_write_select;	/* anchor for read select list */
	int		c_write_sel_state;
					/* message sent/recvd for select */
	int		c_mode;		/* IO modes: */
#define	C_NBIO		0x01		/* non-blocking IO */
#define	C_ASYNC		0x02		/* asynchronous notification */
	struct pgrp	*c_pgrp;
};

#define	char_hash_enter(dev, cp) \
		dev_number_hash_enter(XDEV_CHAR(dev), (char *)(cp))
#define	char_hash_remove(dev) \
		dev_number_hash_remove(XDEV_CHAR(dev))
#define	char_hash_lookup(dev) \
		((struct char_device *)dev_number_hash_lookup(XDEV_CHAR(dev)))

int	char_select_read_reply();	/* forward */
int	char_select_write_reply();	/* forward */

/*
 * Open device (not TTY).
 * Call device server, then enter device and port in hash table.
 */
char_open(dev, flag)
	dev_t	dev;
	int	flag;
{
	char		name[32];
	kern_return_t	rc;
	mach_port_t	device_port;
	int		mode;
	boolean_t	new_char_dev;

	register struct proc *p = (struct proc *)cthread_data(cthread_self());
	register struct char_device *cp;

	/*
	 * Check whether character device is already open.
	 */
	cp = char_hash_lookup(dev);
	if (cp == 0) {
	    /*
	     * Create new char_device structure.
	     */
	    cp = (struct char_device *)malloc(sizeof(struct char_device));
	    char_hash_enter(dev, cp);
	    cp->c_device_port = MACH_PORT_NULL;
	    cp->c_select_port = MACH_PORT_NULL;
	    cp->c_read_select = 0;
	    cp->c_read_sel_state = 0;
	    cp->c_write_select = 0;
	    cp->c_write_sel_state = 0;
	    cp->c_mode = 0;
	    cp->c_pgrp = p->p_pgrp;

	    new_char_dev = TRUE;
	}
	else {
	    new_char_dev = FALSE;
	}

	if (cp->c_device_port == MACH_PORT_NULL) {
	    /*
	     * Device is closed.  Try to open it.
	     */
	    rc = cdev_name_string(dev, name);
	    if (rc != 0)
		return (rc);	/* bad name */

	    /* fix modes */
	    mode = 0;	/* XXX */
	    rc = device_open(device_server_port,
			     mode,
			     name,
			     &device_port);
	    if (rc != D_SUCCESS) {
		if (new_char_dev) {
		    char_hash_remove(dev);
		    free((char *)cp);
		}
		return (dev_error_to_errno(rc));
	    }
	    cp->c_device_port = device_port;
	}
	return (0);
}

char_close(dev, flag)
	dev_t	dev;
	int	flag;
{
	register struct char_device *cp;
	register int	error;

	cp = char_hash_lookup(dev);

	error = dev_error_to_errno(device_close(cp->c_device_port));
	(void) mach_port_deallocate(mach_task_self(), cp->c_device_port);
	cp->c_device_port = MACH_PORT_NULL;

	reply_hash_remove(cp->c_select_port);
	(void) mach_port_mod_refs(mach_task_self(), cp->c_select_port,
				  MACH_PORT_RIGHT_RECEIVE, -1);
	cp->c_select_port = MACH_PORT_NULL;

	cp->c_read_select = 0;
	cp->c_read_sel_state = 0;
	cp->c_write_select = 0;
	cp->c_write_sel_state = 0;

	char_hash_remove(dev);
	free((char *)cp);

	return (dev_error_to_errno(error));
}

mach_port_t char_port(dev)
	dev_t dev;
{
	return (char_hash_lookup(dev)->c_device_port);
}

char_read(dev, uio)
	dev_t	dev;
	register struct uio *uio;
{
	register int	c;
	register kern_return_t	rc;
	io_buf_ptr_inband_t	data;	/* inline array */
	unsigned int	count;
	register struct char_device *cp;
	boolean_t	first = TRUE;

	cp = char_hash_lookup(dev);

	while (uio->uio_iovcnt > 0) {

	    c = uio->uio_iov->iov_len;

	    if (c > IO_INBAND_MAX)
		c = IO_INBAND_MAX;

	    count = IO_INBAND_MAX;
	    rc = device_read_inband(cp->c_device_port,
				((cp->c_mode & C_NBIO) || !first)
					? D_NOWAIT
					: 0,
				0,
				c,
				&data[0],
				&count);
	    if (rc != 0) {
		if (rc == D_WOULD_BLOCK && !first)
		    break;
		return (dev_error_to_errno(rc));
	    }
	    first = FALSE;

	    uiomove(&data[0], count, uio);
	}

	if (cp->c_mode & C_ASYNC) {
	    /*
	     * Post read request if not already posted.
	     */
	    if ((cp->c_read_sel_state & CD_SEL_MSENT) == 0) {
		cp->c_read_sel_state |= CD_SEL_MSENT;
		(void) device_read_request_inband(cp->c_device_port,
						  cp->c_select_port,
						  0,	/* mode */
						  0,	/* recnum */
						  0);	/* bytes wanted */
	    }
	}
	return (0);
}

char_write(dev, uio)
	dev_t	dev;
	struct uio *uio;
{
	/* break up the uio into individual device_write calls */
}

char_ioctl(dev, cmd, data, flag)
	dev_t	dev;
	int	cmd;
	caddr_t	data;
	int	flag;
{
	register struct char_device *cp;
	unsigned int	count;
	register int	error;
	register struct proc *p = (struct proc *)cthread_data(cthread_self());

	cp = char_hash_lookup(dev);

	if (cmd == FIONBIO) {
	    if (*(int *)data)
		cp->c_mode |= C_NBIO;
	    else
		cp->c_mode &= ~C_NBIO;
	    return (0);
	}

	if (cmd == FIOASYNC) {
	    if (*(int *)data) {
		cp->c_mode |= C_ASYNC;

		/*
		 * If select reply port does not exist, create it.
		 */
		if (cp->c_select_port == MACH_PORT_NULL) {
		    cp->c_select_port = mach_reply_port();
		    reply_hash_enter(cp->c_select_port,
				     (char *)cp,
				     char_select_read_reply,
				     char_select_write_reply);
		}
		/*
		 * Post read request if not already posted.
		 */
		if ((cp->c_read_sel_state & CD_SEL_MSENT) == 0) {
		    cp->c_read_sel_state |= CD_SEL_MSENT;
		    (void) device_read_request_inband(cp->c_device_port,
					cp->c_select_port,
					0,	/* mode */
					0,	/* recnum */
					0);	/* bytes wanted */
		}
	    }
	    else {
		cp->c_mode &= ~C_ASYNC;
	    }
	    return (0);
	}

#ifdef	i386
    {
	struct X_kdb {
	    u_int *ptr;
	    u_int size;
	};

#define K_X_KDB_ENTER	_IOW('K', 16, struct X_kdb)
#define K_X_KDB_EXIT	_IOW('K', 17, struct X_kdb)

	if ((cmd == K_X_KDB_ENTER) ||
	    (cmd == K_X_KDB_EXIT)) {
	    u_int X_kdb_buffer[512];
	    struct X_kdb *X_kdb = (struct X_kdb *) data;

	    /* make sure that copyin will do the right thing */
	    if (p->p_reply_msg == 0)
		panic("X_K_KDB ioctl");

	    if (X_kdb->size > sizeof X_kdb_buffer)
		return (ENOENT);

	    if (copyin(X_kdb->ptr, X_kdb_buffer, X_kdb->size))
		return (EFAULT);

	    error = device_set_status(cp->c_device_port, cmd,
				      X_kdb_buffer, X_kdb->size>>2);
	    if (error)
		return (ENOTTY);

	    return (0);
	}
    }
#endif	i386

	count = (cmd & ~(IOC_INOUT|IOC_VOID)) >> 16; /* bytes */
	count = (count + 3) >> 2;		     /* ints */
	if (count == 0)
	    count = 1;

	if (cmd & (IOC_VOID|IOC_IN)) {
	    error = device_set_status(cp->c_device_port,
				      cmd,
				      (int *)data,
				      count);
	    if (error)
		return (ENOTTY);
	}
	if (cmd & IOC_OUT) {
	    error = device_get_status(cp->c_device_port,
				      cmd,
				      (int *)data,
				      &count);
	    if (error)
		return (ENOTTY);
	}
	return (0);
}

char_select(dev, rw)
	dev_t	dev;
	int	rw;
{
	register struct char_device *cp;
	register struct proc *p = (struct proc *)cthread_data(cthread_self());
	register int s;

	cp = char_hash_lookup(dev);

	/*
	 * If select reply port does not exist, create it.
	 */
	if (cp->c_select_port == MACH_PORT_NULL) {
	    cp->c_select_port = mach_reply_port();
	    reply_hash_enter(cp->c_select_port,
			     (char *)cp,
			     char_select_read_reply,
			     char_select_write_reply);
	}

	s = spltty();

	switch (rw) {
	    case FREAD:
		/*
		 * If reply available, consume it
		 */
		if (cp->c_read_sel_state & CD_SEL_MRCVD) {
		    cp->c_read_sel_state = 0;
		    splx(s);
		    return (1);
		}
		/*
		 * Post read request if not already posted.
		 */
		if ((cp->c_read_sel_state & CD_SEL_MSENT) == 0) {
		    cp->c_read_sel_state |= CD_SEL_MSENT;
		    (void) device_read_request_inband(cp->c_device_port,
					cp->c_select_port,
					0,	/* mode */
					0,	/* recnum */
					0);	/* bytes wanted */
		}
		selenter(p, &cp->c_read_select);
		break;

	    case FWRITE:
		/*
		 * If reply available, consume it
		 */
		if (cp->c_write_sel_state & CD_SEL_MRCVD) {
		    cp->c_write_sel_state = 0;
		    splx(s);
		    return (1);
		}
		/*
		 * Post write request if not already posted.
		 */
		if ((cp->c_write_sel_state & CD_SEL_MSENT) == 0) {
		    cp->c_write_sel_state |= CD_SEL_MSENT;
		    (void) device_write_request_inband(cp->c_device_port,
					cp->c_select_port,
					0,	/* mode */
					0,	/* recnum */
					0,	/* data */
					0);	/* bytes wanted */
		}
		selenter(p, &cp->c_write_select);
		break;
	}

	splx(s);

	return (0);
}

/*
 * Handler for c_select_port replies.
 */
char_select_read_reply(cp, error, data, size)
	register struct char_device *cp;
	int		error;
	char		*data;
	unsigned int	size;
{
	interrupt_enter(SPLTTY);
	cp->c_read_sel_state = CD_SEL_MRCVD;
	selwakeup(&cp->c_read_select);
	if (cp->c_mode & C_ASYNC)
	    gsignal(cp->c_pgrp, SIGIO);
	interrupt_exit(SPLTTY);
}

char_select_write_reply(cp, error, size)
	register struct char_device *cp;
	int		error;
	unsigned int	size;
{
	interrupt_enter(SPLTTY);
	cp->c_write_sel_state = CD_SEL_MRCVD;
	selwakeup(&cp->c_write_select);
	interrupt_exit(SPLTTY);
}

/*

 */

/*
 * Device memory object support.
 */
memory_object_t
device_pager_create(dev, offset, size, protection)
	dev_t		dev;
	vm_offset_t	offset;
	vm_size_t	size;
	vm_prot_t	protection;
{
	mach_port_t	(*d_port_routine)();
	mach_port_t	device_port;
	memory_object_t	pager;
	int		status;

	d_port_routine = cdevsw[major(dev)].d_port;
	if (d_port_routine == (mach_port_t (*)())0)
		return (MACH_PORT_NULL);
	device_port = (*d_port_routine)(dev);
	if (device_port == MACH_PORT_NULL)
		return (MACH_PORT_NULL);

	status = device_map(device_port, protection, offset, 
				   size, &pager, 0); 
	if (status != KERN_SUCCESS) {
		return (MACH_PORT_NULL);
	}
	return (pager);
}

device_pager_release(mem_obj)
	memory_object_t	mem_obj;
{
	kern_return_t kr;

	kr = mach_port_deallocate(mach_task_self(), mem_obj);
	if (kr != KERN_SUCCESS)
		panic("device_pager_release");
}

/*
 * Memory device.
 */
#define M_KMEM		1	/* /dev/kmem - virtual kernel memory & I/O */
#define M_NULL		2	/* /dev/null - EOF & Rathole */

mmopen(dev, flag)
	dev_t	dev;
	int	flag;
{
	switch (minor(dev)) {
	    case M_KMEM:
	    case M_NULL:
		return (0);
	}
	return (ENXIO);
}

/*ARGSUSED*/
mmread(dev, uio)
	dev_t	dev;
	struct uio *uio;
{
	return (mmrw(dev, uio, UIO_READ));
}

mmwrite(dev, uio)
	dev_t	dev;
	struct uio *uio;
{
	return (mmrw(dev, uio, UIO_WRITE));
}	

mmrw(dev, uio, rw)
	dev_t dev;
	struct uio *uio;
	enum uio_rw rw;

{
	register u_int c;
	register struct iovec *iov;
	int error = 0;

	while (uio->uio_resid > 0 && error == 0) {
		iov = uio->uio_iov;
		if (iov->iov_len == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			if (uio->uio_iovcnt < 0)
				panic("mmrw");
			continue;
		}
		switch (minor(dev)) {

		case M_KMEM:
			c = iov->iov_len;
			if (mmca((vm_address_t)uio->uio_offset, (vm_size_t)c,
			    rw == UIO_READ ? B_READ : B_WRITE)) {
				error = uiomove((caddr_t)uio->uio_offset,
					(int)c, uio);
				continue;
			}
			return (EFAULT);

		case M_NULL:
			if (rw == UIO_READ)
				return (0);
			c = iov->iov_len;
			iov->iov_base += c;
			iov->iov_len -= c;
			uio->uio_offset += c;
			uio->uio_resid -= c;
			break;

		}
	}
	return (error);
}

/*
 *	Returns true if the region is readable.
 */

mmca(address, count)
    vm_address_t address;
    vm_size_t count;
{
    register vm_offset_t	addr;
    vm_offset_t			r_addr;
    vm_size_t			r_size;
    vm_prot_t			r_protection,
				r_max_protection;
    vm_inherit_t		r_inheritance;
    boolean_t			r_is_shared;
    memory_object_name_t	r_object_name;
    vm_offset_t			r_offset;

    addr = address;
    while (addr < address + count) {
	r_addr = addr;
	if (vm_region(mach_task_self(),
		      &r_addr,
		      &r_size,
		      &r_protection,
		      &r_max_protection,
		      &r_inheritance,
		      &r_is_shared,
		      &r_object_name,
		      &r_offset) != KERN_SUCCESS)
	    return (0);

	if (MACH_PORT_VALID(r_object_name))
	    (void) mach_port_deallocate(mach_task_self(), r_object_name);

	/* is there a gap? */
	if (r_addr > addr)
	    return (0);

	/* is this region not readable? */
	if ((r_protection & VM_PROT_READ) != VM_PROT_READ)
	    return (0);

	/* continue to next region */
	addr = r_addr + r_size;
    }
    return (1);
}

int
cdev_name_string(dev, str)
	dev_t	dev;
	char	str[];	/* REF OUT */
{
	int	major_num = major(dev);
	int	minor_num = minor(dev);
	int	d_flag;

	if (major_num < 0 || major_num >= nchrdev)
	    return (ENXIO);

	d_flag = cdevsw[major_num].d_flags;
	if (d_flag & C_MINOR) {
	    /*
	     * Must check minor device number -
	     * not all minors for this device translate to
	     * the same name
	     */
	    return (check_dev(dev, str));
	}
	else {
	    strcpy(str, cdevsw[major_num].d_name);
	    if (d_flag & C_BLOCK(0)) {
		/*
		 * Disk device - break minor into dev/partition
		 */
		char num[3];
		register int part_count;

		part_count = C_BLOCK_GET(d_flag);
		itoa(minor_num/part_count, num);
		strcat(str, num);
		num[0] = 'a' + (minor_num % part_count);
		num[1] = 0;
		strcat(str, num);
	    }
	    else {
		/*
		 * Add minor number
		 */
		char num[4];
		itoa(minor_num, num);
		strcat(str, num);
	    }
	}
	return (0);
}

int
bdev_name_string(dev, str)
	dev_t	dev;
	char	str[];	/* REF OUT */
{
	int	major_num = major(dev);
	int	minor_num = minor(dev);
	char	num[3];
	int	part_count;

	if (major_num < 0 || major_num >= nblkdev)
	    return (ENXIO);

	strcpy(str, bdevsw[major_num].d_name);

	part_count = C_BLOCK_GET(bdevsw[major_num].d_flags);

	itoa(minor_num/part_count, num);
	strcat(str, num);
	num[0] = 'a' + (minor_num % part_count);
	num[1] = '\0';
	strcat(str, num);

	return (0);
}

itoa(num, str)
	int	num;
	char	str[];
{
	char	digits[11];
	register char *dp;
	register char *cp = str;

	if (num == 0) {
	    *cp++ = '0';
	}
	else {
	    dp = digits;
	    while (num) {
		*dp++ = '0' + num % 10;
		num /= 10;
	    }
	    while (dp != digits) {
		*cp++ = *--dp;
	    }
	}
	*cp++ = '\0';
}

/*
 * Parse root device name into a block device number.
 */
dev_t
parse_root_device(str)
	char	*str;
{
	register char c;
	register char *cp = str;
	char *name_end;
	register int minor_num = 0;
	register int major_num;
	register struct bdevsw *bdp;

	/*
	 * Find device type name (characters before digit)
	 */
	while ((c = *cp) != '\0' &&
		!(c >= '0' && c <= '9'))
	    cp++;
	name_end = cp;

	if (c != '\0') {
	    /*
	     * Find minor_num number
	     */
	    while ((c = *cp) != '\0' &&
		    c >= '0' && c <= '9') {
		minor_num = minor_num * 10 + (c - '0');
		cp++;
	    }
	}
	if (c >= 'a' && c <= 'h') {
	    /*
	     * Disk minor number is 16*minor_num + partition.
	     */
	    minor_num = minor_num * 16 + (c - 'a');
	}
	*name_end = 0;

	for (major_num = 0, bdp = bdevsw;
	     major_num < nblkdev;
	     major_num++, bdp++) {
	    if (!strcmp(str, bdp->d_name))
		break;
	}
	if (major_num == nblkdev) {
	    /* not found */
	    return ((dev_t)-1);
        }
	return (makedev(major_num, minor_num));
}
