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
 * $Log:	emul_generic.c,v $
 * Revision 2.2  92/04/22  14:00:49  rwd
 * 	Fix mach.h include.
 * 	[92/04/22            rwd]
 * 
 * Revision 2.1  92/04/21  17:27:39  rwd
 * BSDSS
 * 
 *
 */

#include <mach.h>
#include <mach/message.h>
#include <mach/msg_type.h>
#include <uxkern/bsd_msg.h>

/*
 * Generic emulated UX call.
 */
struct bsd_request bsd_req_template = {
    {
	MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE),
					/* msgh_bits */
	0,				/* msgh_size */
	MACH_PORT_NULL,			/* msgh_remote_port */
	MACH_PORT_NULL,			/* msgh_local_port */
	MACH_MSGH_KIND_NORMAL,		/* msgh_kind */
	BSD_REQ_MSG_ID			/* msgh_id */
    },
    {
	MACH_MSG_TYPE_INTEGER_32,	/* msgt_name */
	32,				/* msgt_size */
	8,				/* msgt_number */
	TRUE,				/* msgt_inline */
	FALSE,				/* msgt_longform */
	FALSE,				/* msgt_deallocate */
	0				/* msgt_unused */
    },
    0,0,
    { 0, 0, 0, 0, 0, 0 },
};

int
emul_generic(serv_port, interrupt, syscode, argp, rvalp)
	mach_port_t	serv_port;
	boolean_t	*interrupt;
	int	syscode;
	int	* argp;
	int	* rvalp;
{
	register kern_return_t	error;
	register mach_port_t	reply_port;
	union bsd_msg	bsd_msg;

    Restart:
	bsd_msg.req = bsd_req_template;

	reply_port = mig_get_reply_port();
	bsd_msg.req.hdr.msgh_remote_port = serv_port;
	bsd_msg.req.hdr.msgh_local_port  = reply_port;

	bsd_msg.req.syscode	= syscode;
	bsd_msg.req.rval2	= rvalp[1];
	bsd_msg.req.arg[0]	= argp[0];
	bsd_msg.req.arg[1]	= argp[1];
	bsd_msg.req.arg[2]	= argp[2];
	bsd_msg.req.arg[3]	= argp[3];
	bsd_msg.req.arg[4]	= argp[4];
	bsd_msg.req.arg[5]	= argp[5];

	error = mach_msg(&bsd_msg.req.hdr, MACH_SEND_MSG|MACH_RCV_MSG,
			 sizeof bsd_msg.req, sizeof bsd_msg, reply_port,
			 MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);

	if (error != MACH_MSG_SUCCESS) {
	    return (error);
	}

	error = bsd_msg.rep.retcode;
	*interrupt = bsd_msg.rep.interrupt;

	if (error == 0) {

	    register char *		start;
	    char *			end;
	    register mach_msg_type_long_t *	tp;
	    register vm_size_t		size;

	    vm_address_t	user_addr, msg_addr;

	    /*
	     * Pass return values back to caller.
	     */
	    rvalp[0] = bsd_msg.rep.rval[0];
	    rvalp[1] = bsd_msg.rep.rval[1];

	    /*
	     * Scan reply message for data to copy
	     */
	    start = (char *)&bsd_msg + sizeof(struct bsd_reply);
	    end   = (char *)&bsd_msg + bsd_msg.rep.hdr.msgh_size;
	    while (end > start) {

		/*
		 * Descriptor for address
		 */
		start += sizeof(mach_msg_type_t);

		/*
		 * Address
		 */
		user_addr = *(vm_address_t *)start;
		start += sizeof(vm_address_t);

		/*
		 * Data - size is in bytes
		 */
		tp = (mach_msg_type_long_t *)start;
		if (tp->msgtl_header.msgt_longform) {
		    size = tp->msgtl_number;
		    start += sizeof(mach_msg_type_long_t);
		}
		else {
		    size = tp->msgtl_header.msgt_number;
		    start += sizeof(mach_msg_type_t);
		}

		if (tp->msgtl_header.msgt_inline) {
		    bcopy(start, (char *)user_addr, size);
		    start += size;
		    start = (char *)
				( ((int)start + sizeof(int) - 1)
				  & ~(sizeof(int) - 1) );
		}
		else {
		    msg_addr = *(vm_address_t *)start;
		    start += sizeof(vm_address_t);
		    bcopy((char *)msg_addr, (char *)user_addr, size);
		    (void) vm_deallocate(mach_task_self(), msg_addr, size);
		}
	    }
	}

	return (error);
}

