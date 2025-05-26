/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	main.c,v $
 * Revision 2.2  92/06/25  17:30:43  mrt
 * 	Changes from OSF- Added -I flags in order to eliminate knowledge
 * 	of how make is keeping its searchpath. Also eliminates use of
 * 	fopenp which is a CMUism. Removed acient history from log.
 * 	[92/04/27            mrt]
 * 
 * Revision 2.6  91/06/19  11:58:37  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:46:00  rvb]
 * 
 * Revision 2.4  90/11/25  17:48:43  jsb
 * 	Added i860 support.
 * 	[90/11/25  16:53:42  jsb]
 * 
 * Revision 2.3  90/08/27  22:09:35  dbg
 * 	Merged CMU changes into Tahoe release.
 * 	[90/08/16            dbg]
 * 
 * Revision 2.2  90/05/03  15:50:15  dbg
 * 		Cast all sprintf's (void).
 * 		[90/01/22            rvb]
 * 
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)main.c	5.9 (Berkeley) 6/18/88";
#endif /* not lint */

#include <stdio.h>
#include <ctype.h>
#include "gram.h"
#include "config.h"

extern char *realloc();
char *search_path;

/*
 * Config builds a set of files for building a UNIX
 * system given a description of the desired system.
 */
main(argc, argv)
	int argc;
	char **argv;
{

	object_directory = "..";
	config_directory = (char *) 0;
	search_path = (char *) 0;
	DEV_MASK = 0x7;
	DEV_SHIFT = 3;
	while ((argc > 1) && (argv[1][0] == '-')) {
		char		*c;

		argv++; argc--;
		c = &argv[0][1];
		if (*c == 'I') {
			int i;

			c++;
			if (search_path == 0) {
				i = strlen(c) + 1;
				search_path = malloc(i);
				strcpy(search_path, c);
			} else {
				i = strlen(c) + strlen(search_path) + 2;
				search_path = realloc(search_path, i);
				strcat(search_path, ":");
				strcat(search_path, c);
			}
			continue;
		}
		for (; *c; c++) {
			switch (*c) {
				case 'o':
					object_directory = argv[1];
					goto check_arg;

				case 'c':
					config_directory = argv[1];

				 check_arg:
				 	if (argv[1] == (char *) 0)
						goto usage_error;
					argv++; argc--;
					break;

				case 'p':
					fprintf(stderr,
						"make controls profiling\n");
					break;
				default:
					goto usage_error;
			}
		}
	}
	if (config_directory == (char *) 0)
		config_directory = "conf";
	if (search_path == (char *) 0)
		search_path = ".";
	if (argc != 2) {
		usage_error: ;
		fprintf(stderr, "usage: config [ -co dir ] [ -p ] sysname\n");
	}
	PREFIX = argv[1];
	if (freopen(argv[1], "r", stdin) == NULL) {
		perror(argv[1]);
		exit(2);
	}
	dtab = NULL;
	confp = &conf_list;
	opt = 0;
	if (yyparse())
		exit(3);
	machinedep();
	makefile();			/* build Makefile */
	headers();			/* make a lot of .h files */
	swapconf();			/* swap config files */
}

/*
 * get_word
 *	returns EOF on end of file
 *	NULL on end of line
 *	pointer to the word otherwise
 */
char *
get_word(fp)
	register FILE *fp;
{
	static char line[80];
	register int ch;
	register char *cp;

	while ((ch = getc(fp)) != EOF)
		if (ch != ' ' && ch != '\t')
			break;
	if (ch == EOF)
		return ((char *)EOF);
	if (ch == '\n')
		return (NULL);
	if (ch == '|')
		return( "|");
	cp = line;
	*cp++ = ch;
	while ((ch = getc(fp)) != EOF) {
		if (isspace(ch))
			break;
		*cp++ = ch;
	}
	*cp = 0;
	if (ch == EOF)
		return ((char *)EOF);
	(void) ungetc(ch, fp);
	return (line);
}

/*
 * get_rest
 *	returns EOF on end of file
 *	NULL on end of line
 *	pointer to the word otherwise
 */
char *
get_rest(fp)
	register FILE *fp;
{
	static char line[80];
	register int ch;
	register char *cp;

	cp = line;
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n')
			break;
		*cp++ = ch;
	}
	*cp = 0;
	if (ch == EOF)
		return ((char *)EOF);
	return (line);
}

/*
 * prepend the path to a filename
 */
char *
path(file)
	char *file;
{
	register char *cp;

	cp = malloc((unsigned)(strlen(PREFIX)+
			       strlen(file)+
			       strlen(object_directory)+
			       3));
	(void) sprintf(cp, "%s/%s/%s", object_directory, PREFIX, file);
	return (cp);
}
