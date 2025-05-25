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
 * 02-Aug-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: trap.h,v $
 */
/*
 * 	File: ns532/trap.h
 *	Author: Tatu Ylonen, Johannes Helander
 *	Helsinki University of Technology 1992.
 *
 *	Hardware trap vectors for ns532.
 */

#ifndef _NS532_TRAP_H_
#define _NS532_TRAP_H_

#define T_NVI		0	/* non-vectored interrupt */
#define T_NMI		1	/* non-maskable interrupt */
#define T_ABT		2	/* abort */
#define T_SLAVE		3	/* coprocessor trap */
#define T_ILL		4       /* illegal operation in user mode */
#define T_SVC		5	/* supervisor call */
#define T_DVZ		6	/* divide by zero */
#define T_FLG		7	/* flag instruction */
#define T_BPT		8	/* breakpoint instruction */
#define T_TRC		9	/* trace trap */
#define T_UND		10	/* undefined instruction */
#define T_RBE		11	/* restartable bus error */
#define T_NBE		12	/* non-restartable bus error */
#define T_OVF		13	/* integer overflow trap */
#define T_DBG		14	/* debug trap */
#define T_RESERVED	15	/* reserved */

/* Not a real trap. */
#define T_WATCHPOINT	17	/* watchpoint */

/* signal codes */
#define ILL_PRIVIN_FAULT	T_ILL
#define ILL_RESOP_FAULT		T_UND
#define FPE_INTOVF_TRAP		T_OVF
#define FPE_INTDIV_TRAP		T_DVZ

#endif _NS532_TRAP_H_
