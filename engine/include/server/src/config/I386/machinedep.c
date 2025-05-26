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
 * $Log:	machinedep.c,v $
 * Revision 2.2  92/06/25  17:29:22  mrt
 * 	Moved machine dependent code from mkioconf and mkglue to here
 * 	[92/06/17  13:44:50  mrt]
 * 
 * Revision 2.6  91/06/19  11:58:50  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:46:13  rvb]
 * 
 * Revision 2.5  91/05/08  13:09:06  dbg
 * 	Fix sqt_ioconf to allow wildcards in sec devices and change
 * 	location of ioconf.h.
 * 	[91/03/21            dbg]
 * 
 * Revision 2.4  91/02/05  17:53:31  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:52:28  mrt]
 * 
 * Revision 2.3  90/08/27  22:09:51  dbg
 * 	Merged CMU changes into Tahoe release.
 * 	[90/08/16            dbg]
 * 
 * Revision 2.2  90/05/03  15:50:34  dbg
 * 	Cast all sprintf's (void).
 * 	[90/01/22            rvb]
 * 
 * 	Modified sun_ioconf() for Sun 4.
 * 	[89/07/12  16:58:21  jjc]
 * 
 * 	Mach on Mips has no pte.h crud.
 * 	[89/04/20            af]
 * 
 * 	MACH_KERNEL: wrap include files in '#ifndef MACH_KERNEL'.
 * 	[89/03/03            dbg]
 * 
 * Revision 2.1  89/08/03  16:54:47  rwd
 * Created.
 * 
 * Revision 2.7  89/02/25  19:24:03  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.6  89/02/19  18:35:22  rpd
 * 	Made emitted #include's use <> instead of "".
 * 
 * Revision 2.5  89/02/07  22:55:54  mwyoung
 * Code cleanup cataclysm.
 * 
 * Revision 2.4  89/01/23  22:23:35  af
 * 	Changes for MIPS: use mkioconf() from mips
 * 	[89/01/09            rvb]
 * 
 * Revision 2.3  88/11/23  16:41:58  rpd
 * 	romp: Allow fd units to be wildcarded.
 * 	[88/11/04  17:59:12  rpd]
 * 
 * Revision 2.2  88/07/20  16:40:53  rpd
 * Fixed static initializers.
 * 
 *  9-Oct-87  Richard Sanzi (sanzi) at Carnegie-Mellon University
 *	Added change from ACIS to support wildcarding the hdc
 *	controlllers.
 *
 * 08-Jan-87  Robert Beck (beck) at Sequent Computer Systems, Inc.
 *	Add MACHINE_SQT cases.
 *
 * 27-Oct-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Merged in David Black's changes for the Multimax.
 *
 * 05-Jun-86  Jonathan J. Chew (jjc) at Carnegie-Mellon University
 *	Merged in changes for Sun 2 and 3.
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
static char sccsid[] = "@(#)machinedep.c	5.10 (Berkeley) 6/18/88";
#endif /* not lint */

#include <stdio.h>
#include "gram.h"
#include "config.h"

/*
 * build the ioconf.c file
 */
extern char	*qu();
extern char	*intv();

/*
 * Define prototype device spec lines.
 *
 * For now, have static set of controller prototypes.  This should be
 * upgraded to using (eg) controllers.balance (ala Sequent /etc/config)
 * to support custom boards without need to edit this file.
 */

/*
 *  flags for indicating presence of upper and lower bound values
 */

#define	P_LB	1
#define	P_UB	2

struct p_entry {
	char 	*p_name;			/* name of field */
	long	p_def;				/* default value */
	long 	p_lb;				/* lower bound for field */
	long	p_ub;				/* upper bound of field */ 
	char	p_flags;			/* bound valid flags */
};

struct proto {
	char	*p_name;			/* name of controller type */
	struct  p_entry	p_fields[NFIELDS];	/* ordered list of fields */
	int	p_seen;				/* any seen? */
};

/*
 * MULTIBUS Adapter:
 *	type mbad  index csr flags maps[0,256] bin[0,7] intr[0,7]
 */

static	struct	proto	mbad_proto = {
	"mbad",
       {{ "index",	0,	0,	0,	0 },
	{ "csr",	0,	0,	0,	0 },
	{ "flags",	0,	0,	0,	0 },
	{ "maps",	0,	0,	256,	P_LB|P_UB },
	{ "bin",	0,	0,	7,	P_LB|P_UB },
	{ "intr",	0,	0,	7,	P_LB|P_UB },},
};

/*
 * SCSI/Ether Controller:
 *	type sec   flags bin[0,7] req doneq index target[-1,7]=-1 unit
 */

static	struct	proto	sec_proto = {
	"sec",
       {{ "flags",	0,	0,	0,	0 },
	{ "bin",	0,	0,	7,	P_LB|P_UB } ,
	{ "req",	0,	0,	0,	0 },
	{ "doneq",	0,	0,	0,	0 },
	{ "index",	0,	0,	0,	0 },
	{ "target",	-1,	-1,	7,	P_LB|P_UB },
	{ "unit",	0,	0,	0,	0 },},
};

/*
 * "Zeke" (FAST) Disk Controller (Dual-Channel Disk Controller):
 *	type zdc index[0,31] drive[-1,7] drive_type[-1,1]
 *
 * Levgal values for drive_type:
 *	M2333K = 0	(swallow)
 *	M2351A = 1	(eagle)
 *	wildcard = -1	(run-time determined)
 */

static	struct	proto	zdc_proto = {
	"zdc",
       {{ "index",	0,	0,	31,	P_LB|P_UB },
	{ "drive",	0,	-1,	7,	P_LB|P_UB },
	{ "drive_type",	0,	-1,	1,	P_LB|P_UB },},
};

static	struct	proto	*ptab[] = {
	&mbad_proto,
	&sec_proto,
	&zdc_proto,
	(struct proto *) 0
};

/*
 * locate a prototype structure in the queue of such structures.
 * return NULL if not found.
 */

static struct proto *
find_proto(str)
	register char *str;
{
	register struct proto *ptp;
	register int	ptbx;

	for (ptbx = 0; (ptp = ptab[ptbx]) != NULL; ptbx++) {
		if (eq(str, ptp->p_name))
			return(ptp);
	}
	return(NULL);
}

dev_param(dp, str, num)
	register struct device *dp;
	register char *str;
	long	num;
{
    register struct p_entry *entry;
    register struct proto *ptp;

    if ( strcmp(platform->name, "SYMMETRY") == 0 ){
	ptp = find_proto(dp->d_conn->d_name);
	if (ptp == NULL) {
		fprintf(stderr,"dev %s cont %s", dp->d_name, dp->d_conn->d_name);
		yyerror("invalid controller");
		return;
	}

	for (entry = ptp->p_fields; entry->p_name != NULL; entry++) {
		if (eq(entry->p_name, str)) {
			if ((entry->p_flags & P_LB) && (num < entry->p_lb)) {
				yyerror("parameter below range");
				return;
			}
			if ((entry->p_flags & P_UB) && (num > entry->p_ub)) {
				yyerror("parameter above range");
				return;
			}
			dp->d_fields[entry-ptp->p_fields] = num;
			return;
		}
	}

	yyerror("invalid parameter");
    }
}

sqt_ioconf()
{
	register struct device *dp, *mp;
	register int count;
register char *namep;
	register struct proto *ptp;
	register struct p_entry *entry;
	FILE	*fp;
	int	bin_table[8];
	int	ptbx;
	int	found;

	for (count = 0; count < 8; count++)
		bin_table[count] = 0;
	fp = fopen(path("ioconf.c"), "w");
	if (fp == NULL) {
		perror(path("ioconf.c"));
		exit(1);
	}
	fprintf(fp, "#include <sqt/ioconf.h>\n");

	fprintf(fp, "\nu_long\tMBAd_IOwindow =\t\t3*256*1024;\t/* top 1/4 Meg */\n\n");

	for (ptbx = 0; (ptp = ptab[ptbx]) != NULL; ptbx++) {

		fprintf(fp, "/*\n");
		fprintf(fp, " * %s device configuration.\n", ptp->p_name);
		fprintf(fp, " */\n\n");
		fprintf(fp, "\n");
		fprintf(fp, "#include <sqt%s/ioconf.h>\n", ptp->p_name);
		fprintf(fp, "\n");

		/*
		 * Generate dev structures for this controller
		 */
		for (dp = dtab, namep = NULL; dp != 0; dp = dp->d_next) {
			mp = dp->d_conn;
			if (mp == 0 || mp == TO_NEXUS ||
			   !eq(mp->d_name, ptp->p_name) ||
			   (namep != NULL && eq(dp->d_name, namep)) )
				continue;
			fprintf(fp, "extern\tstruct\t%s_driver\t%s_driver;\n",
			    ptp->p_name, namep = dp->d_name);
			ptp->p_seen = 1;
		}

		found = 0;
		for (dp = dtab, namep = NULL; dp != 0; dp = dp->d_next) {
			mp = dp->d_conn;
			if (mp == 0 || mp == TO_NEXUS ||
			   !eq(mp->d_name, ptp->p_name))
				continue;
			if (namep == NULL || !eq(namep, dp->d_name)) {
				count = 0;
				if (namep != NULL) 
					fprintf(fp, "};\n");
				found = 1;
				fprintf(fp, "\nstruct\t%s_dev %s_%s[] = {\n",
						ptp->p_name,
						ptp->p_name,
						namep = dp->d_name);
				fprintf(fp, "/*");
				entry = ptp->p_fields;
				for (; entry->p_name != NULL; entry++)
					fprintf(fp, "\t%s",entry->p_name);
				fprintf(fp, " */\n");
			}
			if (dp->d_bin != UNKNOWN)
				bin_table[dp->d_bin]++;
			fprintf(fp, "{");
			for (entry = ptp->p_fields; entry->p_name != NULL; entry++) {
				if (eq(entry->p_name,"index"))
					fprintf(fp, "\t%d,", mp->d_unit);
				else
					fprintf(fp, "\t%d,",
						dp->d_fields[entry-ptp->p_fields]);
			}
			fprintf(fp, "\t},\t/* %s%d */\n", dp->d_name, count++);
		}
		if (found)
			fprintf(fp, "};\n\n");

		/*
	 	* Generate conf array
	 	*/
		fprintf(fp, "/*\n");
		fprintf(fp, " * %s_conf array collects all %s devices\n", 
			ptp->p_name, ptp->p_name);
		fprintf(fp, " */\n\n");
		fprintf(fp, "struct\t%s_conf %s_conf[] = {\n", 
			ptp->p_name, ptp->p_name);
		fprintf(fp, "/*\tDriver\t\t#Entries\tDevices\t\t*/\n");
		for (dp = dtab, namep = NULL; dp != 0; dp = dp->d_next) {
			mp = dp->d_conn;
			if (mp == 0 || mp == TO_NEXUS ||
			   !eq(mp->d_name, ptp->p_name))
				continue;
			if (namep == NULL || !eq(namep, dp->d_name)) {
				if (namep != NULL)
					fprintf(fp, 
			"{\t&%s_driver,\t%d,\t\t%s_%s,\t},\t/* %s */\n",
			namep, count, ptp->p_name, namep, namep);
				count = 0;
				namep = dp->d_name;
			}
			++count;
		}
		if (namep != NULL) {
			fprintf(fp, 
			  "{\t&%s_driver,\t%d,\t\t%s_%s,\t},\t/* %s */\n",
			  namep, count, ptp->p_name, namep, namep);
		}
		fprintf(fp, "\t{ 0 },\n");
		fprintf(fp, "};\n\n");

	}

	/*
	 * Pseudo's
	 */

	fprintf(fp, "/*\n");
	fprintf(fp, " * Pseudo-device configuration\n");
	fprintf(fp, " */\n\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_type == PSEUDO_DEVICE) {
			fprintf(fp, "extern\tint\t%sboot();\n", dp->d_name);
		}
	}
	fprintf(fp, "\nstruct\tpseudo_dev pseudo_dev[] = {\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_type == PSEUDO_DEVICE) {
			fprintf(fp, "\t{ \"%s\",\t%d,\t%sboot,\t},\n",
				dp->d_name, 
				dp->d_slave == UNKNOWN ? 32 : dp->d_slave, 
				dp->d_name);
		}
	}
	fprintf(fp, "\t{ 0 },\n");
	fprintf(fp, "};\n\n");

	/*
	 * Bin interrupt table and misc
	 */

	fprintf(fp, "/*\n");
	fprintf(fp, " * Interrupt table\n");
	fprintf(fp, " */\n\n");
	fprintf(fp, "int\tbin_intr[8] = {\n");
	fprintf(fp, "\t\t0,\t\t\t\t/* bin 0, always zero */\n");
	for (count=1; count < 8; count++) {
		fprintf(fp, "\t\t%d,\t\t\t\t/* bin %d */\n", 
			bin_table[count], count);
	}
	fprintf(fp, "};\n");

	/*
	 * b8k_cntlrs[]
	 */

	fprintf(fp, "/*\n");
	fprintf(fp, " * b8k_cntlrs array collects all controller entries\n");
	fprintf(fp, " */\n\n");
	for (ptbx = 0; (ptp = ptab[ptbx]) != NULL; ptbx++) {
		if (ptp->p_seen)
			fprintf(fp, "extern int  conf_%s(),\tprobe_%s_devices(),\t%s_map();\n",
				ptp->p_name, ptp->p_name, ptp->p_name);
	}
	fprintf(fp, "\n\nstruct\tcntlrs b8k_cntlrs[] = {\n");
	fprintf(fp, "/*\tconf\t\tprobe_devs\t\tmap\t*/\n");

	for (ptbx = 0; (ptp = ptab[ptbx]) != NULL; ptbx++) {
		if (ptp->p_seen)
			fprintf(fp, "{\tconf_%s,\tprobe_%s_devices,\t%s_map\t}, \n",
				ptp->p_name, ptp->p_name, ptp->p_name);
	}
	fprintf(fp, "{\t0,\t},\n");
	fprintf(fp, "};\n");

	(void) fclose(fp);
}

/*
 * make certain that this is a reasonable type of thing to connect to a nexus
 */
check_nexus(dev, num)
	register struct device *dev;
	int num;
{
	yyerror("don't grok 'nexus' for this machine.");
}

/*
 * make certain that this is a reasonable type of thing to connect to a slot
 */

check_slot(dev, num)
    register struct device *dev;
    int num;
{
    if ( strcmp(platform->name, "SYMMETRY") == 0 ) {
	if (!eq(dev->d_name, "mbad") &&
	    !eq(dev->d_name, "zdc") &&
	    !eq(dev->d_name, "sec")) {
		(void)sprintf(errbuf,
		    "unknown bus type `%s' for slot on %s",
		    dev->d_name, platform->name);
		yyerror(errbuf);
	}
    }
	else
	    yyerror("don't grok 'slot' for this machine.");
}

machinedep()
{
    if ( strcmp(platform->name, "SYMMETRY") == 0 ) 
	sqt_ioconf();

	/* else i386_ioconf(); */
}
