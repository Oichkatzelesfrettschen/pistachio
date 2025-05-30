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
 * $Log:	port_hash.c,v $
 * Revision 2.1  92/04/21  17:11:00  rwd
 * BSDSS
 * 
 *
 */

/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * $Log:	port_hash.c,v $
 * Revision 2.1  92/04/21  17:11:00  rwd
 * BSDSS
 * 
 * Revision 3.0  91/01/17  12:06:12  condict
 * Unchanged copy from Mach 3.0 BSD UNIX server
 * 
 * Revision 2.2  90/06/02  15:28:05  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  20:20:42  rpd]
 * 
 */

/*
 * Port-to-whatchamacallit hash routines.
 */

#include <uxkern/port_hash.h>

port_hash_table_t
port_hash_init(size)
	int	size;
{
	int	i;
	port_hash_table_t	ht;

	ht = (port_hash_table_t) malloc(sizeof(struct port_hash_table) +
					  (size - 1) * sizeof(queue_head_t));
	ht->length = size;

	for (i = 0; i < size; i++) {
	    queue_init(&ht->head[i]);
	}

	return (ht);
}

boolean_t port_hash_enter(ht, port, value)
	port_hash_table_t	ht;
	mach_port_t	port;
	char *		value;
{
	queue_t		q;
	port_hash_entry_t	entry;

	q = &ht->head[PORT_HASH(port) % ht->length];

	entry = (port_hash_entry_t)queue_first(q);
	while (!queue_end(q, (queue_entry_t)entry)) {
	    if (entry->port == port)
		return (FALSE);	/* already there */
	    entry = (port_hash_entry_t)queue_next(&entry->chain);
	}
	entry = (port_hash_entry_t)malloc(sizeof(struct port_hash_entry));

	entry->port = port;
	entry->value = value;

	queue_enter(q, entry, port_hash_entry_t, chain);

	return (TRUE);
}

char *
port_hash_lookup(ht, port)
	port_hash_table_t	ht;
	mach_port_t		port;
{
	queue_t		q;
	port_hash_entry_t	entry;

	q = &ht->head[PORT_HASH(port) % ht->length];

	entry = (port_hash_entry_t)queue_first(q);
	while (!queue_end(q, (queue_entry_t)entry)) {
	    if (entry->port == port)
		return (entry->value);
	    entry = (port_hash_entry_t)queue_next(&entry->chain);
	}
	return (0);
}

boolean_t port_hash_remove(ht, port)
	port_hash_table_t	ht;
	mach_port_t		port;
{
	queue_t		q;
	port_hash_entry_t	entry;

	q = &ht->head[PORT_HASH(port) % ht->length];

	entry = (port_hash_entry_t)queue_first(q);
	while (!queue_end(q, (queue_entry_t)entry)) {
	    if (entry->port == port) {
		queue_remove(q, entry, port_hash_entry_t, chain);
		free(entry);

		return (TRUE);
	    }
	    entry = (port_hash_entry_t)queue_next(&entry->chain);
	}
	return (FALSE);
}

