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
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: simple_spl.c,v $
 */
/*
 * 	File: ns532/simple_spl.c
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 */

#include <ns532/psl.h>
#include <ns532/ipl.h>
#include <ns532/pic.h>
#include <mach/boolean.h>

extern volatile unsigned int 	curr_ipl, curr_pic_mask;
extern volatile char 	       *pic_addr;

extern unsigned int		pic_mask[];

volatile boolean_t 	dotimein = FALSE;

/* PSR_I == 0x0800  */
#define SPLON()  asm("bispsrw 0x800");
#define SPLOFF() asm("bicpsrw 0x800");


void primitive_splx(), do_do_softclock();
unsigned int primitive_spl(), primitive_spln(), primitive_spl0();
unsigned int primitive_splhi();

unsigned int
primitive_spl_with_check(ipl)
	register unsigned int ipl;
{
	if ((ipl < SPL0) || (ipl > SPL7))
	    panic ("primitive_spl: Bad IPL value");

	if (ipl < curr_ipl)
	    panic ("primitive_spl: Invalid lowering of IPL");

	if (ipl >= SPLHI)
	    return primitive_splhi();
	else if (ipl <= SPL0)
	    return primitive_spl0();
	else
	    return primitive_spln(ipl);
}

void
primitive_splx_with_check(ipl)
	register unsigned int ipl;
{
	if ((ipl < SPL0) || (ipl > SPL7))
	    panic ("primitive_splx: Bad IPL value");

	if (ipl > curr_ipl)
	    panic ("primitive_splx: Invalid raising of IPL");
	
	primitive_splx(ipl);
}

unsigned int
primitive_splhi()
{
	register unsigned int old_ipl;

	SPLOFF();

	old_ipl = curr_ipl;
	curr_ipl = SPLHI;

	/* for splhi just leave interrupts disabled */
	return old_ipl;
}

unsigned int
primitive_spln(ipl)
	register unsigned int ipl;
{
	register unsigned int old_ipl, mask;

	SPLOFF();

	old_ipl = curr_ipl;
	curr_ipl = ipl;

	mask = pic_mask[ipl];

	pic_addr[ICU_IMSK] = mask;
	pic_addr[ICU_IMSK+1] = mask >> 8;

	SPLON();

	return old_ipl;
}

unsigned int
primitive_spl0()
{
	register unsigned int old_ipl, mask;

	SPLOFF();

	old_ipl = curr_ipl;
	curr_ipl = SPL0;

	/* at SPL0 check for softclock */
	if (dotimein) {
		do_do_softclock();
		return old_ipl;
		/* SPLON in subroutine */
	}
	mask = pic_mask[SPL0];

	pic_addr[ICU_IMSK] = mask;
	pic_addr[ICU_IMSK+1] = mask >> 8;

	SPLON();

	return old_ipl;
}

void
primitive_splx(ipl)
	register int ipl;
{
	register unsigned int mask;

	SPLOFF();

	curr_ipl = ipl;

	/* for splhi just leave interrupts disabled */
	if (ipl >= SPL7)
	    return;

	/* at SPL0 check for softclock */
	if ((ipl <= SPL0) && dotimein) {
		do_do_softclock();
		/* SPLON in subroutine */
		return;
	}
	mask = pic_mask[ipl];

	pic_addr[ICU_IMSK] = mask;
	pic_addr[ICU_IMSK+1] = mask >> 8;

	SPLON();

	return;
}

/* Preconditions: SPLOFF */
void
do_do_softclock()
{
	register unsigned int mask;

	/* raise to SPL1 */
	mask = pic_mask[SPL1];
	curr_ipl = SPL1;
	pic_addr[ICU_IMSK] = mask;
	pic_addr[ICU_IMSK+1] = mask >> 8;
	SPLON();

	softclock();

	/* lower to SPL0 */
	SPLOFF();
	mask = pic_mask[SPL0];
	curr_ipl = SPL0;
	pic_addr[ICU_IMSK] = mask;
	pic_addr[ICU_IMSK+1] = mask >> 8;
	SPLON();
}

setsoftclock()
{
	dotimein = TRUE;
}
