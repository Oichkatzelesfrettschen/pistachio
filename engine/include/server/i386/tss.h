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
 * $Log:	tss.h,v $
 * Revision 2.2  92/05/25  14:43:11  rwd
 * 	Added for compatibilty for ptrace.
 * 	[92/05/23            rwd]
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
 *	@(#)tss.h	5.4 (Berkeley) 1/18/91
 */

/*
 * Intel 386 Context Data Type
 */

struct i386tss {
	int	tss_link;	/* actually 16 bits: top 16 bits must be zero */
	int	tss_esp0; 	/* kernel stack pointer priviledge level 0 */
#define	tss_ksp	tss_esp0
	int	tss_ss0;	/* actually 16 bits: top 16 bits must be zero */
	int	tss_esp1; 	/* kernel stack pointer priviledge level 1 */
	int	tss_ss1;	/* actually 16 bits: top 16 bits must be zero */
	int	tss_esp2; 	/* kernel stack pointer priviledge level 2 */
	int	tss_ss2;	/* actually 16 bits: top 16 bits must be zero */
	/* struct  ptd *tss_cr3; 	/* page table directory */
	int	tss_cr3; 	/* page table directory */
#define	tss_ptd	tss_cr3
	int	tss_eip; 	/* program counter */
#define	tss_pc	tss_eip
	int	tss_eflags; 	/* program status longword */
#define	tss_psl	tss_eflags
	int	tss_eax; 
	int	tss_ecx; 
	int	tss_edx; 
	int	tss_ebx; 
	int	tss_esp; 	/* user stack pointer */
#define	tss_usp	tss_esp
	int	tss_ebp; 	/* user frame pointer */
#define	tss_fp	tss_ebp
	int	tss_esi; 
	int	tss_edi; 
	int	tss_es;		/* actually 16 bits: top 16 bits must be zero */
	int	tss_cs;		/* actually 16 bits: top 16 bits must be zero */
	int	tss_ss;		/* actually 16 bits: top 16 bits must be zero */
	int	tss_ds;		/* actually 16 bits: top 16 bits must be zero */
	int	tss_fs;		/* actually 16 bits: top 16 bits must be zero */
	int	tss_gs;		/* actually 16 bits: top 16 bits must be zero */
	int	tss_ldt;	/* actually 16 bits: top 16 bits must be zero */
	int	tss_ioopt;	/* options & io offset bitmap: currently zero */
				/* XXX unimplemented .. i/o permission bitmap */
};
