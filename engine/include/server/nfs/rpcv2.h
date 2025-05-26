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
 * $Log:	rpcv2.h,v $
 * Revision 2.1  92/04/21  17:15:03  rwd
 * BSDSS
 * 
 *
 */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
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
 *	@(#)rpcv2.h	7.4 (Berkeley) 6/28/90
 */

/*
 * Definitions for Sun RPC Version 2, from
 * "RPC: Remote Procedure Call Protocol Specification" RFC1057
 */

/* Version # */
#define	RPC_VER2	2

/* Authentication */
#define	RPCAUTH_NULL	0
#define	RPCAUTH_UNIX	1
#define	RPCAUTH_SHORT	2
#define	RPCAUTH_MAXSIZ	400
#define	RPCAUTH_UNIXGIDS 16

/* Rpc Constants */
#define	RPC_CALL	0
#define	RPC_REPLY	1
#define	RPC_MSGACCEPTED	0
#define	RPC_MSGDENIED	1
#define	RPC_PROGUNAVAIL	1
#define	RPC_PROGMISMATCH	2
#define	RPC_PROCUNAVAIL	3
#define	RPC_GARBAGE	4		/* I like this one */
#define	RPC_MISMATCH	0
#define	RPC_AUTHFAIL	1

/* Authentication failures */
#define	AUTH_BADCRED	1
#define	AUTH_REJECTCRED	2
#define	AUTH_BADVERF	3
#define	AUTH_REJECTVERF	4
#define	AUTH_TOOWEAK	5		/* Give em wheaties */

/* Sizes of rpc header parts */
#define	RPC_SIZ		24
#define	RPC_REPLYSIZ	28

/* RPC Prog definitions */
#define	RPCPROG_MNT	100005
#define	RPCMNT_VER1	1
#define	RPCMNT_MOUNT	1
#define	RPCMNT_DUMP	2
#define	RPCMNT_UMOUNT	3
#define	RPCMNT_UMNTALL	4
#define	RPCMNT_EXPORT	5
#define	RPCMNT_NAMELEN	255
#define	RPCMNT_PATHLEN	1024
#define	RPCPROG_NFS	100003
