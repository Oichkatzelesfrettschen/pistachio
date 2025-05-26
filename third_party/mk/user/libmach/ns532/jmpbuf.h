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
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log:$
 */
/*
 * 	File: libmach/ns532/jmpbuf.h
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 * 	4.3BSD jmpbuf for the pc532.
 */

#ifndef _JMPBUF_H_
#define _JMPBUF_H_

#define JMP_BUF_SLOTS 15
  /* JMP_BUF_SIZE = JMP_BUF_SLOTS * sizeof(reg) */
#define JMP_BUF_SIZE 60

#define JMP_BUF_SIGSTACK 0
#define JMP_BUF_SIGMASK 4
#define JMP_BUF_R0 8
#define JMP_BUF_R1 12
#define JMP_BUF_R2 16
#define JMP_BUF_R3 20
#define JMP_BUF_R4 24
#define JMP_BUF_R5 28
#define JMP_BUF_R6 32
#define JMP_BUF_R7 36
#define JMP_BUF_SB 40
#define JMP_BUF_FP 44
#define JMP_BUF_SP 48
#define JMP_BUF_PSR 52
#define JMP_BUF_PC 56

#endif _JMPBUF_H_
