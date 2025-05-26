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
 * $Log:	emul_init.c,v $
 * Revision 2.2  92/04/22  14:00:51  rwd
 * 	Fix mach.h include.
 * 	[92/04/22            rwd]
 * 
 * Revision 2.1  92/04/21  17:27:41  rwd
 * BSDSS
 * 
 *
 */

/*
 * Setup emulator entry points.
 */
#include <mach.h>
#include <mach/message.h>
#include "emul_stack.h"

/*
 * Initialize emulator.  Our bootstrap port is the BSD request port.
 */
mach_port_t	our_bsd_server_port;

emul_stack_t
emul_init()
{
	emul_stack_t stack;

	/*
	 * Bootstrap port is the request port.
	 */
	(void) task_get_bootstrap_port(mach_task_self(), &our_bsd_server_port);

	/*
	 * Set up emulator stack support.
	 */
	stack = emul_stack_init();

#ifdef ECACHE
	/*
	 * Setup cache.
	 */
	emul_cache_init();
#endif ECACHE

#ifdef MAP_UAREA
	/*
	 * Setup mapped area.
	 */
	emul_mapped_init();
#endif MAP_UAREA

	return stack;
}

main(argc, argv)
	int	argc;
	char	**argv;
{
	emul_stack_t stack;

	/*
	 * Initialize the emulator.
	 */
	stack = emul_init();

	/*
	 * Set up the emulator vectors.
	 */
	emul_setup(mach_task_self());

	/*
	 * It is now OK to use the emulator stack reply port
	 */
	mig_init(stack);

	/*
	 * Don't on your life try to use a mig stub here without
	 * specifying the reply_port.  The stack has not been switched
	 * yet, and it won't work.  The emul_execve call results in the
	 * desired stack switch occuring (though it itself never returns)
	 */

	/*
	 * Exec the first program.
	 */
	return (emul_execve(argv[0], argv, (char **)0));
}


/*
 * Initialize emulator on child branch of fork.
 */
void
child_init()
{
	emul_stack_t stack;

	/*
	 * Re-initialize ALL of the emulator - saved
	 * port numbers are no longer valid, and
	 * we have a new request port.
	 */
	mach_init();		/* reset mach_task_self_ ! */
	stack = emul_init();

	/*
	 * It is now OK to use the emulator stack reply port
	 */
	mig_init(stack);
}

/*
 * Fail in an interesting, easy-to-debug way.
 */
void
emul_panic(msg)
	char *msg;
{
	for (;;) {
#ifdef	i386
		asm("int3");
#else	i386
		task_suspend(mach_task_self());
#endif	i386
		(void) mach_msg_trap((mach_msg_header_t *) msg,
				     MACH_MSG_OPTION_NONE, /* do nothing */
				     0, 0, MACH_PORT_NULL,
				     MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	}
}
