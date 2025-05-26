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
 * $Log: cswitch.s,v $
 */
/*
 * 	File: ns532/cswitch.s
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 *	Context switch routines for the 32532.
 */

#include <ns532/asm.h>
#include "assym.s"
#include "hexpanel.h"

/*
 * void switch_context(old, continuation, new)
 *	register thread_t	old;
 *	void 			(*continuation)();
 *	register thread_t	new;
 */

ENTRY(Switch_context)
#if HEXPANEL
	movd	1,tos
	bsr	EX(leds_phase)
	movd	1,tos
	movd	LED_F_CSW,tos
	bsr	EX(leds_f)
	adjspd	-12
#endif HEXPANEL
	movd	@_active_stacks, r2		/* get old kernel stack */
	movd	4(sp), r0			/* get old thread */
	movd	r3, KSS_R3(r2)    		/* save registers */
	movd	r4, KSS_R4(r2)
	movd	r5, KSS_R5(r2)
	movd	r6, KSS_R6(r2)
	movd	r7, KSS_R7(r2)
	sprd	fp, KSS_FP(r2)

	movd	tos, KSS_PC(r2)			/* pop and save return PC */
	sprd	sp, KSS_SP(r2)			/* save SP */
	movd	r2, TH_KERNEL_STACK(r0)
	movd 	4(sp), TH_SWAP_FUNC(r0)		/* get and save continuation */
	movd	8(sp), r1			/* get new thread */

	movd	r1, @_active_threads		/* new thread is active */
	movd	TH_KERNEL_STACK(r1), r2		/* get kernel stack */
	movd 	r2, @_active_stacks
	lprd	sp, KSS_SP(r2)    		/* switch stacks */

	sprd	cfg, r3				/* invalidate FPU */
	bicd	2, r3				/* clear F (float) bit */
	lprd	cfg, r3

#if HEXPANEL
	movd	r0, tos
	movd	r2, tos
	movd	2, tos
	movd	LED_F_CSW, tos
	bsr	EX(leds_f)
	cmpd	tos, tos
	movd	tos, r2
	movd	tos, r0
#endif HEXPANEL

	lprd	fp, KSS_FP(r2)
	movd	KSS_R7(r2), r7
	movd	KSS_R6(r2), r6
	movd	KSS_R5(r2), r5
	movd	KSS_R4(r2), r4
	movd	KSS_R3(r2), r3
	movd	KSS_PC(r2), tos			/* push return PC on stack */
	ret	0				/* return old thread */

ENTRY(load_context)
#if HEXPANEL
	movd	2,tos
	bsr	EX(leds_phase)
	movd	8,tos
	movd	LED_F_CSW,tos
	bsr	EX(leds_f)
	adjspd	-12
#endif HEXPANEL
	movd	S_ARG0,r0			/* get thread */
	movd	TH_KERNEL_STACK(r0),r0		/* get stack */
	movd	r0, @_active_stacks
	lprd	sp,KSS_SP(r0)			/* switch stacks */

	sprd	cfg, r3				/* invalidate FPU */
	bicd	2, r3				/* clear F (float) bit */
	lprd	cfg, r3

	movd	KSS_R3(r0),r3
	movd	KSS_R4(r0),r4
	movd	KSS_R5(r0),r5
	movd	KSS_R6(r0),r6
	movd	KSS_R7(r0),r7
	lprd	fp,KSS_FP(r0)
	movd	KSS_PC(r0),tos
#if HEXPANEL
	movd	9,tos
	movd	LED_F_CSW,tos
	bsr	EX(leds_f)
	cmpd	tos, tos
#endif HEXPANEL
	movqd	0, r0				/* return zero */
	ret 	0

ENTRY(Thread_continue)
	lprd	fp, 0				/* zero fp */
	movd	r0, tos				/* push thread argument */
#if HEXPANEL
	movd	3,tos
	bsr	EX(leds_phase)
	cmpqd	0, tos
#endif HEXPANEL
	jsr	0(r3)				/* call real continuation */
