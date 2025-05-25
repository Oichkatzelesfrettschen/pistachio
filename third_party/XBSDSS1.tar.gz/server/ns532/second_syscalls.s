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
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created from i386 BSDSS.
 *
 * $Log:	second_syscalls.s,v $
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
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

/*
 * system calls needed by second server to access the
 * primary server via unix system calls.
 * We just rename them unix_foo instead of foo not to
 * mix up with this server entries.
 */

#include "second_SYS.h"

RSYSCALL(open)

RSYSCALL(fstat)

RSYSCALL(access)

RSYSCALL(lseek)

RSYSCALL(write)

RSYSCALL(read)

RSYSCALL(close)

RSYSCALL(ioctl)

RSYSCALL(sigvec)

PSEUDO(_exit,exit)

	.data
	.globl	EX(second_errno)

EXT(second_errno):
	.long	0

	.text

	.globl  second_cerror

LCL(second_cerror):
	movd	r0, EX(second_errno)
	movd	-1, r0
	ret	0

