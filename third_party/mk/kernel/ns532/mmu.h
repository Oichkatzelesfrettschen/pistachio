/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * Copyright (c) 1992 Helsinki University of Technology
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND HELSINKI UNIVERSITY OF TECHNOLOGY ALLOW FREE USE
 * OF THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * HELSINKI UNIVERSITY OF TECHNOLOGY DISCLAIM ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * 11-May-92  Tatu Ylonen (ylo) at Helsinki University of Technology
 *	Created.
 *
 * $Log: mmu.h,v $
 */
/*
 * 	File: ns532/mmu.h
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 * 
 *	MMU register definitions for ns532
 */

#ifndef _NS532_MMU_H_
#define _NS523_MMU_H_

#define MSR_TEX		0x03	/* translation exception field */
#define MSR_TEX_NONE	0x00	/* no translation exception */
#define MSR_TEX_FIRST	0x01	/* first level pte invalid */
#define MSR_TEX_SECOND	0x02	/* second level pte invalid */
#define MSR_TEX_PROT	0x03	/* protection violation */
#define MSR_DDT		0x04	/* data direction */
#define MSR_DDT_READ	0x00	/* read cycle */
#define MSR_DDT_WRITE	0x04	/* write cycle */
#define MSR_USR		0x08	/* user/supervisor */
#define MSR_USR_KERNEL	0x00	/* supervisor mode */
#define MSR_USR_USER	0x08	/* user mode */
#define MSR_STT		0xf0	/* cpu status */
#define MSR_STT_SIF	0x80	/* SequEntial Instruction Fetch */
#define MSR_STT_NSIF	0x90	/* Non-Sequential Instruction Fetch */
#define MSR_STT_DT	0xa0	/* Data Transfer */
#define MSR_STT_RMW	0xb0	/* Read in Read-Modify-Write Cycle */
#define MSR_STT_REA	0xc0	/* Read for Effective Address */

#define MCR_AO		0x8	/* access level override */
#define MCR_DS		0x4	/* dual space */
#define MCR_TS		0x2	/* translate supervisor */
#define MCR_TU		0x1	/* translate user */

#define MCR_VALUE	(MCR_TS|MCR_TU) /* mcr value that we use */

#endif /* _NS532_MMU_H_ */
