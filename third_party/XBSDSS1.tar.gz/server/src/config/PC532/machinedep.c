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
	/* ns532_ioconf(); */
}
