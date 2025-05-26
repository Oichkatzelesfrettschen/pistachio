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
 * $Log:	synch.h,v $
 * Revision 2.1  92/04/21  17:15:26  rwd
 * BSDSS
 * 
 *
 */

/*
 * SPL level definitions.
 */
#define	SPL_COUNT	6

#define	SPL0		0
				/* placeholder - not used */
#define	SPLSOFTCLOCK	1
				/* level '0x8' */
#define	SPLNET		2
				/* level '0xc' */
#define	SPLTTY		3
				/* level '0x15' */
#define	SPLBIO		3
				/* level '0x15' */
#define	SPLIMP		4
				/* level '0x16' */
#define	SPLHIGH		5
				/* level '0x18' */


#define	spl0()		(spl_n(SPL0))
#define	splsoftclock()	(spl_n(SPLSOFTCLOCK))
#define	splnet()	(spl_n(SPLNET))
#define	splbio()	(spl_n(SPLBIO))
#define	spltty()	(spl_n(SPLTTY))
#define	splimp()	(spl_n(SPLIMP))
#define	splhigh()	(spl_n(SPLHIGH))
#define	splx(s)		(spl_n(s))
#define splsched()	(spl_n(SPLHIGH))
#define splvm()		(spl_n(SPLIMP))

extern int	spl_n();
extern void	interrupt_enter();
extern void	interrupt_exit();
