/*
 * Mach Operating System
 * Copyright (c) 1993-1992 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON AND OMRON ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND OMRON DISCLAIM ANY LIABILITY OF ANY KIND
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
 * Motorola 88100 pcb definitions 
 *
 */
/*
 * HISTORY
 * $Log:	pcb.h,v $
 * Revision 2.5  93/11/17  17:57:12  dbg
 * 	Added INTERRUPTED_PC.  Removed prototypes that are already
 * 	present in other files.
 * 	[93/10/14            dbg]
 * 
 * Revision 2.4  93/01/26  18:01:11  danner
 * 	Cleaned up includes, added pcb.h prototypes.
 * 	[93/01/25            jfriedl]
 * 
 * Revision 2.3  92/03/03  14:22:28  rpd
 * 	Added dummy definition of syscall_emulation_sync.
 * 	[92/03/03            rpd]
 * 
 * Revision 2.2  92/02/18  18:03:49  elf
 * 	Disabled KEEP_STACKS.
 * 	[92/01/30            danner]
 * 	Created.
 * 	[92/01/20            danner]
 * 
 */
#ifndef	_M88100_THREAD_H_
#define	_M88100_THREAD_H_

#include <mach/m88k/vm_types.h>	/* vm_offset_t */
#include <mach/kern_return.h>	/* kern_return_t */
#include <mach/thread_status.h> /* thread_state_t */
#include <kern/kern_types.h>	/* thread_t */
#include <kern/sched_prim.h>	/* continuation_t */

/* This structure is used to save the kernel state during a save_context
   or other handoff. It was the original pcb in the impure kernel */

/* This must always be an even number of words long */

struct m88100_pcb {
    unsigned pcb_pc;	/* address to return */
    unsigned pcb_r14;
    unsigned pcb_r15;
    unsigned pcb_r16;
    unsigned pcb_r17;
    unsigned pcb_r18;
    unsigned pcb_r19;
    unsigned pcb_r20;
    unsigned pcb_r21;
    unsigned pcb_r22;
    unsigned pcb_r23;
    unsigned pcb_r24;
    unsigned pcb_r25;
    unsigned pcb_r26;
    unsigned pcb_r27;
    unsigned pcb_r28;
    unsigned pcb_r29;
    unsigned pcb_r30;
    unsigned pcb_sp; 	/* kernel stack pointer */
    unsigned pcb_mask;	/* to use in save_context() */
};


/*
 *	m88100_saved_state	this structure corresponds to the state
 *				of the user registers as saved on the
 *				stack upon kernel entry.  This structure
 *				is used internally only.  Since this
 *				structure may change from version to
 *				version, it is hidden from the user.
 */

/* This must always be an even number of words long */

struct m88100_saved_state {
    unsigned r[32];
    unsigned fpsr;
    unsigned fpcr;
    unsigned epsr;
    unsigned sxip;
    unsigned snip;
    unsigned sfip;
    unsigned ssbr;
    unsigned dmt0;
    unsigned dmd0;
    unsigned dma0;
    unsigned dmt1;
    unsigned dmd1;
    unsigned dma1;
    unsigned dmt2;
    unsigned dmd2;
    unsigned dma2;
    unsigned fpecr;
    unsigned fphs1;
    unsigned fpls1;
    unsigned fphs2;
    unsigned fpls2;
    unsigned fppt;
    unsigned fprh;
    unsigned fprl;
    unsigned fpit;
    unsigned vector;   /* exception vector number */
    unsigned mask;     /* interrupt mask level */
    unsigned mode;     /* interrupt mode */
    unsigned scratch1; /* used by locore trap handling code */
    unsigned pad;      /* to make an even length */
} ;

struct pcb
{
  struct m88100_pcb            kernel_state;
  struct m88100_saved_state    user_state;
};

typedef	struct pcb	*pcb_t;		/* exported */

/*
 * user's stack pointer is in the PCB, * or in the processor
 * register if the thread is active.
 */


/*
 *	Location of saved user registers for thread.
 */
#define	USER_REGS(thread) \
	(((struct m88100_saved_state *)  (&((thread)->pcb->user_state))))

#define	INTERRUPTED_PC(thread) \
	(USER_REGS(thread)->sxip & ~0x3)	/* lower 2 bits are flags */

#define syscall_emulation_sync(task)	/* do nothing */

/*
 * Prototypes from "motorola/m88k/m88100/pcb.c".
 */
extern void initial_context(thread_t thread);

/*
 * Prototypes for "continuation.s"
 */
extern thread_t Switch_context(
    thread_t oldthread,
    continuation_t continuation,
    thread_t newthread,
    struct m88100_pcb *oldstate,
    struct m88100_pcb *newstate);


/*
 * Prototypes for "exception_return.s"
 */
extern no_return return_from_exception(void);
extern no_return return_from_syscall(void);

/*
 * Prototypes for "misc.s"
 */
unsigned ff1(unsigned value);

#endif	/* _M88100_THREAD_H_ */
