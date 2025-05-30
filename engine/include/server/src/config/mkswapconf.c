/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	mkswapconf.c,v $
 * Revision 2.2  92/06/25  17:31:25  mrt
 * 	Modified for ODE make
 * 	[92/06/17  13:41:45  mrt]
 * 
 * Revision 2.5  91/06/19  11:59:02  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:46:26  rvb]
 * 
 * Revision 2.4  91/02/05  17:53:57  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:53:06  mrt]
 * 
 * Revision 2.3  90/08/27  22:10:28  dbg
 * 	Merge in old CMU changes into Tahoe release:
 * 
 * 	Made emitted #include lines use <> instead of "".
 * 	Updated initdevtable() to use config_directory, fopenp.
 * 	Skip comment lines (start with #) and blank lines.
 * 	[89/02/19	     rpd]
 * 
 * Revision 2.2  90/05/03  15:51:09  dbg
 * 	Cast all sprintf's (void).
 * 	[90/01/22            rvb]
 * 
 * Revision 2.1  89/08/03  16:55:01  rwd
 * 	Created.
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
static char sccsid[] = "@(#)mkswapconf.c	5.6 (Berkeley) 6/18/88";
#endif /* not lint */

/*
 * Build a swap configuration file.
 */
#include "config.h"

#include <stdio.h>
#include <ctype.h>

swapconf()
{
	register struct file_list *fl;
	struct file_list *do_swap();

	fl = conf_list;
	while (fl) {
		if (fl->f_type != SYSTEMSPEC) {
			fl = fl->f_next;
			continue;
		}
		fl = do_swap(fl);
	}
}

struct file_list *
do_swap(fl)
	register struct file_list *fl;
{
	FILE *fp;
	char  swapname[80], *cp;
	register struct file_list *swap;
	dev_t dev;

	if (eq(fl->f_fn, "generic")) {
		fl = fl->f_next;
		return (fl->f_next);
	}
	(void) sprintf(swapname, "swap%s.c", fl->f_fn);
	fp = fopen(path(swapname), "w");
	if (fp == 0) {
		perror(path(swapname));
		exit(1);
	}
	fprintf(fp, "#include <sys/param.h>\n");
	fprintf(fp, "#include <sys/conf.h>\n");
	fprintf(fp, "\n");
	/*
	 * If there aren't any swap devices
	 * specified, just return, the error
	 * has already been noted.
	 */
	swap = fl->f_next;
	if (swap == 0 || swap->f_type != SWAPSPEC) {
		(void) unlink(path(swapname));
		fclose(fp);
		return (swap);
	}
	fprintf(fp, "dev_t\trootdev = makedev(%d, %d);\n",
		major(fl->f_rootdev), minor(fl->f_rootdev));
	fprintf(fp, "dev_t\targdev  = makedev(%d, %d);\n",
		major(fl->f_argdev), minor(fl->f_argdev));
	fprintf(fp, "dev_t\tdumpdev = makedev(%d, %d);\n",
		major(fl->f_dumpdev), minor(fl->f_dumpdev));
	fprintf(fp, "\n");
	fprintf(fp, "struct\tswdevt swdevt[] = {\n");
	do {
		dev = swap->f_swapdev;
		fprintf(fp, "\t{ makedev(%d, %d),\t0,\t%d },\t/* %s */\n",
		    major(dev), minor(dev), swap->f_swapsize, swap->f_fn);
		swap = swap->f_next;
	} while (swap && swap->f_type == SWAPSPEC);
	fprintf(fp, "\t{ 0, 0, 0 }\n");
	fprintf(fp, "};\n");
/*	
	if (conftype == CONFTYPE_MIPSY || conftype == CONFTYPE_MIPS) {
		fprintf(fp, "\nsetconf()\n");
		fprintf(fp, "{\n");
		fprintf(fp, "\t/ * resolve reference for non-generic kernels * /\n");
		fprintf(fp, "}\n");
	}
*/
	fclose(fp);
	return (swap);
}

static	int devtablenotread = 1;
static	struct devdescription {
	char	*dev_name;
	int	dev_major;
	struct	devdescription *dev_next;
} *devtable;

/*
 * Given a device name specification figure out:
 *	major device number
 *	partition
 *	device name
 *	unit number
 * This is a hack, but the system still thinks in
 * terms of major/minor instead of string names.
 */
dev_t
nametodev(name, defunit, defpartition)
	char *name;
	int defunit;
	char defpartition;
{
	char *cp, partition;
	int unit;
	register struct devdescription *dp;

	cp = name;
	if (cp == 0) {
		fprintf(stderr, "config: internal error, nametodev\n");
		exit(1);
	}
	while (*cp && !isdigit(*cp))
		cp++;
	unit = *cp ? atoi(cp) : defunit;
	if (unit < 0 || unit > 31) {
		fprintf(stderr,
"config: %s: invalid device specification, unit out of range\n", name);
		unit = defunit;			/* carry on more checking */
	}
	if (*cp) {
		*cp++ = '\0';
		while (*cp && isdigit(*cp))
			cp++;
	}
	partition = *cp ? *cp : defpartition;
	if (partition < 'a' || partition > 'h') {
		fprintf(stderr,
"config: %c: invalid device specification, bad partition\n", *cp);
		partition = defpartition;	/* carry on */
	}
	if (devtablenotread)
		initdevtable();
	for (dp = devtable; dp->dev_next; dp = dp->dev_next)
		if (eq(name, dp->dev_name))
			break;
	if (dp == 0) {
		fprintf(stderr, "config: %s: unknown device\n", name);
		return (NODEV);
	}
	return (makedev(dp->dev_major, (unit << DEV_SHIFT) + (partition - 'a')));
}

char *
devtoname(dev)
	dev_t dev;
{
	char buf[80]; 
	register struct devdescription *dp;

	if (devtablenotread)
		initdevtable();
	for (dp = devtable; dp->dev_next; dp = dp->dev_next)
		if (major(dev) == dp->dev_major)
			break;
	if (dp == 0)
		dp = devtable;
	(void) sprintf(buf, "%s%d%c", dp->dev_name,
		minor(dev) >> DEV_SHIFT, (minor(dev) & DEV_MASK) + 'a');
	return (ns(buf));
}

initdevtable()
{
	char buf[BUFSIZ];
	char line[BUFSIZ];
	int maj;
	register struct devdescription **dp = &devtable;
	FILE *fp;

	(void) sprintf(buf, "%s/%s/devices", config_directory, conftypename);
	fp = VPATHopen(buf, "r");
	if (fp == NULL) {
		fprintf(stderr, "config: can't open %s\n", buf);
		exit(1);
	}
	while (fgets(line, BUFSIZ, fp) != 0) {
		if (*line == '#' || *line == '\n')
			continue;
		if (sscanf(line, "%s\t%d\n", buf, &maj) != 2)
			break;
		*dp = (struct devdescription *)malloc(sizeof (**dp));
		(*dp)->dev_name = ns(buf);
		(*dp)->dev_major = maj;
		dp = &(*dp)->dev_next;
	}
	*dp = 0;
	fclose(fp);
	devtablenotread = 0;
}
