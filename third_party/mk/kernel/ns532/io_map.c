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
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created from i386/io_map.c.
 *
 * $Log: io_map.c,v $
 */

#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>

extern vm_offset_t	virtual_avail;

/*
 * Allocate and map memory for devices that may need to be mapped before
 * Mach VM is running.
 */
vm_offset_t
io_map(phys_addr, size)
	vm_offset_t	phys_addr;
	vm_size_t	size;
{
	vm_offset_t	start;

	if (kernel_map == VM_MAP_NULL) {
                /*
                 * VM is not initialized.  Grab memory.
                 */
                start = virtual_avail;
                virtual_avail += round_page(size);
        }
        else {
	      (void) kmem_alloc_pageable(kernel_map, &start, round_page(size));
	}	
	/* pmap_map_bd will map the page with cache inhibit set (ns532) */
	(void) pmap_map_bd(start, phys_addr, phys_addr + round_page(size),
			   VM_PROT_READ|VM_PROT_WRITE);
	return (start);
}
