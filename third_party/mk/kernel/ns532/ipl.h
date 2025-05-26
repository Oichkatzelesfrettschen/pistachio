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
 * $Log: ipl.h,v $
 */
/*
 * 	File: ns532/ipl.h
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 */

#ifndef _NS532_IPL_H_
#define _NS532_IPL_H_


#define SPL0            0
#define SPL1            1
#define SPL2            2
#define SPL3            3
#define SPL4            4
#define SPL5            5
#define SPL6            6
#define SPL7            8

#define SPLSOFTCLOCK	1
#define SPLSCSI		2
#define SPLNET		4
#define SPLPP           5
#define SPLBIO		5
#define SPLTTY          6
#define SPLNI           6

#define IPLHI           SPL7
#define SPLHI           IPLHI


#endif _NS532_IPL_H_
