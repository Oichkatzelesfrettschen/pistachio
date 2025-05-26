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
 * $Log:	bsd_types_gen.c,v $
 * Revision 2.1  92/04/21  17:10:51  rwd
 * BSDSS
 * 
 *
 */

/*
 * Generate definitions for Mig interfaces.  MiG can't handle random
 * C definitions or expressions any better than the assembler.
 */

#include <sys/types.h>
#include <sys/param.h>

main()
{
	printf("#define\tPATH_LENGTH %d\n",
			roundup(MAXPATHLEN,sizeof(int)));
	printf("#define\tSMALL_ARRAY_LIMIT %d\n",
			4096);
	printf("#define\tFD_SET_LIMIT %d\n",
			howmany(FD_SETSIZE, NFDBITS));
	printf("#define\tGROUPS_LIMIT %d\n",
			NGROUPS);
	printf("#define\tHOST_NAME_LIMIT %d\n",
			MAXHOSTNAMELEN);

	return (0);
};
