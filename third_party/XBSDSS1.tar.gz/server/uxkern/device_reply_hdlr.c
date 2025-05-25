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
 * $Log:	device_reply_hdlr.c,v $
 * Revision 2.1  92/04/21  17:11:21  rwd
 * BSDSS
 * 
 *
 */

/*
 * Handler for device read and write replies.  Simulates an
 * interrupt handler.
 */
#include <sys/queue.h>
#include <sys/zalloc.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/proc.h>

#include <uxkern/import_mach.h>
#include <uxkern/device.h>

#include <uxkern/device_reply_hdlr.h>

#define	NREPLY_HASH	64

/*
 *	We add the low 8 bits to the high 24 bits, because
 *	this interacts well with the way the IPC implementation
 *	allocates port names.
 */

#define	REPLY_HASH(port)	\
	((((port) & 0xff) + ((port) >> 8)) & (NREPLY_HASH-1))

struct reply_hash {
	struct mutex lock;
	queue_head_t queue;
	int count;
} reply_hash[NREPLY_HASH];

struct reply_entry {
	queue_chain_t	chain;
	mach_port_t	port;
	char *		object;
	kr_fn_t		read_reply;
	kr_fn_t		write_reply;
};
typedef struct reply_entry *reply_entry_t;

zone_t	reply_entry_zone;

mach_port_t	reply_port_set;

any_t	device_reply_loop();	/* forward */

extern void	ux_create_thread();

enum dev_reply_select {
	DR_READ,
	DR_WRITE
};
typedef enum dev_reply_select	dr_select_t;

void
reply_hash_enter(port, object, read_reply, write_reply)
	mach_port_t	port;
	char		*object;
	kr_fn_t		read_reply, write_reply;
{
	register reply_entry_t	re;
	register struct reply_hash *b;

	re = (reply_entry_t) zalloc(reply_entry_zone);
	re->port = port;
	re->object = object;
	re->read_reply = read_reply;
	re->write_reply = write_reply;

	b = &reply_hash[REPLY_HASH(port)];
	mutex_lock(&b->lock);
	queue_enter(&b->queue, re, reply_entry_t, chain);
	b->count++;
	mutex_unlock(&b->lock);

	(void) mach_port_move_member(mach_task_self(), port, reply_port_set);
}

boolean_t
reply_hash_lookup(port, which, object, func)
	mach_port_t	port;
	dr_select_t	which;
	char		**object;	/* OUT */
	kr_fn_t		*func;		/* OUT */
{
	register reply_entry_t	re;
	register struct reply_hash *b;

	b = &reply_hash[REPLY_HASH(port)];
	mutex_lock(&b->lock);
	for (re = (reply_entry_t)queue_first(&b->queue);
	     !queue_end(&b->queue, (queue_entry_t)re);
	     re = (reply_entry_t)queue_next(&re->chain)) {
	    if (re->port == port) {
		*object = re->object;
		*func   = (which == DR_WRITE) ? re->write_reply
					      : re->read_reply;
		mutex_unlock(&b->lock);
		return (TRUE);
	    }
	}
	mutex_unlock(&b->lock);
	return (FALSE);
}

void
reply_hash_remove(port)
	mach_port_t	port;
{
	register reply_entry_t	re;
	register struct reply_hash *b;

	(void) mach_port_move_member(mach_task_self(), port, MACH_PORT_NULL);

	b = &reply_hash[REPLY_HASH(port)];
	mutex_lock(&b->lock);
	for (re = (reply_entry_t)queue_first(&b->queue);
	     !queue_end(&b->queue, (queue_entry_t)re);
	     re = (reply_entry_t)queue_next(&re->chain)) {
	    if (re->port == port) {
		queue_remove(&b->queue, re, reply_entry_t, chain);
		b->count--;
		mutex_unlock(&b->lock);
		zfree(reply_entry_zone, (vm_offset_t)re);
		return;
	    }
	}
	mutex_unlock(&b->lock);
}

void device_reply_hdlr()
{
	register int	i;

	for (i = 0; i < NREPLY_HASH; i++) {
	    mutex_init(&reply_hash[i].lock);
	    queue_init(&reply_hash[i].queue);
	    reply_hash[i].count = 0;
	}

	reply_entry_zone =
		zinit((vm_size_t)sizeof(struct reply_entry),
		      (vm_size_t)sizeof(struct reply_entry) * 4096,
		      vm_page_size,
		      TRUE,
		      "device_reply_port to device");
	(void) mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
				  &reply_port_set);

	ux_create_thread(device_reply_loop);
}

any_t
device_reply_loop(arg)
	any_t	arg;
{
	kern_return_t		rc;

	union reply_msg {
	    mach_msg_header_t	hdr;
	    char		space[8192];
	} reply_msg;
	struct proc *p;

	/*
	 * We KNOW that none of these messages have replies...
	 */

	mig_reply_header_t	rep_rep_msg;

	system_proc(&p, "DevReplyHdlr");

	/*
	 * Wire this cthread to a kernel thread so we can
	 * up its priority
	 */
	cthread_wire();

	/*
	 * Make this thread high priority.
	 */
	set_thread_priority(mach_thread_self(), 2);

	for (;;) {
	    rc = mach_msg(&reply_msg.hdr, MACH_RCV_MSG,
			  0, sizeof reply_msg, reply_port_set,
			  MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	    if (rc == MACH_MSG_SUCCESS) {
		if (device_reply_server(&reply_msg.hdr,
					&rep_rep_msg.Head))
		{
		    /*
		     * None of these messages need replies
		     */
		}
	    }
	}
}

kern_return_t
device_open_reply(reply_port, return_code, device_port)
	mach_port_t	reply_port;
	kern_return_t	return_code;
	mach_port_t	device_port;
{
	/* NOT USED */
}

kern_return_t
device_write_reply(reply_port, return_code, bytes_written)
	mach_port_t	reply_port;
	kern_return_t	return_code;
	int		bytes_written;
{
	char		*object;
	kr_fn_t		func;

	if (!reply_hash_lookup(reply_port, DR_WRITE, &object, &func))
	    return(0);

	return ((*func)(object, return_code, bytes_written));
}

kern_return_t
device_write_reply_inband(reply_port, return_code, bytes_written)
	mach_port_t	reply_port;
	kern_return_t	return_code;
	int		bytes_written;
{
	char		*object;
	kr_fn_t		func;

	if (!reply_hash_lookup(reply_port, DR_WRITE, &object, &func))
	    return(0);

	return ((*func)(object, return_code, bytes_written));
}

kern_return_t
device_read_reply(reply_port, return_code, data, data_count)
	mach_port_t	reply_port;
	kern_return_t	return_code;
	io_buf_ptr_t	data;
	unsigned int	data_count;
{
	char		*object;
	kr_fn_t		func;

	if (!reply_hash_lookup(reply_port, DR_READ, &object, &func))
	    return(0);

	return ((*func)(object, return_code, data, data_count));
}

kern_return_t
device_read_reply_inband(reply_port, return_code, data, data_count)
	mach_port_t	reply_port;
	kern_return_t	return_code;
	io_buf_ptr_t	data;
	unsigned int	data_count;
{
	char		*object;
	kr_fn_t		func;

	if (!reply_hash_lookup(reply_port, DR_READ, &object, &func))
	    return(0);

	return ((*func)(object, return_code, data, data_count));
}
