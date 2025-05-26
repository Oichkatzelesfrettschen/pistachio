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
 * $Log: pic.h,v $
 */
/*
 * 	File: ns532/pic.h
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 * 
 *	32202 ICU definitions + pc532 specific definitions.
 */

#ifndef _NS532_PIC_H_
#define _NS532_PIC_H_

/* ICU port addresses */

#define ICU_ADDR	0xfffffe00

#define ICU_HVCT	0x00 /* hardware vector */
#define ICU_SVCT	0x01 /* software vector */
#define ICU_ELTG	0x02 /* edge/level triggering */
#define ICU_TPL		0x04 /* triggering polarity */
#define ICU_IPND	0x06 /* interrupts pending */
#define ICU_ISRV	0x08 /* interrupts in-service */
#define ICU_IMSK	0x0a /* interrupt mask */
#define ICU_CSRC	0x0c /* cascaded source */
#define ICU_FPRT	0x0e /* first priority */
#define ICU_MCTL	0x10 /* mode control */
#define ICU_OCASN	0x11 /* output clock assignment */
#define ICU_CIPTR	0x12 /* counter interrupt pointer */
#define ICU_PDAT	0x13 /* port data */
#define ICU_IPS		0x14 /* interrupt/port select */
#define ICU_PDIR	0x15 /* port direction */
#define ICU_CCTL	0x16 /* counter control */
#define ICU_CICTL	0x17 /* counter interrupt control */
#define ICU_LCSV	0x18 /* L-counter starting value */
#define ICU_HCSV	0x1a /* H-counter starting value */
#define ICU_LCCV	0x1c /* L-counter current value */
#define ICU_HCCV	0x1e /* H-counter current value */

/* pc532 icu timer related constants */

#define ICU_CLK		3686400		/* clk cycles per second */
#define ICU_CLOCK	(ICU_CLK/4)	/* but it is prescaled */
#define ICU_TIMER_COUNT	(ICU_CLOCK/HZ)	/* maximum value of counter */

/* interrupt request assignments */

#define IR_TIMER	0	/* timer interrupt */
#define IR_IR1		1	/* unused interrupt 1 */
#define IR_IR2		2	/* unused interrupt 2 */
#define IR_IR3		3	/* unused interrupt 3 */
#define IR_SCSII1	4	/* DP scsi interrupt */
#define IR_SCSII0	5      	/* AIC scsi interrupt */
#define IR_DUAR3	6	/* duart 3 receive */
#define IR_DUA3		7	/* duart 3 other interrupts */
#define IR_DUAR2	8	/* duart 2 receive */
#define IR_DUA2		9	/* duart 2 other interrupts */
#define IR_DUAR1	10	/* duart 1 receive */
#define IR_DUA1		11	/* duart 1 other interrupts */
#define IR_DUAR0	12	/* duart 0 receive */
#define IR_DUA0		13	/* duart 0 other interrupts */
#define IR_UNUSED     	14	/* this line is output */
#define IR_INT15	15	/* unused interrupt 15 */

#define NINTR		16	/* number of interrupt sources */

#endif _NS532_PIC_H_
