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
 * $Log: setjmp.s,v $
 */
/*
 * 	File: ns532/setjmp.s
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 */

/*
 * C library -- _setjmp, _longjmp
 *
 *	_longjmp(a,v)
 * will generate a "return(v)" from
 * the last call to
 *	_setjmp(a)
 * by restoring registers from the stack,
 * The previous signal state is NOT restored.
 *
 */

#include <ns532/asm.h>

	.text
ENTRY(_setjmp)
	movd	tos,r1			/* pc of caller */
	movd	tos,r0			/* buffer argument */
	movd	r0,tos			/* called expects to remove it! */
	movd	r3,0(r0)
	movd	r4,4(r0)
	movd	r5,8(r0)
	movd	r6,12(r0)
	movd	r7,16(r0)
	sprd	sp,20(r0)
	sprd	fp,24(r0)
	movd	r1,28(r0)
	movqd	0,r0			/* return 0 */
        jump	0(r1)

ENTRY(_longjmp)
	movd	S_ARG0,r1		/* buffer */
	movd	S_ARG1,r0		/* return value */
	movd	0(r1),r3
	movd	4(r1),r4
	movd	8(r1),r5
	movd	12(r1),r6
	movd	16(r1),r7
	lprd	fp,24(r1)
	movd	28(r1),r2
	lprd	sp,20(r1)
	cmpqd	0,r0
	bne	L0
	movqd	1,r0
L0:	jump	0(r2)
