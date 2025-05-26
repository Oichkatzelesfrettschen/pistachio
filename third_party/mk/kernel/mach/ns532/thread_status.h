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
 * $Log:$
 */
/*
 *	File:	thread_status.h
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 *	This file contains structure definitions for the thread
 *	state as applied to ns532 processors and ns[035]81 FPUs.
 */

#ifndef	_MACH_NS532_THREAD_STATUS_H_
#define _MACH_NS532_THREAD_STATUS_H_

/*
 *	ns532_xxx_state		these are the structures that are exported
 *				to user threads for use in status/mutate
 *				calls.  These structures should never
 *				change.
 *
 * 	ns532_thread_state	CPU state
 *	ns532_float_state	FPU state
 *	ns532_combined_state	CPU + FPU states in one struct.
 */

#define NS532_THREAD_STATE	(0x32532b)
#define NS532_FLOAT_STATE	(0x32532f)
#define NS532_COMBINED_STATE	(0x32532c)

struct ns532_thread_state {
  unsigned int r0,r1,r2,r3,r4,r5,r6,r7;
  unsigned int sb, fp, sp, pc;
  unsigned int mod, psr;
};

#define NS532_THREAD_STATE_COUNT	(sizeof(struct ns532_thread_state)/sizeof(unsigned int))

/* XXX Should agree with ns532_fp_state (ns532/fp_reg.h). bcopied in kernel */

struct ns532_float_state {
	int		fp_kind; /* NS532_FP_NO, NS532_FP_NS081, ... */
	int		valid; 
	unsigned int 	fsr;
	unsigned int 	l0a,l0b,l2a,l2b,l4a,l4b,l6a,l6b,
			l1a,l1b,l3a,l3b,l5a,l5b,l7a,l7b;
};

#define NS532_FLOAT_STATE_COUNT (sizeof(struct ns532_float_state) / sizeof(unsigned int))

struct ns532_combined_state {
  	struct ns532_thread_state ts;
	struct ns532_float_state  fs;
};

#define NS532_COMBINED_STATE_COUNT (sizeof(struct ns532_combined_state) / sizeof(unsigned int))

#endif	_MACH_NS532_THREAD_STATUS_H_
