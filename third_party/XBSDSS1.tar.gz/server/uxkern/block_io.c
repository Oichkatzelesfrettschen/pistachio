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
 * $Log:	block_io.c,v $
 * Revision 2.1  92/04/21  17:11:18  rwd
 * BSDSS
 * 
 *
 */

/*
 * Block IO using MACH KERNEL interface.
 */

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/errno.h>
#include <sys/synch.h>
#include <sys/ucred.h>

#include <uxkern/import_mach.h>
#include <uxkern/device_reply_hdlr.h>
#include <uxkern/device_utils.h>

#include <device/device.h>

/*
 * We store the block-io port in the hash table
 */
#define	bio_port_enter(dev, port) \
		dev_number_hash_enter(XDEV_BLOCK(dev), (char *)(port))
#define	bio_port_remove(dev)	\
		dev_number_hash_remove(XDEV_BLOCK(dev))
#define	bio_port_lookup(dev)	\
		((mach_port_t)dev_number_hash_lookup(XDEV_BLOCK(dev)))

kern_return_t	bio_read_reply();
kern_return_t	bio_write_reply();

extern int debug_bio;

/*
 * Open block device.
 */
bdev_open(dev, flag)
	dev_t	dev;
	int	flag;
{
	char		name[32];
	kern_return_t	rc;
	mach_port_t	device_port;
	int		mode;

	/*
	 * See whether we have opened the device already.
	 */
	if (bio_port_lookup(dev)) {
	    return (0);
	}

	rc = bdev_name_string(dev, name);
	if (rc != 0)
	    return (rc);

	/* fix modes */
	mode = 0;	/* XXX */
	rc = device_open(device_server_port,
			 mode,
			 name,
			 &device_port);
	if (rc != D_SUCCESS)
	    return (dev_error_to_errno(rc));

	bio_port_enter(dev, device_port);
	return (0);
}

bdev_close(dev, flag)
	dev_t	dev;
	int	flag;
{
	mach_port_t	device_port;
	int		error;

	device_port = (mach_port_t)bio_port_lookup(dev);
	if (device_port == MACH_PORT_NULL)
	    return;	/* shouldn't happen */

	bio_port_remove(dev);
	error = dev_error_to_errno(device_close(device_port));
	(void) mach_port_deallocate(mach_task_self(), device_port);
	return (error);
}

bdev_dump()
{
	printf("bdev_dump()----------\n"); return(0);
}

bdev_size()
{
	printf("bdev_size()----------\n"); return(0);
}

bdev_ioctl(dev, cmd, arg, mode)
dev_t	dev;
int	cmd;
caddr_t	arg;
int	mode;
{
	printf("bdev_ioctl(dev = %x, cmd = %x, arg = %x, mode = %d)---------\n",
			dev, cmd, arg, mode);
	return(EIO);
}


bio_strategy(bp)
	struct buf *	bp;
{
	dev_t	dev;
	mach_port_t	device_port;
	kern_return_t	error;

	/*
	 * Find the request port for the device.
	 */
	device_port = bio_port_lookup(bp->b_dev);
	if (device_port == MACH_PORT_NULL)
		panic("bio_strategy null port");

	/*
	 * Start the IO.  XXX What should we do in case of error???
	 * Calling bio_read_reply/bio_write_reply isn't correct,
	 * because they use interrupt_enter/interrupt_exit.
	 */

	if (bp->b_flags & B_READ) {
	    error = device_read_request(device_port,
					bp->b_reply_port,
					0,
					bp->b_blkno,
					(unsigned)bp->b_bcount);
	    if (error != KERN_SUCCESS)
		panic("bio_strategy read request", error);
	} else {
	    error = device_write_request(device_port,
					 bp->b_reply_port,
					 0,
					 bp->b_blkno,
					 bp->b_un.b_addr,
					 bp->b_bcount);
	    if (error != KERN_SUCCESS)
		panic("bio_strategy write request",error);
	}
}

kern_return_t
bio_read_reply(bp_ptr, return_code, data, data_count)
	char *		bp_ptr;
	kern_return_t	return_code;
	char		*data;
	unsigned int	data_count;
{
	register struct buf *bp = (struct buf *)bp_ptr;
	vm_offset_t dealloc_addr;
	vm_size_t dealloc_size = 0;

	interrupt_enter(SPLBIO);
	if (return_code != D_SUCCESS) {
	    bp->b_flags |= B_ERROR;
	    bp->b_error = EIO;
	} else {
	    /*
	     * Deallocate old memory.  Actually do it later,
	     * after we have lowered IPL.
	     */
	    if (bp->b_bufsize > 0) {
		dealloc_addr = (vm_offset_t) bp->b_un.b_addr;
		dealloc_size = (vm_size_t) bp->b_bufsize;
		bp->b_bufsize = 0;
	    }

	    if (data_count < bp->b_bcount) {
		bp->b_flags |= B_ERROR;
		bp->b_resid = bp->b_bcount - data_count;
	    }
	    bp->b_un.b_addr = data;
	    bp->b_bufsize = round_page(data_count);
	}
	biodone(bp);
	interrupt_exit(SPLBIO);

	if (return_code == D_SUCCESS & debug_bio) printf("r  allocate bp: %x addr: %x size: %x\n",bp,data,bp->b_bufsize);
	if (dealloc_size != 0) {
	    (void) vm_deallocate(mach_task_self(), dealloc_addr, dealloc_size);
	    if (debug_bio) printf("rdeallocate bp: %x addr: %x size: %x\n",bp,dealloc_addr,dealloc_size);
	}
}

kern_return_t
bio_write_reply(bp_ptr, return_code, bytes_written)
	char *		bp_ptr;
	kern_return_t	return_code;
	int		bytes_written;
{
	register struct buf *bp = (struct buf *)bp_ptr;

	interrupt_enter(SPLBIO);
	if (return_code != D_SUCCESS) {
	    bp->b_flags |= B_ERROR;
	    bp->b_error = EIO;
	} else if (bytes_written < bp->b_bcount) {
	    bp->b_flags |= B_ERROR;
	    bp->b_resid = bp->b_bcount - bytes_written;
	}
	biodone(bp);
	interrupt_exit(SPLBIO);
}
