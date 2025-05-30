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
 * $Log:	pcb.h,v $
 * Revision 2.2  92/05/25  14:42:46  rwd
 * 	Added for comptability for ptrace.
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
 *	@(#)pcb.h	5.10 (Berkeley) 5/12/91
 */

/*
 * Intel 386 process control block
 */
#include "machine/tss.h"
#include "machine/npx.h"

struct pcb {
	struct	i386tss pcb_tss;
#define	pcb_ksp	pcb_tss.tss_esp0
#define	pcb_ptd	pcb_tss.tss_cr3
#define	pcb_cr3	pcb_ptd
#define	pcb_pc	pcb_tss.tss_eip
#define	pcb_psl	pcb_tss.tss_eflags
#define	pcb_usp	pcb_tss.tss_esp
#define	pcb_fp	pcb_tss.tss_ebp
#ifdef	notyet
	u_char	pcb_iomap[NPORT/sizeof(u_char)]; /* i/o port bitmap */
#endif
	struct	save87	pcb_savefpu;	/* floating point state for 287/387 */
	struct	emcsts	pcb_saveemc;	/* Cyrix EMC state */
/*
 * Software pcb (extension)
 */
	int	pcb_flags;
#define	FP_WASUSED	0x01	/* floating point has been used in this proc */
#define	FP_NEEDSSAVE	0x02	/* needs save on next context switch */
#define	FP_NEEDSRESTORE	0x04	/* need restore on next DNA fault */
#define	FP_USESEMC	0x08	/* process uses EMC memory-mapped mode */
#define	FM_TRAP		0x10	/* process entered kernel on a trap frame */
	short	pcb_iml;	/* interrupt mask level */
	caddr_t	pcb_onfault;	/* copyin/out fault recovery */
	long	pcb_sigc[8];	/* XXX signal code trampoline */
	int	pcb_cmap2;	/* XXX temporary PTE - will prefault instead */
};

#ifdef KERNEL
struct pcb *curpcb;		/* our current running pcb */
#endif
