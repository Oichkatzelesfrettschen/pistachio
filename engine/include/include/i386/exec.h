#pragma once
/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)exec.h	8.1 (Berkeley) 6/11/93
 */

/* Size of a page in an object file. */
#define	__LDPGSZ	4096

/* Valid magic number check. */
#define	N_BADMAG(ex) \
	((ex).a_magic != NMAGIC && (ex).a_magic != OMAGIC && \
	    (ex).a_magic != ZMAGIC)

/* Address of the bottom of the text segment. */
#define N_TXTADDR(X)	0

/* Address of the bottom of the data segment. */
#define N_DATADDR(ex) \
	(N_TXTADDR(ex) + ((ex).a_magic == OMAGIC ? (ex).a_text \
	: __LDPGSZ + ((ex).a_text - 1 & ~(__LDPGSZ - 1))))

/* Text segment offset. */
#define	N_TXTOFF(ex) \
	((ex).a_magic == ZMAGIC ? __LDPGSZ : sizeof(struct exec))

/* Data segment offset. */
#define	N_DATOFF(ex) \
	(N_TXTOFF(ex) + ((ex).a_magic != ZMAGIC ? (ex).a_text : \
	__LDPGSZ + ((ex).a_text - 1 & ~(__LDPGSZ - 1))))

/* Symbol table offset. */
#define N_SYMOFF(ex) \
	(N_TXTOFF(ex) + (ex).a_text + (ex).a_data + (ex).a_trsize + \
	    (ex).a_drsize)

/* String table offset. */
#define	N_STROFF(ex) 	(N_SYMOFF(ex) + (ex).a_syms)

/* Description of the object file header (a.out format). */
struct exec {
#define	OMAGIC	0407		/* old impure format */
#define	NMAGIC	0410		/* read-only text */
#define	ZMAGIC	0413		/* demand load format */
#define QMAGIC	0314		/* demand load format. Header in text. */
	unsigned int	a_magic;	/* magic number */

	unsigned int	a_text;		/* text segment size */
	unsigned int	a_data;		/* initialized data size */
	unsigned int	a_bss;		/* uninitialized data size */
	unsigned int	a_syms;		/* symbol table size */
	unsigned int	a_entry;	/* entry point */
	unsigned int	a_trsize;	/* text relocation size */
	unsigned int	a_drsize;	/* data relocation size */
};
