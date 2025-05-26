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
 * $Log:	disk_io.c,v $
 * Revision 2.1  92/04/21  17:10:46  rwd
 * BSDSS
 * 
 *
 */

/*
 * Routines for char IO to block devices.
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/errno.h>

#include <uxkern/import_mach.h>
#include <uxkern/device_utils.h>

#define	disk_port_enter(dev, port) \
		dev_number_hash_enter(XDEV_CHAR(dev), (char *)(port))
#define	disk_port_remove(dev) \
		dev_number_hash_remove(XDEV_CHAR(dev))
#define	disk_port_lookup(dev) \
		((mach_port_t) dev_number_hash_lookup(XDEV_CHAR(dev)))

int
disk_open(dev, flag)
	dev_t	dev;
	int	flag;
{
	char		name[32];
	kern_return_t	rc;
	mach_port_t	device_port;
	int		mode;

	rc = cdev_name_string(dev, name);
	if (rc != 0)
	    return (rc);	/* bad name */

	/* fix modes */
	mode = 0;	/* XXX */
	rc = device_open(device_server_port,
			 mode,
			 name,
			 &device_port);
	if (rc != D_SUCCESS)
	    return (dev_error_to_errno(rc));

	disk_port_enter(dev, device_port);
	return (0);
}

int
disk_close(dev, flag)
	dev_t	dev;
	int	flag;
{
	mach_port_t	device_port;
	int		error;

	device_port = disk_port_lookup(dev);
	if (device_port == MACH_PORT_NULL)
	    return (0);		/* should not happen */

	disk_port_remove(dev);
	error = dev_error_to_errno(device_close(device_port));
	(void) mach_port_deallocate(mach_task_self(), device_port);
	return (error);
}

int
disk_read(dev, uio)
	dev_t		dev;
	struct uio	*uio;
{
	register struct iovec *iov;
	register int	c;
	register kern_return_t	rc;
	io_buf_ptr_t	data;
	unsigned int	count;

	while (uio->uio_iovcnt > 0) {

	    iov = uio->uio_iov;
	    if (iov->iov_len == 0) {
		uio->uio_iovcnt--;
		uio->uio_iov++;
		continue;
	    }

	    if (useracc(iov->iov_base, (u_int)iov->iov_len, 0) == 0)
		return (EFAULT);

	    /*
	     * Can read entire block here - device handler
	     * breaks into smaller pieces.
	     */

	    c = iov->iov_len;

	    rc = device_read(disk_port_lookup(dev),
			     0,	/* mode */
			     btodb(uio->uio_offset),
			     iov->iov_len,
			     &data,
			     &count);
	    if (rc != 0)
		return (dev_error_to_errno(rc));

	    (void) moveout(data, iov->iov_base, count);
			/* deallocates data (eventually) */

	    iov->iov_base += count;
	    iov->iov_len -= count;
	    uio->uio_resid -= count;
	    uio->uio_offset += count;

	    /* temp kludge for tape drives */
	    if (count < c)
		break;
	}
	return (0);
}

int
disk_write(dev, uio)
	dev_t		dev;
	struct uio	*uio;
{
	register struct iovec *iov;
	register int	c;
	register kern_return_t	rc;
	vm_offset_t	kern_addr;
	vm_size_t	kern_size;
	vm_offset_t	off;
	vm_size_t	count;

	while (uio->uio_iovcnt > 0) {
	    iov = uio->uio_iov;

	    kern_size = iov->iov_len;
	    (void) vm_allocate(mach_task_self(), &kern_addr, kern_size, TRUE);
	    if (copyin(iov->iov_base, kern_addr, (u_int)iov->iov_len)) {
		(void) vm_deallocate(mach_task_self(), kern_addr, kern_size);
		return (EFAULT);
	    }

	    /*
	     * Can write entire block here - device handler
	     * breaks into smaller pieces.
	     */

	    c = iov->iov_len;

	    rc = device_write(disk_port_lookup(dev),
			      0,	/* mode */
			      btodb(uio->uio_offset),
			      kern_addr,
			      iov->iov_len,
			      &count);

	    (void) vm_deallocate(mach_task_self(), kern_addr, kern_size);

	    if (rc != 0)
		return (dev_error_to_errno(rc));

	    iov->iov_base += count;
	    iov->iov_len -= count;
	    uio->uio_resid -= count;
	    uio->uio_offset += count;

	    /* temp kludge for tape drives */
	    if (count < c)
		break;

	    uio->uio_iov++;
	    uio->uio_iovcnt--;
	}
	return (0);
}

disk_ioctl(dev, cmd, data, flag)
	dev_t	dev;
	int	cmd;
	caddr_t	data;
	int	flag;
{
	mach_port_t	device_port = disk_port_lookup(dev);
	unsigned int	count;
	register int	error;

	count = (cmd & ~(IOC_INOUT|IOC_VOID)) >> 16; /* bytes */
	count = (count + 3) >> 2;		     /* ints */
	if (count == 0)
	    count = 1;

	if (cmd & (IOC_VOID|IOC_IN)) {
	    error = device_set_status(device_port,
				      cmd,
				      (int *)data,
				      count);
	    if (error)
		return (dev_error_to_errno(error));
	}
	if (cmd & IOC_OUT) {
	    error = device_get_status(device_port,
				      cmd,
				      (int *)data,
				      &count);
	}
	if (error)
	     return (dev_error_to_errno(error));
	else
	     return (0);
}

mach_port_t
disk_port(dev)
	dev_t	dev;
{
	return (disk_port_lookup(dev));
}
