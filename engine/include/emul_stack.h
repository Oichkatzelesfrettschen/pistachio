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
 * $Log:	emul_stack.h,v $
 * Revision 2.1  92/04/21  17:27:43  rwd
 * BSDSS
 * 
 *
 */

#ifndef	_EMUL_STACK_H_
#define	_EMUL_STACK_H_

/*
 * Top of emulator stack holds link and reply port.
 */
struct emul_stack_top {
	struct emul_stack_top	*link;
	mach_port_t		reply_port;
};
typedef	struct emul_stack_top	*emul_stack_t;

extern emul_stack_t emul_stack_init();
extern emul_stack_t emul_stack_alloc();

#endif	_EMUL_STACK_H_
