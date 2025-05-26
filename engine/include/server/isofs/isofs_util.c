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
 * $Log:	isofs_util.c,v $
 * Revision 2.2  92/07/08  16:17:34  mrt
 * 	BSDSS
 * 	[92/06/25  18:07:03  rwd]
 * 
 *
 */

int
isonum_711 (p)
char *p;
{
	return (*p & 0xff);
}

int
isonum_712 (p)
char *p;
{
	int val;

	val = *p;
	if (val & 0x80)
		val |= 0xffffff00;
	return (val);
}

int
isonum_721 (p)
char *p;
{
	return ((p[0] & 0xff) | ((p[1] & 0xff) << 8));
}

int
isonum_722 (p)
char *p;
{
	return (((p[0] & 0xff) << 8) | (p[1] & 0xff));
}

int
isonum_723 (p)
char *p;
{
#if 0
	if (p[0] != p[3] || p[1] != p[2]) {
		fprintf (stderr, "invalid format 7.2.3 number\n");
		exit (1);
	}
#endif
	return (isonum_721 (p));
}

int
isonum_731 (p)
char *p;
{
	return ((p[0] & 0xff)
		| ((p[1] & 0xff) << 8)
		| ((p[2] & 0xff) << 16)
		| ((p[3] & 0xff) << 24));
}

int
isonum_732 (p)
char *p;
{
	return (((p[0] & 0xff) << 24)
		| ((p[1] & 0xff) << 16)
		| ((p[2] & 0xff) << 8)
		| (p[3] & 0xff));
}

int
isonum_733 (p)
char *p;
{
	int i;

#if 0
	for (i = 0; i < 4; i++) {
		if (p[i] != p[7-i]) {
			fprintf (stderr, "bad format 7.3.3 number\n");
			exit (1);
		}
	}
#endif
	return (isonum_731 (p));
}

		
	
