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
 * $Log:	limits.h,v $
 * Revision 2.1  92/04/21  17:18:53  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
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
 *	@(#)limits.h	7.2 (Berkeley) 6/28/90
 */

#define	CHAR_BIT	8		/* number of bits in a char */
#define	CLK_TCK		60		/* ticks per second */
#define	MB_LEN_MAX	1		/* no multibyte characters */

#define	SCHAR_MIN	0x80		/* max value for a signed char */
#define	SCHAR_MAX	0x7f		/* min value for a signed char */

#define	UCHAR_MAX	0xff		/* max value for an unsigned char */
#define	CHAR_MAX	0x7f		/* max value for a char */
#define	CHAR_MIN	0x80		/* min value for a char */

#define	USHRT_MAX	0xffff		/* max value for an unsigned short */
#define	SHRT_MAX	0x7fff		/* max value for a short */
#define	SHRT_MIN	0x8000		/* min value for a short */

#define	UINT_MAX	0xffffffff	/* max value for an unsigned int */
#define	INT_MAX		0x7fffffff	/* max value for an int */
#define	INT_MIN		0x80000000	/* min value for an int */

#define	ULONG_MAX	0xffffffff	/* max value for an unsigned long */
#define	LONG_MAX	0x7fffffff	/* max value for a long */
#define	LONG_MIN	0x80000000	/* min value for a long */
