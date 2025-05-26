/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990,1989 Carnegie Mellon University
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
 * 14-Apr-93  Ian Dall (ian) at University of Adelaide
 *	Copied from sun3 version.
 *
 * $Log:	vm_types.h,v $
 * Revision 2.5  93/01/14  17:47:38  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 	New (more) fundamental types.
 * 	Standardized conditional that protects against multiple
 * 	inclusions of this file.
 * 	[92/06/03            af]
 * 
 * Revision 2.4  92/02/29  15:33:45  rpd
 * 	Changed vm_offset_t and vm_size_t to unsigned int.
 * 	[92/02/28            rpd]
 * 
 *
 * Condensed history:
 *	Created (jjc).
 */

#ifndef	_MACHINE_VM_TYPES_H_
#define	_MACHINE_VM_TYPES_H_

/*
 *	Header file for VM data types.  Sun version.
 */

#ifdef	ASSEMBLER
#else	/* ASSEMBLER */

/*
 * A natural_t is the type for the native
 * integer type, e.g. 32 or 64 or.. whatever
 * register size the machine has.  Unsigned, it is
 * used for entities that might be either
 * unsigned integers or pointers, and for
 * type-casting between the two.
 * For instance, the IPC system represents
 * a port in user space as an integer and
 * in kernel space as a pointer.
 */
typedef unsigned int	natural_t;

/*
 * An integer_t is the signed counterpart
 * of the natural_t type. Both types are
 * only supposed to be used to define
 * other types in a machine-independent
 * way.
 */
typedef int		integer_t;

/*
 * An int32 is an integer that is at least 32 bits wide
 */
typedef int		int32;
typedef unsigned int	uint32;

/*
 * A vm_offset_t is a type-neutral pointer,
 * e.g. an offset into a virtual memory space.
 */
typedef	natural_t	vm_offset_t;

/*
 * A vm_size_t is the proper type for e.g.
 * expressing the difference between two
 * vm_offset_t entities.
 */
typedef	natural_t	vm_size_t;

#endif	/* ASSEMBLER */
/*
 * If composing messages by hand (please dont)
 */

#define	MACH_MSG_TYPE_INTEGER_T	MACH_MSG_TYPE_INTEGER_32


#endif	/*_MACHINE_VM_TYPES_H_*/
