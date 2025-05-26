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
 * $Log:	touch.c,v $
 * Revision 2.1  92/04/21  17:11:53  rwd
 * BSDSS
 * 
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
main(argc, argv)
	int argc;
	char **argv;
{

	argc--, argv++;
	while (argc > 0) {
		struct stat stb; int c, f;
		if (stat(*argv, &stb) < 0)
			goto bad;
		if (chmod(*argv, stb.st_mode | 0200) < 0)
			goto bad;
		f = open(*argv, 2);
		if (f < 0)
			goto bad;
		lseek(f, 0, 0);
		read(f, &c, 1);
		lseek(f, 0, 0);
		write(f, &c, 1);
		close(f);
		chmod(*argv, stb.st_mode);
		argc--, argv++;
		continue;
bad:
		perror(*argv);
		argc--, argv++;
		continue;
	}
}
