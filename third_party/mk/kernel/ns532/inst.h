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
 * 11-May-92  Tero Kivinen (kivinen) at Helsinki University of Technology
 *	Created.
 *
 * $Log: inst.h,v $
 */
/*
 * 	File: ns532/inst.h
 *	Author: Tero Kivinen, Helsinki University of Technology 1992.
 */

#ifndef _NS532_INST_H_ 
#define _NS532_INST_H_

#define IS_FLOAT_INST(addr) ( \
			     (*((unsigned char *) addr) == 0x3e) || \
			     (*((unsigned char *) addr) == 0xbe) || \
			     (*((unsigned char *) addr) == 0xfe))

#define IS_ILOCK1(addr) (*((unsigned char *) addr) == 0x4e)
#define IS_ILOCK2(addr) ((*(((unsigned char *) addr) + 1) & 0x2c) == 0x0c)
#define IS_ILOCK(addr) (IS_ILOCK1(addr) && IS_ILOCK2(addr))
  
#define IS_CINV(addr) ( \
		       (*((unsigned char *) addr) == 0x1e) && \
		       (*(((unsigned char *) addr) + 1) & 0x7f == 0x27))
#define IS_CINV_DATA(addr) ((*(((unsigned char *) addr) + 1) & 0x80 == 0x80))
#define IS_CINV_INSTR(addr) ((*(((unsigned char *) addr) + 2) & 0x01 == 0x01))

/*
 * after MAXTRAPCOUNT page_faults at same address check if it is
 * an interlocked bit operation. If page_fault count ever reaches 
 * PANICTRAPCOUNT the system will panic if in kernel mode, or kill the 
 * thread if in user mode. (We have found another sbiti and cbiti).
 */

#define MAXTRAPCOUNT 3
#define PANICTRAPCOUNT 256000

#endif _NS532_INST_H_
