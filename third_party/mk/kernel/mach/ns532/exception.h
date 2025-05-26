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
 *	Codes and subcodes for 32532 exceptions.
 */

#ifndef	_MACH_NS532_EXCEPTION_H_
#define _MACH_NS532_EXCEPTION_H_

#define EXC_NS532_NVI		0	/* non-vectored interrupt */
#define EXC_NS532_NMI		1	/* non-maskable interrupt */
#define EXC_NS532_ABT		2	/* abort */
#define EXC_NS532_SLAVE		3	/* coprocessor trap */
#define EXC_NS532_ILL		4       /* illegal operation in user mode */
#define EXC_NS532_SVC		5	/* supervisor call */
#define EXC_NS532_DVZ		6	/* divide by zero */
#define EXC_NS532_FLG		7	/* flag instruction */
#define EXC_NS532_BPT		8	/* breakpoint instruction */
#define EXC_NS532_TRC		9	/* trace trap */
#define EXC_NS532_UND		10	/* undefined instruction */
#define EXC_NS532_RBE		11	/* restartable bus error */
#define EXC_NS532_NBE		12	/* non-restartable bus error */
#define EXC_NS532_OVF		13	/* integer overflow trap */
#define EXC_NS532_DBG		14	/* debug trap */

#endif	_MACH_NS532_EXCEPTION_H_
