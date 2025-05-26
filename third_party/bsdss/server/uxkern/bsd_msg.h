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
 * $Log:	bsd_msg.h,v $
 * Revision 2.1  92/04/21  17:10:46  rwd
 * BSDSS
 * 
 *
 */

/*
 * Request and reply message for generic BSD kernel call.
 */

#include <mach/boolean.h>
#include <mach/message.h>

struct	bsd_request {
    mach_msg_header_t	hdr;
    mach_msg_type_t	int_type;	/* int[8] */
    int			rval2;
    int			syscode;
    int			arg[6];
};

struct	bsd_reply {
    mach_msg_header_t	hdr;
    mach_msg_type_t	int_type;	/* int[4] */
    int			retcode;
    int			rval[2];
    boolean_t		interrupt;
};

union bsd_msg {
    struct bsd_request	req;
    struct bsd_reply	rep;
    char		msg[8192];
};

#define	BSD_REQ_MSG_ID		100000
