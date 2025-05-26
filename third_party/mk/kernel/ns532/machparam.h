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
 * 15-Apr-93  Ian Dall (ian) at University of Adelaide
 *	Added typedef for spl_t.
 *
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 * $Log: machparam.h,v $
 */
/*
 * 	File: ns532/machparam.h
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 *	Machine-dependent definitions.
 */
#ifndef _NS532_MACHPARAM_H_
#define _NS532_MACHPARAM_H_

#include <mach/ns532/vm_param.h>
#include <ns532/ipl.h>

#ifdef KERNEL
#include "spldebug.h"
#endif

#define PGBYTES NS532_PGBYTES       /* bytes per 32532 page */
#define PGSHIFT NS532_PGSHIFT       /* number of bits to shift for pages */

#define btop(x) ns532_btop(x)
#define ptob(x) ns532_ptob(x)

#define NBPG        PGBYTES         /* bytes/page */

#define PGOFSET     (NBPG-1)        /* byte offset into page */

#define CLSIZE   1

#define     ctob(x) ((x)<<PGSHIFT)

#ifndef ASSEMBLER

typedef unsigned short	spl_t;

#if SPLDEBUG

#define spl0() 		primitive_spl0()
#define splsoftclock() 	primitive_spl_with_check(SPLSOFTCLOCK)
#define spldroptosoftclock() 	primitive_spln(SPLSOFTCLOCK)
#define splnet()	primitive_spl_with_check(SPLNET)
#define splscsi()	primitive_spl_with_check(SPLSCSI)
#define splbio()	primitive_spl_with_check(SPLBIO)
#define spltty()	primitive_spl_with_check(SPLTTY)
#define splhi()		primitive_spl_with_check(SPLHI)
#define splx(x)		primitive_splx_with_check(x)
#define splx_no_check(x)	primitive_splx(x)

#else SPLDEBUG

#define spl0() 		primitive_spl0()
#define splsoftclock() 	primitive_spln(SPLSOFTCLOCK)
#define spldroptosoftclock() 	primitive_spln(SPLSOFTCLOCK)
#define splnet()	primitive_spln(SPLNET)
#define splscsi()	primitive_spln(SPLSCSI)
#define splbio()	primitive_spln(SPLBIO)
#define spltty()	primitive_spln(SPLTTY)
#define splhi()		primitive_splhi()
#define splx(x)		primitive_splx(x)
#define splx_no_check(x)	primitive_splx(x)

#endif SPLDEBUG

#define splhigh()	splhi()
#define splclock()	splhi()
#define splimp()	splhi()
#define splvm()		splhi()
#define splsched()	splhi()

#endif ASSEMBLER

#endif _NS532_MACHPARAM_H_
