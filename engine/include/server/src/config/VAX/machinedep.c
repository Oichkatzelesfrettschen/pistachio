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
 * Revision 2.2  92/06/25  17:30:14  mrt
 * 	Moved machine dependent code from mkioconf and mkglue to here
 * 	[92/06/17  13:45:34  mrt]
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
#include <ctype.h>

/*
 * build the ioconf.c file
 */
extern char	*qu();
extern char	*intv();

vax_ioconf()
{
	register struct device *dp, *mp, *np;
	register int uba_n, slave;
	FILE *fp;

	fp = fopen(path("ioconf.c"), "w");
	if (fp == 0) {
		perror(path("ioconf.c"));
		exit(1);
	}
/*MACH_KERNEL*/
	fprintf(fp, "#ifndef  MACH_KERNEL\n");
/*MACH_KERNEL*/
	fprintf(fp, "#include <machine/pte.h>\n");
	fprintf(fp, "#include <sys/param.h>\n");
	fprintf(fp, "#include <sys/buf.h>\n");
	fprintf(fp, "#include <sys/map.h>\n");
	fprintf(fp, "#include <sys/vm.h>\n");
/*MACH_KERNEL*/
	fprintf(fp, "#endif   MACH_KERNEL\n");
/*MACH_KERNEL*/
	fprintf(fp, "\n");
	fprintf(fp, "#include <vaxmba/mbavar.h>\n");
	fprintf(fp, "#include <vaxuba/ubavar.h>\n\n");
	fprintf(fp, "\n");
	fprintf(fp, "#define C (caddr_t)\n\n");
	/*
	 * First print the mba initialization structures
	 */
	if (seen_mba) {
		for (dp = dtab; dp != 0; dp = dp->d_next) {
			mp = dp->d_conn;
			if (mp == 0 || mp == TO_NEXUS ||
			    !eq(mp->d_name, "mba"))
				continue;
			fprintf(fp, "extern struct mba_driver %sdriver;\n",
			    dp->d_name);
		}
		fprintf(fp, "\nstruct mba_device mbdinit[] = {\n");
		fprintf(fp, "\t/* Device,  Unit, Mba, Drive, Dk */\n");
		for (dp = dtab; dp != 0; dp = dp->d_next) {
			mp = dp->d_conn;
			if (dp->d_unit == QUES || mp == 0 ||
			    mp == TO_NEXUS || !eq(mp->d_name, "mba"))
				continue;
			if (dp->d_addr != UNKNOWN) {
				printf("can't specify csr address on mba for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_vec != 0) {
				printf("can't specify vector for %s%d on mba\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive == UNKNOWN) {
				printf("drive not specified for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_slave != UNKNOWN) {
				printf("can't specify slave number for %s%d\n", 
				    dp->d_name, dp->d_unit);
				continue;
			}
			fprintf(fp, "\t{ &%sdriver, %d,   %s,",
				dp->d_name, dp->d_unit, qu(mp->d_unit));
			fprintf(fp, "  %s,  %d },\n",
				qu(dp->d_drive), dp->d_dk);
		}
		fprintf(fp, "\t0\n};\n\n");
		/*
		 * Print the mbsinit structure
		 * Driver Controller Unit Slave
		 */
		fprintf(fp, "struct mba_slave mbsinit [] = {\n");
		fprintf(fp, "\t/* Driver,  Ctlr, Unit, Slave */\n");
		for (dp = dtab; dp != 0; dp = dp->d_next) {
			/*
			 * All slaves are connected to something which
			 * is connected to the massbus.
			 */
			if ((mp = dp->d_conn) == 0 || mp == TO_NEXUS)
				continue;
			np = mp->d_conn;
			if (np == 0 || np == TO_NEXUS ||
			    !eq(np->d_name, "mba"))
				continue;
			fprintf(fp, "\t{ &%sdriver, %s",
			    mp->d_name, qu(mp->d_unit));
			fprintf(fp, ",  %2d,    %s },\n",
			    dp->d_unit, qu(dp->d_slave));
		}
		fprintf(fp, "\t0\n};\n\n");
	}
	/*
	 * Now generate interrupt vectors for the unibus
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_vec != 0) {
			struct idlst *ip;
			mp = dp->d_conn;
			if (mp == 0 || mp == TO_NEXUS ||
			    !eq(mp->d_name, "uba"))
				continue;
			fprintf(fp,
			    "extern struct uba_driver %sdriver;\n",
			    dp->d_name);
			fprintf(fp, "extern ");
			ip = dp->d_vec;
			for (;;) {
				fprintf(fp, "X%s%d()", ip->id, dp->d_unit);
				ip = ip->id_next;
				if (ip == 0)
					break;
				fprintf(fp, ", ");
			}
			fprintf(fp, ";\n");
			fprintf(fp, "int\t (*%sint%d[])() = { ", dp->d_name,
			    dp->d_unit);
			ip = dp->d_vec;
			for (;;) {
				fprintf(fp, "X%s%d", ip->id, dp->d_unit);
				ip = ip->id_next;
				if (ip == 0)
					break;
				fprintf(fp, ", ");
			}
			fprintf(fp, ", 0 } ;\n");
		}
	}
	fprintf(fp, "\nstruct uba_ctlr ubminit[] = {\n");
	fprintf(fp, "/*\t driver,\tctlr,\tubanum,\talive,\tintr,\taddr */\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_type != CONTROLLER || mp == TO_NEXUS || mp == 0 ||
		    !eq(mp->d_name, "uba"))
			continue;
		if (dp->d_vec == 0) {
			printf("must specify vector for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_addr == UNKNOWN) {
			printf("must specify csr address for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
			printf("drives need their own entries; dont ");
			printf("specify drive or slave for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_flags) {
			printf("controllers (e.g. %s%d) ",
			    dp->d_name, dp->d_unit);
			printf("don't have flags, only devices do\n");
			continue;
		}
		fprintf(fp,
		    "\t{ &%sdriver,\t%d,\t%s,\t0,\t%sint%d, C 0%o },\n",
		    dp->d_name, dp->d_unit, qu(mp->d_unit),
		    dp->d_name, dp->d_unit, dp->d_addr);
	}
	fprintf(fp, "\t0\n};\n");
/* unibus devices */
	fprintf(fp, "\nstruct uba_device ubdinit[] = {\n");
	fprintf(fp,
"\t/* driver,  unit, ctlr,  ubanum, slave,   intr,    addr,    dk, flags*/\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_unit == QUES || dp->d_type != DEVICE || mp == 0 ||
		    mp == TO_NEXUS || mp->d_type == MASTER ||
		    eq(mp->d_name, "mba"))
			continue;
		np = mp->d_conn;
		if (np != 0 && np != TO_NEXUS && eq(np->d_name, "mba"))
			continue;
		np = 0;
		if (eq(mp->d_name, "uba")) {
			if (dp->d_vec == 0) {
				printf("must specify vector for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_addr == UNKNOWN) {
				printf("must specify csr for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
				printf("drives/slaves can be specified ");
				printf("only for controllers, ");
				printf("not for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			uba_n = mp->d_unit;
			slave = QUES;
		} else {
			if ((np = mp->d_conn) == 0) {
				printf("%s%d isn't connected to anything ",
				    mp->d_name, mp->d_unit);
				printf(", so %s%d is unattached\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			uba_n = np->d_unit;
			if (dp->d_drive == UNKNOWN) {
				printf("must specify ``drive number'' ");
				printf("for %s%d\n", dp->d_name, dp->d_unit);
				continue;
			}
			/* NOTE THAT ON THE UNIBUS ``drive'' IS STORED IN */
			/* ``SLAVE'' AND WE DON'T WANT A SLAVE SPECIFIED */
			if (dp->d_slave != UNKNOWN) {
				printf("slave numbers should be given only ");
				printf("for massbus tapes, not for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_vec != 0) {
				printf("interrupt vectors should not be ");
				printf("given for drive %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_addr != UNKNOWN) {
				printf("csr addresses should be given only ");
				printf("on controllers, not on %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			slave = dp->d_drive;
		}
		fprintf(fp, "\t{ &%sdriver,  %2d,   %s,",
		    eq(mp->d_name, "uba") ? dp->d_name : mp->d_name, dp->d_unit,
		    eq(mp->d_name, "uba") ? " -1" : qu(mp->d_unit));
		fprintf(fp, "  %s,    %2d,   %s, C 0%-6o,  %d,  0x%x },\n",
		    qu(uba_n), slave, intv(dp),
		    dp->d_addr == UNKNOWN ? 0 : dp->d_addr, dp->d_dk,
		    dp->d_flags);
	}
	fprintf(fp, "\t0\n};\n");
	(void) fclose(fp);
}

/*
 * make certain that this is a reasonable type of thing to connect to a nexus
 */
check_nexus(dev, num)
	register struct device *dev;
	int num;
{
	if (!eq(dev->d_name, "uba") && !eq(dev->d_name, "mba"))
		yyerror("only uba's and mba's should be connected to the nexus");
	if (num != QUES)
		yyerror("can't give specific nexus numbers");
}

/*
 * make certain that this is a reasonable type of thing to connect to a slot
 */

check_slot(dev, num)
	register struct device *dev;
	int num;
{
	yyerror("don't grok 'slot' for this machine.");
}

dev_param(dp, str, num)
	register struct device *dp;
	register char *str;
	long	num;
{
}

/*
 * Create the UNIBUS interrupt vector glue file.
 */
ubglue()
{
	register FILE *fp, *gp;
	register struct device *dp, *mp;

	fp = fopen(path("ubglue.s"), "w");
	if (fp == 0) {
		perror(path("ubglue.s"));
		exit(1);
	}
	gp = fopen(path("ubvec.s"), "w");
	if (gp == 0) {
		perror(path("ubvec.s"));
		exit(1);
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_ubavec(fp, id->id,
						    dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	dump_std(fp, gp);
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_intname(fp, id->id,
							dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	dump_ctrs(fp);
	(void) fclose(fp);
	(void) fclose(gp);
}

static int cntcnt = 0;		/* number of interrupt counters allocated */

/*
 * Print a UNIBUS interrupt vector.
 */
dump_ubavec(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	char nbuf[80];
	register char *v = nbuf;

	(void) sprintf(v, "%s%d", vector, number);
	fprintf(fp, "\t.globl\t_X%s\n\t.align\t2\n_X%s:\n",
	    v, v);
	fprintf(fp,"\tTIM_PUSHR(0)\n");
	fprintf(fp, "\tincl\t_fltintrcnt+(4*%d)\n", cntcnt++);
	if (strncmp(vector, "dzx", 3) == 0)
		fprintf(fp, "\tmovl\t$%d,r0\n\tjmp\tdzdma\n\n", number);
	else {
		if (strncmp(vector, "uur", 3) == 0) {
			fprintf(fp, "#ifdef UUDMA\n");
			fprintf(fp, "\tmovl\t$%d,r0\n\tjsb\tuudma\n",
				    number);
			fprintf(fp, "#endif\n");
		}
		fprintf(fp, "\tpushl\t$%d\n", number);
		fprintf(fp, "\tcalls\t$1,_%s\n",vector);
		fprintf(fp, "\tCOUNT(V_INTR)\n");
		fprintf(fp, "\tTSREI_POPR\n");
	}
}

static	char *vaxinames[] = {
	"clock", "cnr", "cnx", "tur", "tux",
	"mba0", "mba1", "mba2", "mba3",
	"uba0", "uba1", "uba2", "uba3"
};
static	struct stdintrs {
	char	**si_names;	/* list of standard interrupt names */
	int	si_n;		/* number of such names */
} stdintrs = {
	vaxinames, sizeof (vaxinames) / sizeof (vaxinames[0])
};
/*
 * Start the interrupt name table with the names
 * of the standard vectors not directly associated
 * with a bus.  Also, dump the defines needed to
 * reference the associated counters into a separate
 * file which is prepended to locore.s.
 */
dump_std(fp, gp)
	register FILE *fp, *gp;
{
	register struct stdintrs *si = &stdintrs;
	register char **cpp;
	register int i;

	fprintf(fp, "\n\t.globl\t_intrnames\n");
	fprintf(fp, "\n\t.globl\t_eintrnames\n");
	fprintf(fp, "\t.data\n");
	fprintf(fp, "_intrnames:\n");
	cpp = si->si_names;
	for (i = 0; i < si->si_n; i++) {
		register char *cp, *tp;
		char buf[80];

		cp = *cpp;
		if (cp[0] == 'i' && cp[1] == 'n' && cp[2] == 't') {
			cp += 3;
			if (*cp == 'r')
				cp++;
		}
		for (tp = buf; *cp; cp++)
			if (islower(*cp))
				*tp++ = toupper(*cp);
			else
				*tp++ = *cp;
		*tp = '\0';
		fprintf(gp, "#define\tI_%s\t%d\n", buf, i*sizeof (long));
		fprintf(fp, "\t.asciz\t\"%s\"\n", *cpp);
		cpp++;
	}
}

dump_intname(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	register char *cp = vector;

	fprintf(fp, "\t.asciz\t\"");
	/*
	 * Skip any "int" or "intr" in the name.
	 */
	while (*cp)
		if (cp[0] == 'i' && cp[1] == 'n' &&  cp[2] == 't') {
			cp += 3;
			if (*cp == 'r')
				cp++;
		} else {
			putc(*cp, fp);
			cp++;
		}
	fprintf(fp, "%d\"\n", number);
}

/*
 * Reserve space for the interrupt counters.
 */
dump_ctrs(fp)
	register FILE *fp;
{
	struct stdintrs *si = &stdintrs;

	fprintf(fp, "_eintrnames:\n");
	fprintf(fp, "\n\t.globl\t_intrcnt\n");
	fprintf(fp, "\n\t.globl\t_eintrcnt\n");
	fprintf(fp, "\t.align 2\n");
	fprintf(fp, "_intrcnt:\n");
	fprintf(fp, "\t.space\t4 * %d\n", si->si_n);
	fprintf(fp, "_fltintrcnt:\n");
	fprintf(fp, "\t.space\t4 * %d\n", cntcnt);
	fprintf(fp, "_eintrcnt:\n\n");
	fprintf(fp, "\t.text\n");
}

machinedep()
{
	vax_ioconf();		/* Print ioconf.c */
	ubglue();		/* Create ubglue.s */
}
