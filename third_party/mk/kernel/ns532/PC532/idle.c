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
 * 08-Aug-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Added cpu_number arg to machine_idle to agree with new MI code.
 *
 * 11-May-92  Tero Kivinen (kivinen) at Helsinki University of Technology
 *	Created.
 *
 * $Log: idle.c,v $
 */
/*
 * 	File: ns532/PC532/idle.c
 *	Author: Tero Kivinen, Helsinki University of Technology 1992.
 *
 *	Show CPU idle on led panel.
 */

#include "hexpanel.h"
#include "idleleds.h"

#if IDLELEDS && KITT_LED
      error you cannot have both idleleds and KITT_LED defined.
#endif

/* Kitt lights */
#if KITT_LED && HEXPANEL
    
static int points[]= {
	LEDPOINT0, LEDPOINT1, LEDPOINT2, LEDPOINT3, LEDPOINT4,
	LEDPOINT5, LEDPOINT6, LEDPOINT7, LEDPOINT8, LEDPOINT9,
	LEDPOINT9, LEDPOINT8, LEDPOINT7, LEDPOINT6, LEDPOINT5,
	LEDPOINT4, LEDPOINT3, LEDPOINT2, LEDPOINT1, LEDPOINT0
};
#define NUMPOINTS (sizeof(points)/sizeof(points[0]))
int kitt_led_pos = 0;

#endif /* KITT_LED */

/*ARGSUSED*/
void machine_idle(cpu_number)
	int cpu_number;
{
  asm("wait");
  
#if KITT_LED && HEXPANEL
  led_points(points[kitt_led_pos]);
  kitt_led_pos = (kitt_led_pos + 1) % NUMPOINTS;
#endif /* KITT_LED && HEXPANEL */
}

#if IDLELEDS

#include <ns532/PC532/leds.h>
#include <mach/machine.h>
#include <kern/cpu_number.h>

int ticks_between_idle_calk = 50;

static int idletable[] = {
	LEDPOINT9,
	LEDPOINT8 | LEDPOINT9,
	LEDPOINT7 | LEDPOINT8 | LEDPOINT9,
	LEDPOINT6 | LEDPOINT7 | LEDPOINT8 | LEDPOINT9,
	LEDPOINT5 | LEDPOINT6 | LEDPOINT7 | LEDPOINT8 | LEDPOINT9,
	LEDPOINT4 | LEDPOINT5 | LEDPOINT6 | LEDPOINT7 | LEDPOINT8 | LEDPOINT9,
	LEDPOINT3 | LEDPOINT4 | LEDPOINT5 | LEDPOINT6 | LEDPOINT7 | LEDPOINT8 |
	    LEDPOINT9,
	LEDPOINT2 | LEDPOINT3 | LEDPOINT4 | LEDPOINT5 | LEDPOINT6 | LEDPOINT7 |
	    LEDPOINT8 | LEDPOINT9,
	LEDPOINT1 | LEDPOINT2 | LEDPOINT3 | LEDPOINT4 | LEDPOINT5 | LEDPOINT6 |
	    LEDPOINT7 | LEDPOINT8 | LEDPOINT9,
	LEDPOINT0 | LEDPOINT1 | LEDPOINT2 | LEDPOINT3 | LEDPOINT4 | LEDPOINT5 |
	    LEDPOINT6 | LEDPOINT7 | LEDPOINT8 | LEDPOINT9
};

extern int ts_tick_count;

/* show_idle is called from hardclock. */
show_idle()
{
	static int lasttick = 0;
	static int previdle = 0, prevtotal = 0;
	static int cpuidle = 0;
	if (ticks_between_idle_calk != 0) {
		if (ts_tick_count - lasttick > ticks_between_idle_calk) {
			register int my_cpu = cpu_number();
			register int total, idle, i;
			
			total = 0;
			for (i = 0; i < CPU_STATE_MAX; i++) {
				total += machine_slot[my_cpu].cpu_ticks[i];
			}
			idle = machine_slot[my_cpu].cpu_ticks[CPU_STATE_IDLE];
			cpuidle = ((idle - previdle) * 100) /
			    (total-prevtotal);
			cpuidle /= 10;
			if (cpuidle > 9) {
				cpuidle = 9;
			}
			previdle = idle;
			prevtotal = total;
			lasttick = ts_tick_count;
			/* Use your favorite idle output device here */
#if HEXPANEL
			led_points(idletable[cpuidle]);
#else
			led_outraw(idletable[cpuidle*8/10]);
#endif /* HEXPANEL */
		}
	}
}
#endif /* IDLELEDS */
