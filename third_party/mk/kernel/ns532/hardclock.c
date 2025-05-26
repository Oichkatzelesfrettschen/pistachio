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
 * 11-May-92  Tero Kivinen (kivinen) at Helsinki University of Technology
 *	Created.
 *
 * $Log: hardclock.c,v $
 */
/*
 * 	File: ns532/hardclock.c
 *	Author: Tero Kivinen, Tatu Ylonen
 *	Helsinki University of Technology 1992.
 *
 * Clock interrupt.
 */
#include <kern/time_out.h>
#include <ns532/ipl.h>
#include <ns532/psl.h>
#include <ns532/thread.h>

#include "hexpanel.h"
#include "idleleds.h"

extern void	clock_interrupt();

hardclock(ivect, old_ipl, regs)
	int	ivect;			   /* interrupt number */
	int	old_ipl;		   /* old interrupt level */
        struct ns532_saved_state *regs;	   /* pointer to saved state */
{
#if HEXPANEL
	leds_hardclock((regs->psr & PSR_U) != 0);
#endif	HEXPANEL
#if IDLELEDS
	show_idle();
#endif
	clock_interrupt(tick,	                   /* usec per clock tick */
			(regs->psr & PSR_U) != 0,  /* from user mode? */
			old_ipl == SPL0);          /* base priority */
}
