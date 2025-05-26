/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	clockpitintr.s,v $
 * Revision 2.2  93/11/17  16:42:21  dbg
 * 	Created.
 * 	[93/02/10            dbg]
 * 
 */

/*
 *	Quick assembler clock interrupt routine for PIT clock chip.
 *	Jumps to hardclock to take system clock interrupt.
 */

#include <platforms.h>

	.globl	_clockpitsysintr
_clockpitsysintr:
#ifdef	PS2
	inb	$0x61, %al		/* read port 61 */
	orb	$0x80, %al		/* reset clock interrupt */
	outb	%al, $0x61		/* write back */
#endif
	jmp	_hardclock		/* do hardclock processing */

/*
 *	Same interrupt routine, but for a standard (non-system)
 *	clock interrupt.
 */

	.globl	_clockpitintr
_clockpitintr:
#ifdef	PS2
	inb	$0x61, %al		/* read port 61 */
	orb	$0x80, %al		/* reset clock interrupt */
	outb	%al, $0x61		/* write back */
#endif
	movl	$_clockpit0,4(%esp)	/* pass clock as parameter */
	jmp	_clock_interrupt	/* to clock interrupt routine */
