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
 * $Log: leds.h,v $
 */
/*
 * 	File: ns532/PC532/leds.h
 *	Author: Tatu Ylonen, Tero Kivinen
 *	Helsinki University of Technology 1992.
 */

#define LED_PHASE	0 /*  */
#define LED_INT		1 /* number of last interrupt */
#define LED_USERMODE	2 /* last clock tick in user mode */
#define LED_TRAP	3 /* trap number */
#define LED_SYSCALL1	4 /* first digit of syscall number */
#define LED_SYSCALL2	5 /* second digit of syscall number */
#define LED_BEAT1	6 /* first digit of heartbeat */
#define LED_BEAT2	7 /* second digit of heartbeat */
#define LED_FUNCTION	8 /* function code */
#define LED_MISC	9 /* misc data */

/* function values */
#define LED_F_NONE		0
#define LED_F_CSW		3
#define LED_F_SCSI		4
#define LED_F_COM		5
#define LED_F_COPYIN		6
#define LED_F_COPYOUT		7

#define LEDPOINT0	0x002
#define LEDPOINT1	0x001
#define LEDPOINT2	0x080
#define LEDPOINT3	0x040
#define LEDPOINT4	0x020
#define LEDPOINT5	0x010
#define LEDPOINT6	0x800
#define LEDPOINT7	0x400
#define LEDPOINT8	0x200
#define LEDPOINT9	0x100

#define LEDPALL		0xff3
#define LEDPBYTE	0x551
#define LEDPWORDL	0x440
#define LEDPWORDR	0x110
