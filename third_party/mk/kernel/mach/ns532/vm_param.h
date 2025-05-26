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
 * 11-May-92  Tatu Ylonen (ylo) at Helsinki University of Technology
 *	Created.
 *
 * $Log:$
 */
/*
 * 	File: mach/ns532/vm_param.h
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 *
 *	32532 machine dependent virtual memory parameters.
 */

#ifndef	_MACH_NS532_VM_PARAM_H_
#define _MACH_NS532_VM_PARAM_H_

#define BYTE_SIZE	8	/* byte size in bits */

#define NS532_PGBYTES	4096	/* bytes per 32532 page */
#define NS532_PGSHIFT	12	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define ns532_btop(x)		(((unsigned)(x)) >> NS532_PGSHIFT)
#define ns532_ptob(x)		(((unsigned)(x)) << NS532_PGSHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define ns532_round_page(x)	((((unsigned)(x)) + NS532_PGBYTES - 1) & \
					~(NS532_PGBYTES-1))
#define ns532_trunc_page(x)	(((unsigned)(x)) & ~(NS532_PGBYTES-1))

/* The ns532 CPU is brain-damaged and it is difficult to generate efficient
 * code for addresses above 0x20000000. 
 *
 * This problem seems to be solved by the new gcc (can't remember version)
 *      by using two instructions for absolute addresses. All the assembly
 *      code needs to be checked though. (The CPU is still brain damaged
 *      anyhow).
 */

#define VM_MIN_ADDRESS		((vm_offset_t) 0)
#define VM_MAX_ADDRESS		((vm_offset_t) 0x18000000)

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t) 0x18000000)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) 0xffffffff)

#define KERNEL_STACK_SIZE	(1*NS532_PGBYTES)
#define INTSTACK_SIZE		(1*NS532_PGBYTES) /* compare with locore.s */
						/* interrupt stack size */

/*
 *	Conversion between 32532 pages and VM pages
 */

#define trunc_ns532_to_vm(p)	(atop(trunc_page(ns532_ptob(p))))
#define round_ns532_to_vm(p)	(atop(round_page(ns532_ptob(p))))
#define vm_to_ns532(p)		(ns532_btop(ptoa(p)))

/*
 *	Physical memory is mapped 1-1 with virtual memory starting
 *	at VM_MIN_KERNEL_ADDRESS.
 */
#define phystokv(a)	((vm_offset_t)(a) + VM_MIN_KERNEL_ADDRESS)

#endif	_MACH_NS532_VM_PARAM_H_
