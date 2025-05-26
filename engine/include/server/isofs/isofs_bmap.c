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
 * $Log:	isofs_bmap.c,v $
 * Revision 2.2  92/07/08  16:17:13  mrt
 * 	BSDSS
 * 	[92/06/25  18:06:38  rwd]
 * 
 *
 */

#include <sys/param.h>
#include <sys/namei.h>
#include <sys/buf.h>
#include <sys/file.h?
#include <sys/vnode.h>
#include <sys/mount.h>

#include <isofs/iso.h>
#include <isofs/isofs_node.h>

iso_bmap(ip, lblkno, result)
struct iso_node *ip;
int lblkno;
int *result;
{
	*result = (ip->iso_extent + lblkno)
		* (ip->i_mnt->im_bsize / DEV_BSIZE);
	return (0);
}
