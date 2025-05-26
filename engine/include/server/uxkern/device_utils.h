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
 * $Log:	device_utils.h,v $
 * Revision 2.1  92/04/21  17:10:57  rwd
 * BSDSS
 * 
 *
 */

/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	device_utils.h,v $
 * Revision 2.1  92/04/21  17:10:57  rwd
 * BSDSS
 * 
 * Revision 3.0  91/01/17  12:06:02  condict
 * Unchanged copy from Mach 3.0 BSD UNIX server
 * 
 * Revision 2.3  90/06/02  15:27:32  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  20:13:46  rpd]
 * 
 * Revision 2.2  89/09/15  15:29:30  rwd
 * 	Undefine KERNEL while including device_types.h
 * 	[89/09/11            rwd]
 * 
 */
/*
 * Support routines for device interface in out-of-kernel kernel.
 */

#include <sys/param.h>
#include <sys/types.h>

#include <uxkern/import_mach.h>

#ifdef	KERNEL
#define	KERNEL__
#undef	KERNEL
#endif	KERNEL
#include <device/device_types.h>
#ifdef	KERNEL__
#undef	KERNEL__
#define	KERNEL	1
#endif	KERNEL__

extern mach_port_t	device_server_port;

/*
 * The dev_number_hash table contains both block and character
 * devices.  Distinguish the two.
 */
typedef	int	xdev_t;				/* extended device type */
#define	XDEV_BLOCK(dev)	((xdev_t)(dev) | 0x10000)
#define	XDEV_CHAR(dev)	((xdev_t)(dev))

extern void	dev_utils_init();

extern void	dev_number_hash_enter();	/* dev_t, char * */
extern void	dev_number_hash_remove();	/* dev_t */
extern char *	dev_number_hash_lookup();	/* dev_t */

extern int	dev_error_to_errno();		/* int */

