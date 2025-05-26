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
 * $Log:	hardclock.s,v $
 * Revision 2.2  93/11/17  16:35:45  dbg
 * 	Created.
 * 	[93/02/10            dbg]
 * 
 */

#include <platforms.h>

#include <i386/eflags.h>
#include <assym.s>

/*
 *	Assembler version of system clock interrupt
 *	routine.
 *
 * On stack:
 * esp->	return address to interrupt dispatcher
 *		unit number (unneeded on return)
 * [ PS2 only	interrupt vector number ]
 *		saved SPL
 *		return address from interrupt dispatcher
 *		pointer to saved registers
 */

#ifdef	PS2
#define	RET_OFFSET	16
#else	/* not PS2 */
#define	RET_OFFSET	12
#endif

	.globl	_hardclock
_hardclock:
	movl	RET_OFFSET(%esp),%eax	/* get dispatcher return address */
	cmpl	$_return_to_iret,%eax	/* from interrupt level? */
	jne	hardclock_system	/* in kernel if so */
	movl	RET_OFFSET+4(%esp),%eax	/* point to registers */
	testl	$(EFL_VM),I_EFL(%eax)	/* in V86 mode? */
	jnz	hardclock_user		/* in user if so */
	testb	$3,I_CS(%eax)		/* protection level 3? */
	jnz	hardclock_user		/* in user if so */

/* clock interrupt from system */

hardclock_system:
	movl	$0,4(%esp)		/* set usermode = FALSE */
	jmp	_sys_clock_interrupt	/* take system clock interrupt */

/* clock interrupt from user */

hardclock_user:
	movl	$1,4(%esp)		/* set usermode = TRUE */
	jmp	_sys_clock_interrupt	/* take system clock interrupt */
