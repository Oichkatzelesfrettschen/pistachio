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
 * $Log:	syscall_table.h,v $
 * Revision 2.1  92/04/21  17:27:31  rwd
 * BSDSS
 * 
 *
 */

/*
 * Definition of system call table.
 */
struct sysent {
	int	nargs;		/* number of arguments, or special code */
	int	(*routine)();
};

/*
 * Special arguments:
 */
#define	E_GENERIC	(-1)
				/* no specialized routine */
#define	E_CHANGE_REGS	(-2)
				/* may change registers */

/*
 * Exported system call table
 */
extern struct sysent	sysent[];	/* normal system calls */
extern int		nsysent;


extern struct sysent	sysent_task_by_pid;
extern struct sysent	sysent_pid_by_task;
extern struct sysent	sysent_htg_ux_syscall;
extern struct sysent	sysent_init_process;
