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
 * Revision 2.2  92/06/25  17:29:52  mrt
 * 	Moved machine dependent code from mkioconf and mkglue to here
 * 	[92/06/17  13:45:13  mrt]
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

mips_ioconf()
{
	register struct device *dp, *mp, *np;
	register int slave;
	FILE *fp;
	char buf1[64], buf2[64];
	char *intv2();
	char *concat3();

	unlink(path("ioconf.c"));
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
	if (seen_mbii && seen_vme) {
		printf("can't have both vme and mbii devices\n");
		exit(1);
	}
	if (seen_mbii)
		fprintf(fp, "#include <mipsmbii/mbiivar.h>\n");
	if (seen_vme)
		fprintf(fp, "#include <mipsvme/vmevar.h>\n");
	fprintf(fp, "\n");
	fprintf(fp, "#define C	(caddr_t)\n");
	fprintf(fp, "#define NULL	0\n\n");
	if (!seen_mbii)
		goto checkvme;
	/*
	 * MBII stuff should go here
	 */

checkvme:
	if (!seen_vme)
		goto closefile;
	/*
	 * Now generate interrupt vectors for the vme bus
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_vec != 0) {
			struct idlst *ip;
			mp = dp->d_conn;
			if (mp == 0 || mp == TO_NEXUS || !eq(mp->d_name, "vme"))
				continue;
			if (is_declared(dp->d_name))
				continue;
			declare(dp->d_name);
			fprintf(fp, "extern struct vme_driver %sdriver;\n",
			    dp->d_name);
			fprintf(fp, "extern ");
			ip = dp->d_vec;
			for (;;) {
				fprintf(fp, "%s()", ip->id);
				ip = ip->id_next;
				if (ip == 0)
					break;
				fprintf(fp, ", ");
			}
			fprintf(fp, ";\n");
			fprintf(fp, "int (*_%sint[])() = { ", dp->d_name,
			    dp->d_unit);
			ip = dp->d_vec;
			for (;;) {
				fprintf(fp, "%s", ip->id);
				ip = ip->id_next;
				if (ip == 0)
					break;
				fprintf(fp, ", ");
			}
			fprintf(fp, ", 0 } ;\n\n");
		}
	}
	fprintf(fp, "\nstruct vme_ctlr vmminit[] = {\n");
	fprintf(fp,
"  /*          driver  ctlr alive        intr          addr    am */\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_type != CONTROLLER || mp == TO_NEXUS || mp == 0 ||
		    !eq(mp->d_name, "vme"))
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
		if (dp->d_addrmod == 0) {
			printf("must specify address modifier for %s%d\n",
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
"  {   %14s, %3d,    0, %11s, C 0x%08x, 0x%02x },\n",
		     concat3(buf1, "&", dp->d_name, "driver"),
		     dp->d_unit,
		     concat3(buf2, "_", dp->d_name, "int"),
		     dp->d_addr,
		     dp->d_addrmod);
	}
	fprintf(fp, "  {             NULL }\n};\n");
	/*
	 * vme devices
	 */
	fprintf(fp, "\nstruct vme_device vmdinit[] = {\n");
	fprintf(fp,
"/*       driver  unit ctlr slave      intr          addr    am dk       flags */\n"
	);
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_unit == QUES || dp->d_type != DEVICE || mp == 0 ||
		    mp == TO_NEXUS || mp->d_type == MASTER)
			continue;
		for (np = mp; np && np != TO_NEXUS; np = np->d_conn)
			if (eq(np->d_name, "vme"))
				break;
		if (np != 0 && np != TO_NEXUS && !eq(np->d_name, "vme"))
			continue;
		np = 0;
		if (eq(mp->d_name, "vme")) {
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
			if (dp->d_addrmod == 0) {
				printf(
			"must specify address modifier for device %s%d\n",
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
			slave = QUES;
		} else {
			if ((np = mp->d_conn) == 0) {
				printf("%s%d isn't connected to anything ",
				    mp->d_name, mp->d_unit);
				printf(", so %s%d is unattached\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive == UNKNOWN) {
				printf("must specify ``drive number'' ");
				printf("for %s%d\n", dp->d_name, dp->d_unit);
				continue;
			}
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
			if (dp->d_addrmod != 0) {
				printf("address modifiers should be given only ");
				printf("on controllers, not on %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			slave = dp->d_drive;
		}
		fprintf(fp,
"{%14s, %3d, %3s, %4d,%10s, C 0x%08x, 0x%02x, %1d, 0x%08x },\n",
		    concat3(buf1, "&",
		        eq(mp->d_name, "vme") ? dp->d_name : mp->d_name,
			"driver"),
		    dp->d_unit,
		    eq(mp->d_name, "vme") ? "-1" : qu(mp->d_unit),
		    slave,
		    intv2(dp),
		    dp->d_addr == UNKNOWN ? 0 : dp->d_addr,
		    dp->d_addrmod,
		    dp->d_dk,
		    dp->d_flags);
	}
	fprintf(fp, "{          NULL }\n};\n");
closefile:
	(void) fclose(fp);
}

char *
intv2(dev)
register struct device *dev;
{
	static char buf[20];

	if (dev->d_vec == 0)
		return ("NULL");
	(void) sprintf(buf, "_%sint", dev->d_name);
	return (buf);
}

char *
concat3(buf, p1, p2, p3)
char *buf;
char *p1, *p2, *p3;
{
	(void) sprintf(buf, "%s%s%s", p1, p2, p3);
	return (buf);
}

#define	MAXDEVS	100
#define	DEVLEN	10
char decl_devices[MAXDEVS][DEVLEN];

declare(cp)
register char *cp;
{
	register i;

	for (i = 0; i < MAXDEVS; i++)
		if (decl_devices[i][0] == 0) {
			strncpy(decl_devices, cp, DEVLEN);
			return;
		}
	printf("device table full, fix machinedep.c\n");
	exit(1);
}

is_declared(cp)
register char *cp;
{
	register i;

	for (i = 0; i < MAXDEVS; i++) {
		if (decl_devices[i][0] == 0)
			return(0);
		if (strncmp(decl_devices[i], cp, DEVLEN) == 0)
			return(1);
	}
	return(0);
}

/*
 * make certain that this is a reasonable type of thing to connect to a nexus
 */
check_nexus(dev, num)
	register struct device *dev;
	int num;
{
	if (!eq(dev->d_name, "vme") && !eq(dev->d_name, "mbii"))
		yyerror("only vme's and mbii's should be connected to the nexus");
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

machinedep()
{
	mips_ioconf();
}
