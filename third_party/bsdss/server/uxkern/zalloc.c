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
 * $Log:	zalloc.c,v $
 * Revision 2.1  92/04/21  17:11:20  rwd
 * BSDSS
 * 
 *
 */

#include <sys/zalloc.h>
#include <uxkern/import_mach.h>

zone_t		zone_zone;	/* this is the zone containing other zones */

boolean_t	zone_ignore_overflow = FALSE;

vm_offset_t	zdata;
vm_size_t	zdata_size;

#define	lock_zone(zone)		mutex_lock(&zone->lock)

#define	unlock_zone(zone)	mutex_unlock(&zone->lock)

#define	lock_zone_init(zone)	mutex_init(&zone->lock)

/*
 *	Initialize the "zone of zones."
 */
void zone_init()
{
	vm_offset_t	p;

	zdata_size = round_page(64 * sizeof(struct zone));
	(void) vm_allocate(mach_task_self(), &zdata, zdata_size, TRUE);
#if	0
	(void) vm_pageable(mach_task_self(), zdata, zdata_size,
			   VM_PROT_READ|VM_PROT_WRITE);
#endif

	zone_zone = ZONE_NULL;
	zone_zone = zinit(sizeof(struct zone), sizeof(struct zone), 0,
					FALSE, "zones");
	p = (vm_offset_t)(zone_zone + 1);
	zcram(zone_zone, p, (zdata + zdata_size) - p);
}

/*
 *	zinit initializes a new zone.  The zone data structures themselves
 *	are stored in a zone, which is initially a static structure that
 *	is initialized by zone_init.
 */
zone_t zinit(size, max, alloc, pageable, name)
	vm_size_t	size;		/* the size of an element */
	vm_size_t	max;		/* maximum memory to use */
	vm_size_t	alloc;		/* allocation size */
	boolean_t	pageable;	/* is this zone pageable? */
	char		*name;		/* a name for the zone */
{
	register zone_t		z;

	if (zone_zone == ZONE_NULL)
		z = (zone_t) zdata;
	else if ((z = (zone_t) zalloc(zone_zone)) == ZONE_NULL)
		return(ZONE_NULL);

	/*
	 *	Round off all the parameters appropriately.
	 */

	if ((max = round_page(max)) < (alloc = round_page(alloc)))
		max = alloc;

	z->free_elements = 0;
	z->cur_size = 0;
	z->max_size = max;
	z->elem_size = size;
	z->alloc_size = alloc;
	z->pageable = pageable;
	z->zone_name = name;
	z->count = 0;
	z->doing_alloc = FALSE;
	z->exhaustible = z->sleepable = FALSE;
	lock_zone_init(z);
	return(z);
}

/*
 *	Cram the given memory into the specified zone.
 */
void zcram(zone, newmem, size)
	register zone_t		zone;
	vm_offset_t		newmem;
	vm_size_t		size;
{
	register vm_size_t	elem_size;

	elem_size = zone->elem_size;

	lock_zone(zone);
	zone->cur_size += size;
	while (size >= elem_size) {
		ADD_TO_ZONE(zone, newmem);
		zone->count++;	/* compensate for ADD_TO_ZONE */
		size -= elem_size;
		newmem += elem_size;
	}
	unlock_zone(zone);
}

/*
 *	zalloc returns an element from the specified zone.
 */
vm_offset_t zalloc(zone)
	register zone_t	zone;
{
	register vm_offset_t	addr;

	if (zone == ZONE_NULL)
		panic ("zalloc: null zone");

	lock_zone(zone);
	REMOVE_FROM_ZONE(zone, addr, vm_offset_t);
	while (addr == 0) {
		/*
		 *	If nothing was there, try to get more
		 */
		vm_offset_t	alloc_addr;

		if ((zone->cur_size + zone->alloc_size) > zone->max_size) {
			if (zone->exhaustible)
				break;

			if (!zone_ignore_overflow) {
				printf("zone \"%s\" empty.\n", zone->zone_name);
				panic("zalloc");
			}
		}
		unlock_zone(zone);

		if (vm_allocate(mach_task_self(),
				&alloc_addr,
				zone->alloc_size,
				TRUE)
			!= KERN_SUCCESS) {
		    if (zone->exhaustible)
			break;
		    panic("zalloc");
		}
#if	0
		if (!zone->pageable) {
		    (void) vm_pageable(mach_task_self(),
					alloc_addr,
					zone->alloc_size,
					VM_PROT_READ|VM_PROT_WRITE);
		}
#endif
		zcram(zone, alloc_addr, zone->alloc_size);

		lock_zone(zone);

		REMOVE_FROM_ZONE(zone, addr, vm_offset_t);
	}

	unlock_zone(zone);
	return(addr);
}

/*
 *	zget returns an element from the specified zone
 *	and immediately returns nothing if there is nothing there.
 *
 *	This form should be used when you can not block (like when
 *	processing an interrupt).
 */
vm_offset_t zget(zone)
	register zone_t	zone;
{
	register vm_offset_t	addr;

	if (zone == ZONE_NULL)
		panic ("zalloc: null zone");

	lock_zone(zone);
	REMOVE_FROM_ZONE(zone, addr, vm_offset_t);
	unlock_zone(zone);

	return(addr);
}

void zfree(zone, elem)
	register zone_t	zone;
	vm_offset_t	elem;
{
	lock_zone(zone);
	ADD_TO_ZONE(zone, elem);
	unlock_zone(zone);
}


void zchange(zone, pageable, sleepable, exhaustible)
	zone_t		zone;
	boolean_t	pageable;
	boolean_t	sleepable;
	boolean_t	exhaustible;
{
	zone->pageable = pageable;
	zone->sleepable = sleepable;
	zone->exhaustible = exhaustible;
}
