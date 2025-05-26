/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * Copyright (c) 1992 Helsinki University of Technology
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND HELSINKI UNIVERSITY OF TECHNOLOGY ALLOW FREE USE
 * OF THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * HELSINKI UNIVERSITY OF TECHNOLOGY DISCLAIM ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * 11-May-92  Tatu Ylonen (ylo) at Helsinki University of Technology
 *	Created.
 *
 * $Log: setjmp.h,v $
 */
/*
 * 	File: ns532/setjmp.h
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 *
 *	Setjmp/longjmp buffer for NS532.
 */

#ifndef	_NS532_SETJMP_H_
#define	_NS532_SETJMP_H_

/*
 * Note: This is a completely different jmpbuf from that used in user progs.
 *       do not confuse. 
 *
 * XXX   Must agree with struct ns532_kernel_state  XXX 
 */
typedef	struct jmp_buf {
	int	jmp_buf[8]; /* r3,r4,r5,r6,r7,sp,fp,pc */
} jmp_buf_t;

#endif	/* _NS532_SETJMP_H_ */
