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
 * $Log: board.h,v $
 */
/*
 * 	File: ns532/PC532/board.h
 *	Author: Johannes Helander, Jukka Virtanen
 *	Helsinki University of Technology 1992.
 *
 * 	pc532 hardware specific definitions. 
 */

#ifndef __NS532_PC532_BOARD_H_
#define __NS532_PC532_BOARD_H_

#ifndef ASSEMBLER
#include <ns532/pic.h>
#endif ASSEMBLER

#define DUART_BOOT_VIRTUAL_ADDR	0xffffe000
#define DUART_ADDR	0x28000000
#define DUART_STAT	1
#define DUART_DATA	3
#define DUART_TX_EMPTY	0x08

/* ICU SCSI chip selection */
#ifndef ASSEMBLER
#define ICU_SELECT	((char *)(ICU_ADDR + ICU_PDAT))
#endif ASSEMBLER

#define ICU_MASK	0x7f
#define ICU_DP		0
#define ICU_AIC		0x80

/* SCSI port address definitions */
#define PC532_SCSI_BASE		0x30000000 /* base addr of scsi controller */
#define PC532_SCSI_DMABASE 	0x38000000 /* base addr for scsi pseudo dma */
#define PC532_SCSI_DMASIZE 	0x08000000 /* size of are for pseudo dma */

/* the last byte is written here. End of DMA */
#define PC532_SCSI_DMAEOP	(0x38400000)

#if 0
#define PC532_SCSI_DMAWINDOW (64*1024) /* ??? used window in dma port */
#endif

/* Buffer for SCSI commands and command results */
/* Data is not buffered here; instead we pseudo-dma it directly
 * to the ior XXX???
 */
#define SDP_RAM_SIZE     (8*1024)
#define AIC_RAM_SIZE     (8*1024)

#ifndef ASSEMBLER

#if MACH_KDB
#define gimmeabreak() Debugger("gimmeabreak")
#else
#define gimmeabreak() do ; while(0)
#endif

/* for debugging purposes only */
extern volatile int xxx_curr_select;
extern volatile int xxx_prev_select;

/* select ICU_AIC or ICU_DP */
#ifndef __GNUC__ 
/* You have to implement this as a function! */
#define PC532_SCSI_SELECT(X) pc532_scsi_chip_select(X)
#else
#define PC532_SCSI_SELECT(X) \
    ({ int old = *ICU_SELECT;			\
       xxx_prev_select = (old & ICU_MASK);	\
       *ICU_SELECT = (old & ICU_MASK) | (X);	\
	xxx_curr_select = (X);			\
	old & ~(ICU_MASK);			\
    })
#endif
#endif ASSEMBLER

#endif __NS532_PC532_BOARD_H_
