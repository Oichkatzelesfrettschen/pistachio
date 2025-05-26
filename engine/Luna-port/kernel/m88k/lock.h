/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * $Log:	lock.h,v $
 * Revision 2.3  93/11/17  17:37:29  dbg
 * 	Added prototypes for simple_lock functions (machine-dependent).
 * 	[93/10/20            dbg]
 * 
 * Revision 2.2  92/08/03  17:47:56  jfriedl
 * 	Created [danner]
 * 
 */

/*
 * Machine-dependent simple locks.
 */
#ifndef	_MACHINE_LOCK_H_
#define	_MACHINE_LOCK_H_

extern void		simple_lock_init(simple_lock_t);
extern void		simple_lock(simple_lock_t);
extern void		simple_unlock(simple_lock_t);
extern boolean_t	simple_lock_try(simple_lock_t);
extern void		simple_lock_pause(void);

#endif	/* _MACHINE_LOCK_H_ */
