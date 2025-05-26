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
 * $Log:	second_syscalls.s,v $
# Revision 2.1  92/04/21  17:19:17  rwd
# BSDSS
# 
 *
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

SYSCALL(open)
	ret

SYSCALL(fstat)
	ret

SYSCALL(access)
	ret

SYSCALL(lseek)
	ret

SYSCALL(write)
	ret

SYSCALL(read)
	ret

SYSCALL(close)
	ret

SYSCALL(ioctl)
	ret

SYSCALL(sigvec)
	ret

PSEUDO(_exit,exit)

	.data
	.globl	EXT(second_errno)

EXT(second_errno):
	.long	0

	.text

	.globl  LCL(second_cerror)

LCL(second_cerror):
	movl	%eax, EXT(second_errno)
	movl	$-1,%eax
	ret

