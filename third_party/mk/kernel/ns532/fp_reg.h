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
 * $Log: fp_reg.h,v $
 */
/*
 * 	File: 
 *	Author: Tatu Ylonen, Johannes Helander
 *	Helsinki University of Technology 1992.
 */

#ifndef	_NS532_FP_SAVE_H_
#define	_NS532_FP_SAVE_H_

#define NS532_FP_NO   	0
#define NS532_FP_NS081 	(0x32081)
#define NS532_FP_NS381 	(0x32381)
#define NS532_FP_NS581 	(0x32581)

/* XXXX Assumed to be the same as ns532_float_state (bcopied) */
struct ns532_fp_state {
  	int	     fp_kind;	/* thread's fp kind (see ns532/fpu.c) */
	int	     valid;	/* boolean (false if fpu has not been used) */
	unsigned int fsr;
	unsigned int 	l0a,l0b,l2a,l2b,l4a,l4b,l6a,l6b,
			l1a,l1b,l3a,l3b,l5a,l5b,l7a,l7b;
};

/*
 * Control register
 */
#define FPC_RMB		0x00010000	/* register modify bit */
#define FPC_SWF		0x0000fe00	/* reserved for software */
#define FPC_RM		0x00000180	/* rounding mode */
#define FPC_RM_NEAREST	0x00000000	/* round to nearest */
#define FPC_RM_TOZERO	0x00000080	/* round towards zero */
#define FPC_RM_TOPOS	0x00000100	/* round towards +infinity */
#define FPC_RM_TONEG	0x00000180	/* round towards -infinity */
#define FPC_IF		0x00000040	/* inexact result flag */
#define FPC_IEN		0x00000020	/* inexact result trap enable */
#define FPC_UF		0x00000010	/* underflow flag */
#define FPC_UEN		0x00000008	/* underflow trap enable */
#define FPC_TT		0x00000007	/* trap type */
#define FPC_TT_NONE	0x00000000	/* no exceptional condition */
#define FPC_TT_UNDFL	0x00000001	/* underflow */
#define FPC_TT_OVFL	0x00000002	/* overflow */
#define FPC_TT_DIV0	0x00000003	/* divide by zero */
#define FPC_TT_ILL	0x00000004	/* illegal instruction */
#define FPC_TT_INVOP	0x00000005	/* invalid operation */
#define FPC_TT_INEXACT	0x00000006	/* inexact result */

#endif	_NS532_FP_SAVE_H_
