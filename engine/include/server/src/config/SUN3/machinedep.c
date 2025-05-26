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
 * Revision 2.2  92/06/25  17:30:04  mrt
 * 	Moved machine dependent code from mkioconf and mkglue to here
 * 	[92/06/17  13:45:23  mrt]
 * 
 * Revision 2.6  91/06/19  11:58:50  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:46:13  rvb]
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

#define SP_OBIO	0x0004	/* on board i/o (for sun/autoconf.h) */

#define	VEC_LO	64
#define	VEC_HI	255

check_vector(vec)
	register struct idlst *vec;
{

	if (vec->id_vec == 0)
		fprintf(stderr, "vector number for %s not given\n", vec->id);
	else if (vec->id_vec < VEC_LO || vec->id_vec > VEC_HI)
		fprintf(stderr,
			"vector number %d for %s is not between %d and %d\n",
			vec->id_vec, vec->id, VEC_LO, VEC_HI);
}

sun_ioconf()
{
	register struct device *dp, *mp;
	register int slave;
	register struct idlst *vp;
	FILE *fp;

	fp = fopen(path("ioconf.c"), "w");
	if (fp == 0) {
		perror(path("ioconf.c"));
		exit(1);
	}
/*MACH_KERNEL*/
	fprintf(fp, "#ifndef  MACH_KERNEL\n");
/*MACH_KERNEL*/
	fprintf(fp, "#include <sys/param.h>\n");
	fprintf(fp, "#include <sys/buf.h>\n");
	fprintf(fp, "#include <sys/map.h>\n");
	fprintf(fp, "#include <sys/vm.h>\n");
/*MACH_KERNEL*/
	fprintf(fp, "#endif   MACH_KERNEL\n");
/*MACH_KERNEL*/
	fprintf(fp, "\n");
	fprintf(fp, "#include <sundev/mbvar.h>\n");
	fprintf(fp, "\n");
	fprintf(fp, "#define C (caddr_t)\n\n");
	fprintf(fp, "\n");

	/*
	 * Now generate interrupt vectors for the Mainbus
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp == TO_NEXUS || mp == 0 || mp->d_conn != TO_NEXUS)
			continue;
		fprintf(fp, "extern struct mb_driver %sdriver;\n",
			    dp->d_name);
		if (dp->d_vec != 0) {
			if (dp->d_pri == 0)
				fprintf(stderr,
				    "no priority specified for %s%d\n",
				    dp->d_name, dp->d_unit);
			fprintf(fp, "extern ");
			for (vp = dp->d_vec;;) {
				if ( 0 ) /* (machine == MACHINE_SUN4) */
					fprintf(fp, "%s()", vp->id);
				else
					fprintf(fp, "X%s%d()",
						vp->id, dp->d_unit);
				vp = vp->id_next;
				if (vp == 0)
					break;
				fprintf(fp, ", ");
			}
			fprintf(fp, ";\n");

			for (vp = dp->d_vec; vp; vp = vp->id_next) {
				fprintf(fp, "int V%s%d = %d;\n",
				    vp->id, dp->d_unit, dp->d_unit);
			}

			fprintf(fp, "struct vec %s[] = { ", intv(dp));
			for (vp = dp->d_vec; vp != 0; vp = vp->id_next) {
				if ( 0 ) /* (machine == MACHINE_SUN4) */
					fprintf(fp, "{ %s, %d, &V%s%d }, ",
						vp->id, vp->id_vec,
						vp->id, dp->d_unit);
				else
				fprintf(fp, "{ X%s%d, %d, &V%s%d }, ",
					vp->id, dp->d_unit, vp->id_vec,
					vp->id, dp->d_unit);
				check_vector(vp);
			}
			fprintf(fp, "0 };\n");
		}
	}

	/*
	 * Now spew forth the mb_ctlr structures
	 */
	fprintf(fp, "\nstruct mb_ctlr mbcinit[] = {\n");
	fprintf(fp,
"/* driver,\tctlr,\talive,\taddress,\tintpri,\t intr,\tspace */\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_type != CONTROLLER || mp == TO_NEXUS || mp == 0 ||
		    mp->d_conn != TO_NEXUS)
			continue;
		if (dp->d_addr == UNKNOWN) {
			printf("must specify csr address for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
			printf("drives need their own entries; ");
			printf("don't specify drive or slave for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_flags) {
			printf("controllers (e.g. %s%d) don't have flags, ");
			printf("only devices do\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if ( 0 ) /* machine == MACHINE_SUN4) */
		fprintf(fp,
		"{ &%sdriver,\t%d,\t0,\tC 0x%08x,\t%d,\t%s, 0x%x },\n",
		    dp->d_name, dp->d_unit, dp->d_addr,
		    (dp->d_bus==SP_OBIO) ? (dp->d_pri << 1) : (dp->d_pri<<1)-1,
		    intv(dp), ((dp->d_mach << 16) | dp->d_bus));
		else
			fprintf(fp,
		"{ &%sdriver,\t%d,\t0,\tC 0x%08x,\t%d,\t%s, 0x%x },\n",
		    dp->d_name, dp->d_unit, dp->d_addr,
		    dp->d_pri, intv(dp), ((dp->d_mach << 16) | dp->d_bus));
	}
	fprintf(fp, "\t0\n};\n");

	/*
	 * Now we go for the mb_device stuff
	 */
	fprintf(fp, "\nstruct mb_device mbdinit[] = {\n");
	fprintf(fp,
"/* driver,\tunit, ctlr, slave, address,      pri, dk, flags, intr, space */\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_unit == QUES || dp->d_type != DEVICE || mp == 0 ||
		    mp == TO_NEXUS || mp->d_type == MASTER)
			continue;
		if (mp->d_conn == TO_NEXUS) {
			if (dp->d_addr == UNKNOWN) {
				printf("must specify csr for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
				printf("drives/slaves can be specified only ");
				printf("for controllers, not for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			slave = QUES;
		} else {
			if (mp->d_conn == 0) {
				printf("%s%d isn't connected to anything, ",
				    mp->d_name, mp->d_unit);
				printf("so %s%d is unattached\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive == UNKNOWN) {
				printf("must specify ``drive number'' for %s%d\n",
				   dp->d_name, dp->d_unit);
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
			if (dp->d_pri != 0) {
				printf("interrupt priority should not be ");
				printf("given for drive %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_addr != UNKNOWN) {
				printf("csr addresses should be given only");
				printf(" on controllers, not on %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			slave = dp->d_drive;
		}
		if ( 0 ) /* (machine == MACHINE_SUN4) */
		fprintf(fp,
"{ &%sdriver,\t%d,  %s,   %2d,     C 0x%08x, %d,   %d, 0x%x, %s, 0x%x },\n",
		    mp->d_conn == TO_NEXUS? dp->d_name : mp->d_name, dp->d_unit,
		    mp->d_conn == TO_NEXUS? " -1" : qu(mp->d_unit),
		    slave,
		    dp->d_addr == UNKNOWN? 0 : dp->d_addr,
		    dp->d_pri * 2, dp->d_dk, dp->d_flags, intv(dp),
		    ((dp->d_mach << 16) | dp->d_bus));
		else
			fprintf(fp,
"{ &%sdriver,\t%d,  %s,   %2d,     C 0x%08x, %d,   %d, 0x%x, %s, 0x%x },\n",
		    mp->d_conn == TO_NEXUS? dp->d_name : mp->d_name, dp->d_unit,
		    mp->d_conn == TO_NEXUS? " -1" : qu(mp->d_unit),
		    slave,
		    dp->d_addr == UNKNOWN? 0 : dp->d_addr,
		    dp->d_pri, dp->d_dk, dp->d_flags, intv(dp),
		    ((dp->d_mach << 16) | dp->d_bus));
	}
	fprintf(fp, "\t0\n};\n");
	pseudo_inits(fp);
	(void) fclose(fp);
}

pseudo_inits(fp)
	FILE *fp;
{
	register struct device *dp;
	int count;

#ifdef	notdef
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_type != PSEUDO_DEVICE || dp->d_init == 0)
			continue;
		fprintf(fp, "extern %s();\n", dp->d_init);
	}
#endif	notdef
	fprintf(fp, "struct pseudo_init {\n");
	fprintf(fp, "\tint\tps_count;\n\tint\t(*ps_func)();\n");
	fprintf(fp, "} pseudo_inits[] = {\n");
#ifdef	notdef
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_type != PSEUDO_DEVICE || dp->d_init == 0)
			continue;
		count = dp->d_slave;
		if (count <= 0)
			count = 1;
		fprintf(fp, "\t%d,\t%s,\n", count, dp->d_init);
	}
#endif	notdef
	fprintf(fp, "\t0,\t0,\n};\n");
}

/*
 * make certain that this is a reasonable type of thing to connect to a nexus
 */
check_nexus(dev, num)
	register struct device *dev;
	int num;
{
	if (!eq(dev->d_name, "virtual") &&
	    !eq(dev->d_name, "obmem") &&
	    !eq(dev->d_name, "obio") &&
	    !eq(dev->d_name, "mbmem") &&
	    !eq(dev->d_name, "mbio") &&
	    !eq(dev->d_name, "vme16d16") &&
	    !eq(dev->d_name, "vme24d16") &&
	    !eq(dev->d_name, "vme32d16") &&
	    !eq(dev->d_name, "vme16d32") &&
	    !eq(dev->d_name, "vme24d32") &&
	    !eq(dev->d_name, "vme32d32")) {
		(void)sprintf(errbuf,
		    "unknown bus type `%s' for nexus connection on %s",
		    dev->d_name, conftypename);
		yyerror(errbuf);
	}
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

/*
 * bi_info gives the magic number used to construct the token for
 * the autoconf code.  bi_max is the maximum value (across all
 * machine types for a given architecture) that a given "bus 
 * type" can legally have.
 */
struct bus_info {
	char    *bi_name;
	u_short bi_info;
	u_int   bi_max;
};

struct bus_info sun3_info[] = {
	{ "virtual",    0x0001, (1<<32)-1 },
	{ "obmem",      0x0002, (1<<32)-1 },
	{ "obio",       0x0004, (1<<21)-1 },
	{ "vme16d16",   0x0100, (1<<16)-1 },
	{ "vme24d16",   0x0200, (1<<24)-(1<<16)-1 },
	{ "vme32d16",   0x0400, (1<<32)-(1<<24)-1 },
	{ "vme16d32",   0x1000, (1<<16) },
	{ "vme24d32",   0x2000, (1<<24)-(1<<16)-1 },
	{ "vme32d32",   0x4000, (1<<32)-(1<<24)-1 },
	{ (char *)0,    0,      0 }
};

bus_encode(addr, dp)
        u_int addr;
	register struct device *dp;
{
	register char *busname;
	register struct bus_info *bip;
	register int num;

	bip = sun3_info;

        if (dp->d_conn == TO_NEXUS || dp->d_conn == 0) {
		yyerror("bad connection");
		exit(1);
	}

        busname = dp->d_conn->d_name;
        num = dp->d_conn->d_unit;

        for (; bip->bi_name != 0; bip++)
                if (eq(busname, bip->bi_name))
                        break;

        if (bip->bi_name == 0) {
                (void)sprintf(errbuf, "bad bus type '%s' for machine %s",
                        busname, conftypename);
                yyerror(errbuf);
        } else if (addr > bip->bi_max) {
                (void)sprintf(errbuf,
                        "0x%x exceeds maximum address 0x%x allowed for %s",
                        addr, bip->bi_max, busname);
                yyerror(errbuf);
        } else {
                dp->d_bus = bip->bi_info;       /* set up bus type info */
                if (num != QUES)
                        /*
                         * Set up cpu type since the connecting
                         * bus type is not wildcarded.
                         */
                        dp->d_mach = num;
        }
}

dev_param(dp, str, num)
	register struct device *dp;
	register char *str;
	long	num;
{
	if (strcmp(str, "csr") == 0)
		bus_encode(num, dp);
/*	else
		yyerror("bad device spec");
*/
}


/*
 * print an interrupt handler for mainbus
 */
dump_mb_handler(fp, vec, number)
	register FILE *fp;
	register struct idlst *vec;
	int number;
{
	fprintf(fp, "\tVECINTR(_X%s%d, _%s, _V%s%d)\n",
		vec->id, number, vec->id, vec->id, number);
}

mbglue()
{
	register FILE *fp;
	char *name = "mbglue.s";

	fp = fopen(path(name), "w");
	if (fp == 0) {
	    perror(path(name));
	    exit(1);
	}
	fprintf(fp, "#include <machine/asm_linkage.h>\n\n");
	glue(fp, dump_mb_handler);
	(void) fclose(fp);
}

glue(fp, dump_handler)
	register FILE *fp;
	register int (*dump_handler)();
{
	register struct device *dp, *mp;

	for (dp = dtab; dp != 0; dp = dp->d_next) {
	    mp = dp->d_conn;
	    if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
		struct idlst *vd, *vd2;

		for (vd = dp->d_vec; vd; vd = vd->id_next) {
		    for (vd2 = dp->d_vec; vd2; vd2 = vd2->id_next) {
			if (vd2 == vd) {
			    (void)(*dump_handler)(fp, vd, dp->d_unit);
			    break;
			}
			if (!strcmp(vd->id, vd2->id))
			    break;
		    }
		}
	    }
	}
}

machinedep()
{
	sun_ioconf();           /* Print ioconf.c */
	mbglue();               /* Create mbglue.s */
}
