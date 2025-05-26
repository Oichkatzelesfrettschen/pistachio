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
 * 05-Sep-92  Ian Dall (ian) at University of Adelaide
 *	See if sdp works better (less spurious interrupts) if we use
 *	level triggered interrupts.
 *
 * 09-Aug-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Added some volatile attributes.
 *
 * 11-May-92  Tatu Ylonen (ylo) at Helsinki University of Technology
 *	Created.
 * $Log: pic.c,v $
 * Revision 1.19  1992/05/08  23:32:07  jvh
 * 	Edge triggering for sdp always.
 * 	Interrupt units for sdp and aic are now set by autoconfig.
 * 	Better autoconfig is still needed.
 */
/* 
 * 	File: ns532/pic.c
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 *
 * 	pc532 interrupt controller handling.
 */

/* includes for all devices */
#include "com.h"
#include "sdp.h"
#include "aic.h"
 
#include <ns532/ipl.h>
#include <ns532/pic.h>
#include <ns532/mach_param.h>
#include <mach/ns532/vm_param.h>
#include <ns532/machparam.h>

volatile char *pic_addr = 0;
volatile unsigned curr_ipl;

unsigned int	pic_mask[SPLHI] = {
	0,			/* spl0 */
	0,			/* spl1 */
	0,			/* spl2 */
	0,			/* spl3 */
	0,			/* spl4 */
	0,			/* spl5 */
	0,			/* spl6 */
	0,			/* spl7 */
				/* SPLHI */
};

/* Patched in autoconf.c */
int iunit[NINTR] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

int	nintr = NINTR;

/* These interrupts are always present */
extern intnull(), intnullnull(), dropint(), hardclock();

#if	NCOM > 0
extern	comintr();
#else	NCOM > 0
#define comintr	intnull
#endif	NCOM > 0

#if	NAIC > 0
extern	aic_intr();
#else
#define aic_intr intnull
#endif

#if	NSDP > 0
extern	sdp_intr();
#else
#define sdp_intr intnull
#endif

#define UNUSED_INTR_MASK 0xc00e
int (*ivect[NINTR])() =  {
  	
	hardclock, 	/*00*//* swap output; timer interrupt is mapped here */
	intnull,	/*01*/	/* ir1 */
	intnull,	/*02*/	/* ir2 */
	intnull,	/*03*/ 	/* ir3 */
	sdp_intr,	/*04*/	/* scsii1 (sdp) */
	aic_intr,	/*05*/	/* scsii0 (aic) */
	comintr,	/*06*/	/* duar3 */
	comintr,	/*07*/	/* dua3 */
	comintr,	/*08*/	/* duar2 */
	comintr,	/*09*/	/* dua2 */
	comintr,	/*10*/	/* duar1 */
	comintr,	/*11*/	/* dua1 */
	comintr,	/*12*/	/* duar0 */
	comintr,	/*13*/	/* dua0 */
	intnull,	/*14*/	/* select output */
	intnullnull,	/*15*/ 	/* int15 */	
};
     
/*
 * SPL0 is not allowed here unless you add a test in locore.s
 * (ifdeffed out currently)
 */
unsigned char intpri[NINTR] = {
	SPLHI,	/*00*/	/* timer */
	SPLHI,	/*01*/	/* ir1 */
	SPLHI,	/*02*/	/* ir2 */
	SPLHI,	/*03*/	/* ir3 */
	SPLSCSI,/*04*/	/* scsii1 */
	SPLSCSI,/*05*/	/* scsii0 */
	SPLTTY,	/*06*/	/* duar3 */
	SPLTTY,	/*07*/	/* dua3 */
	SPLTTY,	/*08*/	/* duar2 */
	SPLTTY,	/*09*/	/* dua2 */
	SPLTTY,	/*10*/	/* duar1 */
	SPLTTY,	/*11*/	/* dua1 */
	SPLTTY,	/*12*/	/* duar0 */
	SPLTTY,	/*13*/	/* dua0 */
	SPLHI,	/*14*/	/* swap output */
	SPLHI,	/*15*/ /* int15 */
};
     
unsigned int delaycount;		/* loop count for 1 millisecond */
unsigned int microdata = 50;		/* loop count for 10 microseconds */
     
/*
 * picinit() - This routine 
 *		* Establishes a table of interrupt vectors
 *		* Establishes a table of interrupt priority levels
 *		* Establishes a table of interrupt masks to be put
 *			in the PICs.
 *		* Initialises icu
 *
 *	At this stage the interrupt functionality of this system should be 
 *	coplete.
 *
 */
     
picinit()
{
	int 	i, mctl_base, cnt, loopcount;
	char 	*io_map();
  
	pic_addr = io_map(ICU_ADDR & ~(NS532_PGBYTES - 1), 0x20) +
	    (ICU_ADDR & (NS532_PGBYTES - 1));
  
	/* This is entered with interrupts disabled */
	
	for (i = SPL0; i < SPLHI; i++)
	    pic_mask[i] = form_pic_mask(i) | UNUSED_INTR_MASK;
	
	/*
	 * The L counter is used to generate dma refreshes.
	 * We must be careful not to disturb it.
	 */
	
	pic_addr[ICU_CCTL] = 0x14; 		/* only L counter running */
	pic_addr[ICU_HCCV] = ICU_TIMER_COUNT; 	/* reset current count for H */
	pic_addr[ICU_HCCV+1] = ICU_TIMER_COUNT>>8;
	pic_addr[ICU_HCSV] = ICU_TIMER_COUNT;	/* set max count for H */
	pic_addr[ICU_HCSV+1] = ICU_TIMER_COUNT>>8;
	pic_addr[ICU_CIPTR] = 0<<4;	/* timer interrupt vector */
	pic_addr[ICU_CICTL] = 0x30;	/* enable H counter interrupt */
	pic_addr[ICU_PDIR] = 0x7e;	/* G0 and G7 are outputs */
	pic_addr[ICU_IPS] = 0x7e;	/* G0 and G7 are used for i/o */
	pic_addr[ICU_OCASN] = 0x00;	/* clock H not assigned to G0-3 */
	pic_addr[ICU_PDAT] = 0xfe;	/* keem rom at high mem */
	pic_addr[ICU_ISRV] = 0x00;	/* clear in-service register */
	pic_addr[ICU_ISRV+1] = 0x00;	pic_addr[ICU_CSRC] = 0x00; /* clear cascaded interrupt register */
	pic_addr[ICU_CSRC+1] = 0x00;
	pic_addr[ICU_SVCT] = 0x00;	/* dummy vector (non-vectored mode) */
	pic_addr[ICU_FPRT] = 0x00;	/* set first priority register */
	pic_addr[ICU_TPL] = 0x10;	/* set triggering levels (low=0) */
	pic_addr[ICU_TPL+1] = 0x00;
	pic_addr[ICU_ELTG] = 0xd0;	/* edge0/level1 triggering */
	pic_addr[ICU_ELTG+1] = 0x3f;
	pic_addr[ICU_IMSK] = 0xff;	/* all interrupts initially masked */
	pic_addr[ICU_IMSK+1] = 0xff;

	mctl_base = 0x02;
	pic_addr[ICU_MCTL] = mctl_base;	/* fixed priority */
	pic_addr[ICU_CCTL] = 0x1c;	/* both counters running */
	/* 
	 * we run the H counter here for some time to measure the
	 * speed of the CPU. Later, the counter is halted before
	 * enabling interrupts, and then finally enabled in
	 * pic_start_rtclock (called from startrtclock).
	 */
	
	/* find our speed */
	loopcount = 10000;
	for (i = 0; i < loopcount; i++)
	    ;
	pic_addr[ICU_MCTL] = mctl_base | 0x80;	/* freeze current count */
	cnt = (unsigned char)pic_addr[ICU_HCCV] +
	    256 * (unsigned char)pic_addr[ICU_HCCV+1];
	pic_addr[ICU_MCTL] = mctl_base;		/* unfreeze counts */
	
	/* 
	 * Formula for delaycount is :
	 *  (loopcount * timer clock speed) / (counter ticks * 1000)
	 * 1000 is for figuring out milliseconds 
	 */
	if (cnt >= ICU_TIMER_COUNT - 50)
	{
		panic("pic.c: picinit: findspeed");
	}
	delaycount = (loopcount * (ICU_CLOCK/1000)) / (ICU_TIMER_COUNT-cnt);
	microdata = delaycount / 100 + 1;
	pic_addr[ICU_CCTL] = 0x14;     /* disable H counter for now. */
	
	curr_ipl = SPLHI;
	
	(void)splhi();
}

/* This starts the real time clock timer. */

pic_start_rtclock()
{
	pic_addr[ICU_HCCV] = ICU_TIMER_COUNT;   /* reset current count for H */
	pic_addr[ICU_HCCV+1] = ICU_TIMER_COUNT>>8;
	pic_addr[ICU_HCSV] = ICU_TIMER_COUNT;	/* set max count for H */
	pic_addr[ICU_HCSV+1] = ICU_TIMER_COUNT>>8;
	
	pic_addr[ICU_CCTL] = 0x1c;		/* both counters running */
}

/* Shut down real time clock timer */
pic_stop_rtclock()
{
	pic_addr[ICU_CCTL] = 0x14; /* disable H counter */
}

/*
 * form_pic_mask(int_lvl) 
 *
 *	For a given interrupt priority level (int_lvl), this routine goes out 
 * and scans through the interrupt level table, and forms a mask based on the
 * entries it finds there that have the same or lower interrupt priority level
 * as (int_lvl). It returns a 16-bit mask.
 *
 */

form_pic_mask(int_lvl)
	unsigned short int_lvl;
{
	unsigned short i, bit, mask;

	mask = 0;
	
	for (i = 0x00, bit = 0x01; i < NINTR; i++)
	{
		if (intpri[i] <= int_lvl)
		    mask |= bit;
		bit  = (bit << 0x01); 
	}
	return(mask);
}

intnull(unit)
	int unit;
{
	printf("intnull(%d)\n", unit);
#ifdef	MACH_KDB
	Debugger();
#endif	MACH_KDB
}

intnullnull(irq)
	int irq;
{
}

/* This stuff is from pit.c */

spinwait(millis)
	int millis;		/* number of milliseconds to delay */
{
	int i, j;
	
	for (i = 0; i<millis; i++)
	    for (j = 0; j<delaycount; j++)
		;
}
