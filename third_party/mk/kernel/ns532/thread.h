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
 * $Log: thread.h,v $
 */
/*
 * 	File: ns532/thread.h
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 *	This file contains the structure definitions for the thread
 *	state as applied to 32532 processors.
 */

#ifndef	_NS532_THREAD_H_
#define _NS532_THREAD_H_

#include <mach/boolean.h>
#include <ns532/fp_reg.h>

/* Check against locore.s if you change this! */

struct ns532_saved_state {
	unsigned int trapno;
	unsigned int msr;		/* last page fault (by cpu) status */
	unsigned int tear;		/* last page fault (by cpu) address */
	unsigned int mod;		/* only low-order 16 bits are used */
	unsigned int sb;     
	unsigned int fp;     
	unsigned int usp;    
	unsigned int r7;     
	unsigned int r6;     
	unsigned int r5;     
	unsigned int r4;     
	unsigned int r3;     
	unsigned int r2;     
	unsigned int r1;     
	unsigned int r0;     
	unsigned int pc;     
	unsigned short reserved2;	/* filler */
	unsigned short psr;		/* MSW (saved here by hardware) */
};

/*
 *      ns532_kernel_state:
 *
 *      This structure corresponds to the state of kernel registers
 *      as saved in a context-switch.  It lives at the base of the stack.
 */

struct ns532_kernel_state {
	unsigned int	k_r3;	/* kernel context */
	unsigned int	k_r4;
	unsigned int	k_r5;
	unsigned int	k_r6;
	unsigned int	k_r7;
	unsigned int	k_sp;
	unsigned int	k_fp;
	unsigned int	k_pc;
};

typedef struct pcb {
        struct ns532_saved_state iss;
	struct ns532_fp_state	*fps;
} *pcb_t;

#define STACK_IKS(stack)	\
	((struct ns532_kernel_state *)((stack) + KERNEL_STACK_SIZE) - 1)

#define STACK_ISS(stack)	\
	((struct ns532_saved_state *)STACK_IKS(stack) - 1)

/*
 *    Note: We are looking at kernel_stack without locking.
 *    This should be OK, because we shouldn't be using USER_REGS
 *    to look at the thread if it isn't quiescent.
 */
#define USER_REGS(thread) 		\
    (((thread)->kernel_stack == 0) ? 	\
     &(thread)->pcb->iss : 		\
     STACK_ISS((thread)->kernel_stack))

#define syscall_emulation_sync(task)	/* do nothing */

#endif	_NS532_THREAD_H_
