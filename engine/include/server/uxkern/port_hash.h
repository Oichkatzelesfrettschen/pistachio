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
 * $Log:	port_hash.h,v $
 * Revision 2.1  92/04/21  17:11:01  rwd
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
 * $Log:	port_hash.h,v $
 * Revision 2.1  92/04/21  17:11:01  rwd
 * BSDSS
 * 
 * Revision 3.1  91/06/25  17:13:16  condict
 * Moved sys/*.h files that were from OSF/1 kern dir, back to kern.
 * 
 * Revision 3.0  91/01/17  12:06:13  condict
 * Unchanged copy from Mach 3.0 BSD UNIX server
 * 
 * Revision 2.2  90/06/02  15:28:09  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  20:21:33  rpd]
 * 
 */

/*
 * Port-to-pointer hash routines.
 */
#include <sys/queue.h>
#include <uxkern/import_mach.h>

struct port_hash_entry {
	queue_chain_t	chain;
	mach_port_t	port;
	char *		value;
};
typedef struct port_hash_entry *port_hash_entry_t;

struct port_hash_table {
	int		length;
	queue_head_t	head[1];	/* variable size */
};
typedef struct port_hash_table *port_hash_table_t;

#define	PORT_HASH(port)	(port)

port_hash_table_t	port_hash_init();
boolean_t		port_hash_enter();
char *			port_hash_lookup();
boolean_t		port_hash_remove();
