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
 * $Log:	mkmakefile.c,v $
 * Revision 2.2  92/06/25  17:31:17  mrt
 * 	Changes from OSF - Find machine dependent file in subdirectories.
 * 	Removed %RULES macro, now the rules all come from the .mk files.
 * 	[92/04/27            mrt]
 * 
 * 	Output the makevariable options from MASTER before at start
 * 	of Makefile like was done originally so that they can be used
 * 	to trigger conditonal rules. 
 * 	[91/05/24            mrt]
 * 
 * 	Changed MACHINE variable in do_swapspec to KERN_MACHINE_DIR} to
 * 	be sure we get swapconfig.c from the kernel directory for the 
 * 	target machine.
 * 	[91/05/22            mrt]
 * 
 * Revision 2.8  91/06/19  11:58:56  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:46:20  rvb]
 * 
 * Revision 2.6  90/11/25  17:48:46  jsb
 * 	Added i860 support.
 * 	[90/11/25  16:53:13  jsb]
 * 
 * Revision 2.5  90/08/27  22:10:17  dbg
 * 	Merge old CMU changes into Tahoe release.
 * 	[90/07/10            dbg]
 * 
 * Revision 2.4  90/06/02  15:04:07  rpd
 * 	Removed mig*files, confdep.h.
 * 	[90/03/26  22:58:36  rpd]
 * 
 * Revision 2.3  90/05/03  15:51:03  dbg
 * 	Re-add migdebugfiles to .ORDER list.
 * 
 * 		Cast all sprintf's (void).
 * 		Allow for profiling I386.
 * 		[90/01/22            rvb]
 * 		Made changes for Sun 4.
 * 		[89/07/05  20:18:51  jjc]
 * 		Don't unlink the old Makefile; rename it to Makefile.old.
 * 		This works around a problem with in-kernel AFS.
 * 		[89/04/10	     rpd]
 * 
 * Revision 2.2  89/11/29  14:15:54  af
 * 	Fix rule for swapgeneric for Mips (an oldtimer..).
 * 	[89/10/28            af]
 * 
 * 	Changes for MACH_KERNEL:  remove migdebugfiles from .ORDER list,
 * 	add migdevicefiles.
 * 	[89/03/07            dbg]
 * 
 * Revision 2.1  89/08/03  16:54:54  rwd
 * Created.
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
static char sccsid[] = "@(#)mkmakefile.c	5.21 (Berkeley) 6/18/88";
#endif /* not lint */

/*
 * Build the makefile for the system, from
 * the information in the files files and the
 * additional files for the machine being compiled to.
 */

#include <stdio.h>
#include <ctype.h>
#include "gram.h"
#include "config.h"

#define next_word(fp, wd) \
	{ register char *word = get_word(fp); \
	  if (word == (char *)EOF) \
		return; \
	  else \
		wd = word; \
	}

static	struct file_list *fcur;
char *tail();

/*
 * Lookup a file, by name.
 */
struct file_list *
fl_lookup(file)
	register char *file;
{
	register struct file_list *fp;

	for (fp = ftab ; fp != 0; fp = fp->f_next) {
		if (eq(fp->f_fn, file))
			return (fp);
	}
	return (0);
}

/*
 * Lookup a file, by final component name.
 */
struct file_list *
fltail_lookup(file)
	register char *file;
{
	register struct file_list *fp;

	for (fp = ftab ; fp != 0; fp = fp->f_next) {
		if (eq(tail(fp->f_fn), tail(file)))
			return (fp);
	}
	return (0);
}

/*
 * Make a new file list entry
 */
struct file_list *
new_fent()
{
	register struct file_list *fp;

	fp = (struct file_list *) malloc(sizeof *fp);
	fp->f_needs = 0;
	fp->f_next = 0;
	fp->f_flags = 0;
	fp->f_type = 0;
	fp->f_extra = (char *) 0;
	if (fcur == 0)
		fcur = ftab = fp;
	else
		fcur->f_next = fp;
	fcur = fp;
	return (fp);
}

extern char *search_path;

/*
 * Build the makefile from the skeleton
 */
makefile()
{
	FILE *ifp, *ofp;
	char line[BUFSIZ];
	struct opt *op;
	extern int build_platforms();

	read_files();
	(void) sprintf(line, "%s/template.mk", config_directory);
	ifp = VPATHopen(line, "r");
	if (ifp == 0) {
		perror(line);
		exit(1);
	}
	rename(path("Makefile"), path("Makefile.old"));
	ofp = fopen(path("Makefile"), "w");
	if (ofp == 0) {
		perror(path("Makefile"));
		exit(1);
	}

	fprintf(ofp, "IDENT=-D%s -D%s", conftypename, raise(ident));
	if (platform == 0) {
		printf("platform type must be specified\n");
		exit(1);
	}
	do_build("platforms.h", build_platforms);

	for (op = opt; op; op = op->op_next)
		if (op->op_value)
			fprintf(ofp, " -D%s=\"%s\"", op->op_name, op->op_value);
		else
			fprintf(ofp, " -D%s", op->op_name);
	fprintf(ofp, "\n");

	for (op = mkopt; op; op = op->op_next)
		fprintf(ofp, "%s=%s\n", op->op_name, op->op_value);	
				
	while (fgets(line, BUFSIZ, ifp) != 0) {
		if (*line != '%') {
			fprintf(ofp, "%s", line);
			continue;
		}
		if (eq(line, "%OBJS\n")) {
			do_objs(ofp, "OBJS=", -1);
			continue;
		}
		if (eq(line, "%CFILES\n")) {
			do_files(ofp, 'c');
			do_objs(ofp, "COBJS=", 'c');
			continue;
		}
		if (eq(line, "%SFILES\n")) {
			do_files(ofp, 's');
			do_objs(ofp, "SOBJS=", 's');
			continue;
		}
		if (eq(line, "%BFILES\n")) {
			do_files(ofp, 'b');
			do_objs(ofp, "BOBJS=", 'b');
			continue;
		}
		if (eq(line, "%MACHDEP\n")) {
			do_machdep(ofp);
			continue;
		}
		if (eq(line, "%ORDERED\n")) {
			do_ordered(ofp);
			continue;
		}
		if (eq(line, "%CFLAGS\n")) {
			do_cflags(ofp);
			continue;
		}
		if (eq(line, "%LOAD\n")) {
			do_load(ofp);
			continue;
		}
		fprintf(stderr,
			"Unknown %% construct in generic makefile: %s",
			line);
	}
	(void) fclose(ifp);
	(void) fclose(ofp);
}

/*
 * Read in the information about files used in making the system.
 * Store it in the ftab linked list.
 */
read_files()
{
	FILE *fp;
	register struct file_list *tp, *pf;
	register struct device *dp;
	register struct opt *op;
	char *wd, *this, *needs, *devorprof;
	int options;
	int not_option;
	int ordered;
	int sedit;				/* SQT */
	char fname[1024];
	char *rest = (char *) 0;
	struct platform *pp;
	int nreqs, first = 1, isdup;

	ftab = 0;
	(void) sprintf(fname, "%s/files", config_directory);
openit:
	fp = VPATHopen(fname, "r");
	if (fp == 0) {
		perror(fname);
		exit(1);
	}
next:
	options = 0;
	rest = (char *) 0;
	/*
	 * filename	[ standard | optional ]
	 *	[ dev* | profiling-routine ] [ device-driver]
	 */
	/*
	 * MACHINE_SQT ONLY:
	 *
	 * filename	[ standard | optional ] 
	 *	[ ordered | sedit ]
	 *	[ dev* | profiling-routine ] [ device-driver]
	 */
	wd = get_word(fp);
	if (wd == (char *)EOF) {
		(void) fclose(fp);
		if (first == 1) {
			(void) sprintf(fname, "%s/%s/files", config_directory, conftypename);
			first++;
			goto openit;
		}
		if (first == 2) {
			(void) sprintf(fname, "files.%s", raise(ident));
			first++;
			fp = VPATHopen(fname, "r");
			if (fp != 0)
				goto next;
		}
		return;
	}
	if (wd == 0)
		goto next;
	/*
	 *  Allow comment lines beginning witha '#' character.
	 */
	if (*wd == '#')
	{
		while ((wd=get_word(fp)) && wd != (char *)EOF)
			;
		goto next;
	}

	this = ns(wd);
	next_word(fp, wd);
	if (wd == 0) {
		printf("%s: No type for %s.\n",
		    fname, this);
		exit(1);
	}
	if ((pf = fl_lookup(this)) && (pf->f_type != INVISIBLE || pf->f_flags))
		isdup = 1;
	else
		isdup = 0;
	tp = 0;
	if (first == 3 && (tp = fltail_lookup(this)) != 0)
		printf("%s: Local file %s overrides %s.\n",
		    fname, this, tp->f_fn);
	nreqs = 0;
	devorprof = "";
	ordered = 0;
	sedit = 1;				/* SQT: assume sedit for now */
	needs = 0;
	if (eq(wd, "standard"))
		goto checkdev;
	if (!eq(wd, "optional")) {
		printf("%s: %s must be optional or standard\n", fname, this);
		exit(1);
	}
	if (strncmp(this, "OPTIONS/", 8) == 0)
		options++;
	not_option = 0;
nextopt:
	next_word(fp, wd);
	if (wd == 0)
		goto doneopt;
	if (eq(wd, "ordered")) {
		ordered++;
		goto nextopt;
	}
	if (eq(wd, "sedit")) {
		sedit++;
		goto nextopt;
	}
	if (eq(wd, "not")) {
		not_option = !not_option;
		goto nextopt;
	}
	devorprof = wd;
	if (eq(wd, "device-driver") || eq(wd, "profiling-routine")) {
		next_word(fp, wd);
		goto save;
	}
	nreqs++;
	if (needs == 0 && nreqs == 1)
		needs = ns(wd);
	if (isdup)
		goto invis;
	if (options)
	{
		register struct opt *op;
		struct opt *lop = 0;
		struct device tdev;

		/*
		 *  Allocate a pseudo-device entry which we will insert into
		 *  the device list below.  The flags field is set non-zero to
		 *  indicate an internal entry rather than one generated from
		 *  the configuration file.  The slave field is set to define
		 *  the corresponding symbol as 0 should we fail to find the
		 *  option in the option list.
		 */
		init_dev(&tdev);
		tdev.d_name = ns(wd);
		tdev.d_type = PSEUDO_DEVICE;
		tdev.d_flags++;
		tdev.d_slave = 0;

		for (op=opt; op; lop=op, op=op->op_next)
		{
			char *od = raise(ns(wd));

			/*
			 *  Found an option which matches the current device
			 *  dependency identifier.  Set the slave field to
			 *  define the option in the header file.
			 */
			if (strcmp(op->op_name, od) == 0)
			{
				tdev.d_slave = 1;
				if (lop == 0)
					opt = op->op_next;
				else
					lop->op_next = op->op_next;
				free(op);
				op = 0;
			 }
			free(od);
			if (op == 0)
				break;
		}
		newdev(&tdev);
	}
 	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (eq(dp->d_name, wd) && (dp->d_type != PSEUDO_DEVICE || dp->d_slave)) {
			if (not_option)
				goto invis;	/* dont want file if option present */
			else
				goto nextopt;
		}
	}
	if (not_option)
		goto nextopt;		/* want file if option missing */

	for (op = opt; op != 0; op = op->op_next)
		if (op->op_value == 0 && opteq(op->op_name, wd)) {
			if (nreqs == 1) {
				free(needs);
				needs = 0;
			}
			goto nextopt;
		}

	for (pp = platform; pp; pp = pp->next)
		if (opteq(pp->name, wd)) {
			if (nreqs == 1) {
				free(needs);
				needs = 0;
			}
			goto nextopt;
		}

invis:
	while ((wd = get_word(fp)) != 0)
		;
	if (tp == 0)
		tp = new_fent();
	tp->f_fn = this;
	tp->f_type = INVISIBLE;
	tp->f_needs = needs;
	tp->f_flags = isdup;
	goto next;

doneopt:
	if (nreqs == 0) {
		printf("%s: what is %s optional on?\n",
		    fname, this);
		exit(1);
	}

checkdev:
	if (wd) {
		if (*wd == '|')
			goto getrest;
		next_word(fp, wd);
		if (wd) {
			if (eq(wd, "ordered")) {
				ordered++;
				goto checkdev;
			}
			if (eq(wd, "sedit")) {
				sedit++;
				goto checkdev;
			}
			devorprof = wd;
			next_word(fp, wd);
		}
	}

save:
getrest:
	if (wd) {
		if (*wd == '|') {
			rest = ns(get_rest(fp));
		} else {
			printf("%s: syntax error describing %s\n",
			       fname, this);
			exit(1);
		}
	}
	if (eq(devorprof, "profiling-routine"))
		goto next;
	if (tp == 0)
		tp = new_fent();
	tp->f_fn = this;
	tp->f_extra = rest;
	if (options)
		tp->f_type = INVISIBLE;
	else if (eq(devorprof, "device-driver"))
		tp->f_type = DRIVER;
	else if (eq(devorprof, "profiling-routine"))
		tp->f_type = PROFILING;
	else
		tp->f_type = NORMAL;
	tp->f_flags = 0;
	if (ordered)
		tp->f_flags |= ORDERED;
	if (sedit)				/* SQT */
		tp->f_flags |= SEDIT;
	tp->f_needs = needs;
	if (pf && pf->f_type == INVISIBLE)
		pf->f_flags = 1;		/* mark as duplicate */
	goto next;
}

opteq(cp, dp)
	char *cp, *dp;
{
	char c, d;

	for (; ; cp++, dp++) {
		if (*cp != *dp) {
			c = isupper(*cp) ? tolower(*cp) : *cp;
			d = isupper(*dp) ? tolower(*dp) : *dp;
			if (c != d)
				return (0);
		}
		if (*cp == 0)
			return (1);
	}
}

do_objs(fp, msg, ext)
	char	*msg;
	int	ext;
	FILE *fp;
{
	register struct file_list *tp, *fl;
	register int lpos, len;
	register char *cp, och, *sp;
	char swapname[32];

	fprintf(fp, msg);
	lpos = strlen(msg);
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if (tp->f_type == INVISIBLE)
			continue;

		/*
		 *	Check for '.o' file in list
		 */
		cp = tp->f_fn + (len = strlen(tp->f_fn)) - 1;
		if ((ext == -1 && (tp->f_flags & ORDERED)) || /* not in objs */
		    (ext != -1 && *cp != ext))
			continue;
		else if (*cp == 'o') {
			if (len + lpos > 72) {
				lpos = 8;
				fprintf(fp, "\\\n\t");
			}
			fprintf(fp, "%s ", tp->f_fn);
			fprintf(fp, " ");
			lpos += len + 1;
			continue;
		}
		sp = tail(tp->f_fn);
		for (fl = conf_list; fl; fl = fl->f_next) {
			if (fl->f_type != SWAPSPEC)
				continue;
			(void) sprintf(swapname, "swap%s.c", fl->f_fn);
			if (eq(sp, swapname))
				goto cont;
		}
		cp = sp + (len = strlen(sp)) - 1;
		och = *cp;
		*cp = 'o';
		if (len + lpos > 72) {
			lpos = 8;
			fprintf(fp, "\\\n\t");
		}
		fprintf(fp, "%s ", sp);
		lpos += len + 1;
		*cp = och;
cont:
		;
	}
	if (lpos != 8)
		putc('\n', fp);
}

/* not presently used and probably broken,  use ORDERED instead */
do_ordered(fp)
	FILE *fp;
{
	register struct file_list *tp, *fl;
	register int lpos, len;
	register char *cp, och, *sp;

	fprintf(fp, "ORDERED=");
	lpos = 10;
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if ((tp->f_flags & ORDERED) != ORDERED)
			continue;
		sp = tail(tp->f_fn);
		cp = sp + (len = strlen(sp)) - 1;
		och = *cp;
		*cp = 'o';
		if (len + lpos > 72) {
			lpos = 8;
			fprintf(fp, "\\\n\t");
		}
		fprintf(fp, "%s ", sp);
		lpos += len + 1;
		*cp = och;
cont:
		;
	}
	if (lpos != 8)
		putc('\n', fp);
}

do_files(fp, ext)
	FILE	*fp;
	char	ext;
{
	register struct file_list *tp;
	register int len;
	register char *cp, och, *sp, *np;

	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if (tp->f_type == INVISIBLE)
			continue;
		cp = (np = tp->f_fn) + (len = strlen(tp->f_fn)) - 1;
		if (*cp != ext)
			continue;
		if (np[0] == '.' && np[1] == '/')
			np += 2;
		sp = tail(np);
		och = *cp;
		*cp = 'o';
		fprintf(fp, "%s_SOURCE=", sp);
		*cp = och;
		fprintf(fp, "%s\n", np);
	}
	putc('\n', fp);
}

/*
 *  Include machine dependent makefile in output
 */

do_machdep(ofp)
	FILE *ofp;
{
	int c;
	FILE *ifp;
	char line[BUFSIZ];

	(void) sprintf(line, "%s/%s/template.mk",
		       config_directory, conftypename);
	ifp = VPATHopen(line, "r");
	if (ifp == 0) {
		perror(line);
		exit(1);
	}
	while (fgets(line, BUFSIZ, ifp) != 0)
		fputs(line, ofp);
	fclose(ifp);
}



/*
 *  Format cpu types file.
 */

build_platforms(fp)
	FILE *fp;
{
	struct platform *pp;

	for (pp = platform; pp; pp = pp->next)
		fprintf(fp, "#define\t%s\t1\n", pp->name);
}


/*
 *  Build a define parameter file.  Create it first in a temporary location and
 *  determine if this new contents differs from the old before actually
 *  replacing the original (so as not to introduce avoidable extraneous
 *  compilations).
 */

do_build(name, format)
	char *name;
	int (*format)();
{
	static char temp[]="#config.tmp";
	FILE *tfp, *ofp;
	int c;

	tfp = fopen(path(temp), "w+");
	if (tfp == 0) {
		perror(path(temp));
		exit(1);
	}
	unlink(path(temp));
	(*format)(tfp);
	ofp = fopen(path(name), "r");
	if (ofp != 0)
	{
		fseek(tfp, 0, 0);
		while ((c = fgetc(tfp)) != EOF)
			if (fgetc(ofp) != c)
				goto copy;
		if (fgetc(ofp) == EOF)
			goto same;
		
	}
copy:
	if (ofp)
		fclose(ofp);
	ofp = fopen(path(name), "w");
	if (ofp == 0) {
		perror(path(name));
		exit(1);
	}
	fseek(tfp, 0, 0);
	while ((c = fgetc(tfp)) != EOF)
		fputc(c, ofp);
same:
	fclose(ofp);
	fclose(tfp);
}

char *
tail(fn)
	char *fn;
{
	register char *cp;

	cp = rindex(fn, '/');
	if (cp == 0)
		return (fn);
	return (cp+1);
}

/*
 * Create any special cflags for each file which is part of the system.
 */
do_cflags(f)
	FILE *f;
{
	register char *cp, *np, och, *tp;
	register struct file_list *ftp;

	for (ftp = ftab; ftp != 0; ftp = ftp->f_next) {
		if (ftp->f_type == INVISIBLE)
			continue;
		cp = (np = ftp->f_fn) + strlen(ftp->f_fn) - 1;
		och = *cp;
		if (och == 'o')
			continue;
		if ((np[0] == '.') && (np[1] == '/'))
			np += 2;
		*cp = 'o';
		tp = tail(np);
		if (ftp->f_extra)
			fprintf(f, "%s_CFLAGS+=%s\n", tp, ftp->f_extra);
		if (och == 's' || och == 'b') {
			*cp = och;
			continue;
		}
		if (ftp->f_type == DRIVER) {
			fprintf(f, "%s_CFLAGS+=${DRIVER_CFLAGS}\n", tp);
			fprintf(f, "%s_DRIVER=\n", tp);
		} else if (ftp->f_type == PROFILING) {
			fprintf(f, "%s_CFLAGS+=${PROFILING_CFLAGS}\n", tp);
			fprintf(f, "%s_PROFILING=\n", tp);
		} else if (ftp->f_type != NORMAL)
			printf("Don't know rules for %s\n", np);
		*cp = och;
	}
}

/*
 * Create the load strings
 */
do_load(f)
	register FILE *f;
{
	register struct file_list *fl;

	fl = conf_list;
	while (fl) {
		if (fl->f_type != SYSTEMSPEC) {
			fl = fl->f_next;
			continue;
		}
		do_swapspec(f, fl->f_fn, fl->f_needs);
		for (fl = fl->f_next;
		     fl != NULL && fl->f_type == SWAPSPEC;
		     fl = fl->f_next)
		continue;
	}
	fprintf(f, "LOAD =");
	for (fl = conf_list; fl != 0; fl = fl->f_next)
		if (fl->f_type == SYSTEMSPEC)
			fprintf(f, " %s", fl->f_needs);
	fprintf(f, "\n");
}

do_swapspec(f, name, system)
	char *system;
	FILE *f;
	register char *name;
{

	char *gdir = eq(name, "generic")?"${KERN_MACHINE_DIR}/":"";

	fprintf(f, "%s.swap_SWAPSPEC=swap%s.o\n", system, name);
        fprintf(f, "swap%s.o_SOURCE=%sswap%s.c\n", name, gdir, name);
	fprintf(f, "COBJS+=swap%s.o\n", name);
}

char *
raise(str)
	register char *str;
{
	register char *cp = str;

	while (*str) {
		if (islower(*str))
			*str = toupper(*str);
		str++;
	}
	return (cp);
}

FILE *
VPATHopen(file, mode)
register char *file, *mode;
{
	register char *nextpath,*nextchar,*fname,*lastchar;
	char fullname[BUFSIZ];
	FILE *fp;

	nextpath = ((*file == '/') ? "" : search_path);
	do {
		fname = fullname;
		nextchar = nextpath;
		while (*nextchar && (*nextchar != ':'))
			*fname++ = *nextchar++;
		if (nextchar != nextpath && *file) *fname++ = '/';
		lastchar = nextchar;
		nextpath = ((*nextchar) ? nextchar + 1 : nextchar);
		nextchar = file;	/* append file */
		while (*nextchar)  *fname++ = *nextchar++;
		*fname = '\0';
		fp = fopen (fullname, mode);
	} while (fp == NULL && (*lastchar));
	return (fp);
}
