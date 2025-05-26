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
 * $Log:	device_reply_hdlr.h,v $
 * Revision 2.1  92/04/21  17:10:54  rwd
 * BSDSS
 * 
 *
 */

/*
 * Handler for device read and write replies.
 */
#ifndef	DEVICE_REPLY_HDLR_H
#define	DEVICE_REPLY_HDLR_H

#include <mach/kern_return.h>

typedef kern_return_t	(*kr_fn_t)();

extern void	reply_hash_enter();	/* mach_port_t, char *, kr_fn_t, kr_fn_t */
extern void	reply_hash_remove();	/* mach_port_t */

extern void	device_reply_hdlr();

#endif	DEVICE_REPLY_HDLR_H

