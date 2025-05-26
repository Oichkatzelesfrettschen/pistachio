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
 * $Log:	netisr.c,v $
 * Revision 2.1  92/04/21  17:14:10  rwd
 * BSDSS
 * 
 *
 */

#include <iso.h>
#include <ns.h>
#include <inet.h>
#include <imp.h>

#include <sys/param.h>
#include <sys/proc.h>
#include <net/netisr.h>

int		netisr = 0;
struct mutex	netisr_mutex = MUTEX_INITIALIZER;

/*
 * Must be called at splnet.
 */
void dosoftnet()
{
    struct proc *p = (struct proc *)cthread_data(cthread_self());
    int original_master = p->p_master_lock;

    mutex_lock(&netisr_mutex);

#if	NIMP > 0
	if (netisr & (1<<NETISR_IMP)) {
	    netisr &= ~(1<<NETISR_IMP);
	    mutex_unlock(&netisr_mutex);
	    impintr();
	    mutex_lock(&netisr_mutex);
	}
#endif	NIMP > 0
#if	INET
	if (netisr & (1<<NETISR_IP)) {
	    netisr &= ~(1<<NETISR_IP);
	    mutex_unlock(&netisr_mutex);
	    ipintr();
	    mutex_lock(&netisr_mutex);
	}
#endif	INET
#if	ISO
	if (netisr & (1<<NETISR_ISO)) {
	    netisr &= ~(1<<NETISR_ISO);
	    mutex_unlock(&netisr_mutex);
	    isointr();
	    mutex_lock(&netisr_mutex);
	}
#endif	ISO
#if	CCITT
	if (netisr & (1<<NETISR_CCITT)) {
	    netisr &= ~(1<<NETISR_CCITT);
	    mutex_unlock(&netisr_mutex);
	    ccittintr();
	    mutex_lock(&netisr_mutex);
	}
#endif	CCITT
#if	NS
	if (netisr & (1<<NETISR_NS)) {
	    netisr &= ~(1<<NETISR_NS);
	    mutex_unlock(&netisr_mutex);
	    nsintr();
	    mutex_lock(&netisr_mutex);
	}
#endif	NS
	if (netisr & (1<<NETISR_RAW)) {
	    netisr &= ~(1<<NETISR_RAW);
	    mutex_unlock(&netisr_mutex);
	    rawintr();
	}
	else {
	    mutex_unlock(&netisr_mutex);
	}

	if (original_master != p->p_master_lock)
		panic("dosoftnet");
}
