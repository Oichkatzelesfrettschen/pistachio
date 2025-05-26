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
 * $Log:	allocator.c,v $
 * Revision 2.3  92/07/08  16:11:30  mrt
 * 	Renamed local version of vm_map to em_vm_map so it wouldn't get
 * 	confused with the prototype of the kernel's vm_map which has
 * 	different arguments.
 * 	[92/07/06            mrt]
 * 
 * Revision 2.2  92/04/22  14:00:47  rwd
 * 	Fix mach.h include.
 * 	[92/04/22            rwd]
 * 
 * Revision 2.1  92/04/21  17:27:33  rwd
 * BSDSS
 * 
 *
 */

/*
 * Replacement for vm_allocate, to keep memory within emulator address space.
 */

#include <mach.h>
#include <machine/param.h>

#ifdef	mips
#define VM_PROT_DEFAULT (VM_PROT_READ|VM_PROT_WRITE)
#endif	mips

extern mach_port_t mig_get_reply_port();

kern_return_t
vm_allocate(task, addr, size, anywhere)
	task_t		task;
	vm_offset_t	*addr;
	vm_size_t	size;
	boolean_t	anywhere;
{
	if (anywhere)
	    *addr = EMULATOR_BASE;

	return (htg_vm_map(task, mig_get_reply_port(),
			addr, size, (vm_offset_t)0, anywhere,
			MEMORY_OBJECT_NULL, (vm_offset_t)0, FALSE,
			VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_COPY));
}

kern_return_t
em_vm_map(task, reply_port, address, size, mask, anywhere,
	memory_object, offset, copy,
	cur_protection, max_protection, inheritance)

	vm_task_t	task;
	mach_port_t	reply_port;
	vm_address_t	*address;
	vm_size_t	size;
	vm_address_t	mask;
	boolean_t	anywhere;
	memory_object_t	memory_object;
	vm_offset_t	offset;
	boolean_t	copy;
	vm_prot_t	cur_protection;
	vm_prot_t	max_protection;
	vm_inherit_t	inheritance;
{
	if (anywhere && *address < EMULATOR_BASE)
	    *address = EMULATOR_BASE;

	return (htg_vm_map(task, reply_port, address, size, mask, anywhere,
			memory_object, offset, copy,
			cur_protection, max_protection, inheritance));
}
