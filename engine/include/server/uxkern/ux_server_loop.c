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
 * $Log:	ux_server_loop.c,v $
 * Revision 2.1  92/04/21  17:11:17  rwd
 * BSDSS
 * 
 *
 */

/*
 * Server thread utilities and main loop.
 */
#include <uxkern/import_mach.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/user.h>

mach_port_t ux_server_port_set = MACH_PORT_NULL;

/*
 * Number of server threads available to handle user messages.
 */
struct mutex	ux_server_thread_count_lock = MUTEX_INITIALIZER;
int		ux_server_thread_count = 0;
int		ux_server_thread_min = 4;
int		ux_server_receive_min = 2;
int		ux_server_receive_max = 6;
int		ux_server_thread_max = 80;
int		ux_server_stack_size = 4096*16;
/* This must be at least 5 (wired) + 1 (running) + receive_max */
int		ux_server_max_kernel_threads = 13;

void ux_create_single_server_thread(); /* forward */

void
ux_server_init()
{
	mach_port_t first_port;
	ur_cthread_t tmp;
	kern_return_t kr;

	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
				&ux_server_port_set);

	cthread_set_kernel_limit(ux_server_max_kernel_threads);

}

void
ux_server_add_port(port)
	mach_port_t	port;
{
	(void) mach_port_move_member(mach_task_self(),
				     port, ux_server_port_set);
}

void
ux_server_remove_port(port)
	mach_port_t	port;
{
	(void) mach_port_move_member(mach_task_self(), port, MACH_PORT_NULL);
}

any_t
ux_thread_bootstrap(arg)
	any_t	arg;
{
	register int	routine_addr = (int)arg;
	cthread_fn_t	real_routine = (cthread_fn_t)routine_addr;
	register int calc;

	return ((*real_routine)((any_t)0));
}

/*
 * Create a thread
 */
void
ux_create_thread(routine)
	cthread_fn_t	routine;
{
	int		routine_addr = (int)routine;

	cthread_detach(cthread_fork(ux_thread_bootstrap,
				    (any_t)routine_addr));
}

any_t	ux_server_loop();	/* forward */

void
ux_create_server_thread()
{
	ux_create_thread(ux_server_loop);
}

void
ux_server_thread_busy()
{
	cthread_msg_busy(ux_server_port_set,
			 ux_server_receive_min,
			 ux_server_receive_max);
	mutex_lock(&ux_server_thread_count_lock);
	if (--ux_server_thread_count < ux_server_thread_min) {
	    mutex_unlock(&ux_server_thread_count_lock);
	    ux_create_server_thread();
	}
	else {
	    mutex_unlock(&ux_server_thread_count_lock);
	}
}

void
ux_server_thread_active()
{
	cthread_msg_active(ux_server_port_set,
			 ux_server_receive_min,
			 ux_server_receive_max);
	mutex_lock(&ux_server_thread_count_lock);
	++ux_server_thread_count;
	mutex_unlock(&ux_server_thread_count_lock);
}

#define	ux_server_thread_check() \
	(ux_server_thread_count > ux_server_thread_max)

/*
 * Main loop of UX server.
 */
/*ARGSUSED*/
any_t
ux_server_loop(arg)
	any_t	arg;
{
	register kern_return_t	ret;

	union request_msg {
	    mach_msg_header_t	hdr;
	    mig_reply_header_t	death_pill;
	    char		space[8192];
	} msg_buffer_1, msg_buffer_2;

	mach_msg_header_t * request_ptr;
	mig_reply_header_t * reply_ptr;
	mach_msg_header_t * tmp;

	char	name[64];

	sprintf(name, "ux server thread %x", cthread_self());
	cthread_set_name(cthread_self(), name);

	ux_server_thread_active();

	request_ptr = &msg_buffer_1.hdr;
	reply_ptr = &msg_buffer_2.death_pill;

	do {
	    ret = cthread_mach_msg(request_ptr, MACH_RCV_MSG,
				   0, sizeof msg_buffer_1, ux_server_port_set,
				   MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL,
				   ux_server_receive_min,
				   ux_server_receive_max);
	    if (ret != MACH_MSG_SUCCESS)
		panic("ux_server_loop: receive", ret);
	    while (ret == MACH_MSG_SUCCESS) {
		if (!bsd_1_server(request_ptr, &reply_ptr->Head))
		if (!ux_generic_server(request_ptr, &reply_ptr->Head))
		if (!exc_server(request_ptr, &reply_ptr->Head))
		{}

		if (reply_ptr->Head.msgh_remote_port == MACH_PORT_NULL) {
		    /* no reply port, just get another request */
		    break;
		}

		if (reply_ptr->RetCode == MIG_NO_REPLY) {
		    /* deallocate reply port right */
		    (void) mach_port_deallocate(mach_task_self(),
					reply_ptr->Head.msgh_remote_port);
		    break;
		}

		ret = cthread_mach_msg(&reply_ptr->Head,
				       MACH_SEND_MSG|MACH_RCV_MSG,
				       reply_ptr->Head.msgh_size,
				       sizeof msg_buffer_2,
				       ux_server_port_set,
				       MACH_MSG_TIMEOUT_NONE,
				       MACH_PORT_NULL,
				       ux_server_receive_min,
				       ux_server_receive_max);
		if (ret != MACH_MSG_SUCCESS) {
		    if (ret == MACH_SEND_INVALID_DEST) {
			/* deallocate reply port right */
			/* XXX should destroy entire reply msg */
			(void) mach_port_deallocate(mach_task_self(),
					reply_ptr->Head.msgh_remote_port);
		    } else
			panic("ux_server_loop: rpc", ret);
		}

		tmp = request_ptr;
		request_ptr = (mach_msg_header_t *) reply_ptr;
		reply_ptr = (mig_reply_header_t *) tmp;
	    }

	} while (!ux_server_thread_check());

	ux_server_thread_busy();

        printf("Server loop done: %s\n",name);

	return ((any_t)0);	/* exit */
}

