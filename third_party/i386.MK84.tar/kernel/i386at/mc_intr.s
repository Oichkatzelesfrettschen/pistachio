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
 * $Log:	mc_intr.s,v $
 * Revision 2.2  93/11/17  16:51:53  dbg
 * 	Created.
 * 	[93/02/10            dbg]
 * 
 */

/*
 *	Quick assembler clock interrupt routine for MC clock chip.
 *	Reads register C to ack interrupt, then jumps to hardclock,
 *	to take system clock interrupt.
 */

	.globl	_mc_sysintr
_mc_sysintr:
	movb	$0x0c, %al		/* MC reg C */
	outb	%al, $0x70		/* output address */
	jmp	0f			/* delay */
0:
	inb	$0x71, %al		/* read to ack */
	jmp	_hardclock		/* do hardclock processing */

/*
 *	Same interrupt routine, but for a standard (non-system)
 *	clock interrupt.
 */

	.globl	_mc_intr
_mc_intr:
	movb	$0x0c, %al		/* MC reg C */
	outb	%al, $0x70		/* output address */
	jmp	0f			/* delay */
0:
	inb	$0x71, %al		/* read to ack */
	movl	$_mc_clock0,4(%esp)	/* pass clock as parameter */
	jmp	_clock_interrupt	/* to clock interrupt routine */
