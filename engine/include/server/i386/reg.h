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
 * $Log:	reg.h,v $
 * Revision 2.2  92/05/25  14:42:52  rwd
 * 	Define NIPCREG always, nuke ipcreg.
 * 	[92/05/22            rwd]
 * 
 * Revision 2.1  92/04/21  17:19:00  rwd
 * BSDSS
 * 
 *
 */

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)reg.h	5.5 (Berkeley) 1/18/91
 */

/*
 * Location of the users' stored
 * registers within appropriate frame of 'trap' and 'syscall', relative to
 * base of stack frame.
 * Normal usage is u.u_ar0[XX] in kernel.
 */

/* When referenced during a trap/exception, registers are at these offsets */

#define	tES	(0)
#define	tDS	(1)
#define	tEDI	(2)
#define	tESI	(3)
#define	tEBP	(4)

#define	tEBX	(6)
#define	tEDX	(7)
#define	tECX	(8)
#define	tEAX	(9)

#define	tEIP	(12)
#define	tCS	(13)
#define	tEFLAGS	(14)
#define	tESP	(15)
#define	tSS	(16)

/* During a system call, registers are at these offsets instead of above. */

#define	sEDI	(0)
#define	sESI	(1)
#define	sEBP	(2)

#define	sEBX	(4)
#define	sEDX	(5)
#define	sECX	(6)
#define	sEAX	(7)
#define	sEFLAGS	(8)
#define	sEIP	(9)
#define	sCS	(10)
#define	sESP	(11)
#define	sSS	(12)

#define	PC	sEIP
#define	SP	sESP
#define	PS	sEFLAGS
#define	R0	sEDX
#define	R1	sECX
/*
 * Registers accessible to ptrace(2) syscall for debugger
 */
#define	NIPCREG 14
