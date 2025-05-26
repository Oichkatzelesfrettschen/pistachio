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
 * 14-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: spl.s,v $
 */
/*
 * 	File: ns532/spl.s
 *	Author: Tatu Ylonen, Johannes Helander
 *	Helsinki University of Technology 1992.
 */

#include <ns532/asm.h>
#include <ns532/ipl.h>
#include <ns532/pic.h>
#include <ns532/psl.h>

/* 
 *	Function: set_spl_mask
 *	
 *	Like splx but a different calling convention. Also doesn't
 *	check for a pending softclock request.
 *	Only used by t_nvi_interrupt in locore.s.
 *
 *	Argument: new spl in r0.
 *	Corrupts: r0, r1.
 *	Returns:  nothing useful.
 */

ENTRY(set_spl_mask)			/* this is much like splx */
	DFRAME
	bicpsrw	PSR_I			/* disable interrupts */
	movd	r0,@EX(curr_ipl)	/* set new level in memory */
	cmpd	SPLHI,r0		/* at splhi, just disable interrupts */
	beq	spl_mask_return
	movd	@EX(pic_mask)[r0:d],r0	/* get hardware interrupt mask */
	movd	@EX(pic_addr),r1
	movb	r0,ICU_IMSK(r1)		/* set icu mask register */
	lshd	-8,r0
	movb	r0,(ICU_IMSK+1)(r1)
spl_mask_return:
	DEMARF
	ret	0
