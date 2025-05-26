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
 * $Log: psl.h,v $
 */
/*
 * 	File: ns532/psl.h
 *	Author: Jukka Virtanen, Helsinki University of Technology 1992.
 */

#define PSR_C		0x0001	/* Carry bit */
#define	PSR_T		0x0002	/* trace enable bit */
#define	PSR_L		0x0004	/* Comparison (unsigned int): op1 > op2 -> 1 */

/* User mode bits */
#define	PSR_V		0x0010	/* Integer overflow bit for OVF instruction */
#define	PSR_F		0x0020	/* Flag bit */
#define	PSR_Z		0x0040	/* Comparison: Operands are equal -> 1 */
#define	PSR_N		0x0080	/* Comparison (signed int): op1 > op2 -> 1 */

/* Supervisor mode bits, accessible in supervisor mode only  */
#define	PSR_U		0x0100	/* Superwiser mode, when 1 */
#define	PSR_S		0x0200	/* Supervisor (0) /User stack (1) select*/
#define	PSR_P		0x0400	/* Trace pending */
#define	PSR_I		0x0800	/* Accept interrupts */
#define PSR_I_BIT_NUMBER 11	/* Accept interrupts, bit position */

#define PSR_USER	0x00ff	/* User mode accessible bits */
#define PSR_SUPER	0xffff	/* Kernel mode accessible bits */

/* Incidently, this works only on supervisor mode :-) */
#define USERMODE(ps)	((ps) & PSR_U)
