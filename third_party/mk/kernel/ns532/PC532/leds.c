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
 * $Log: leds.c,v $
 */
/*
 * 	File: ns532/PC532/leds.c
 *	Author: Tero Kivinen, Helsinki University of Technology 1992.
 *
 * 	Display hex digits on led display.
 */

#include <aic.h>
#include <ns532/PC532/leds.h>

#if NAIC == 0
     error you must have aic driver to get hexpanel work
#endif

#define SD_AIC_DATAOUT 0xd

int show_leds_hardclock = 1;
int show_leds_trap = 1;
int show_leds_syscall = 1;
int show_leds_int = 1;
int show_leds_function = 1;

void led_outdigit (pos, val)
	int pos;
	int val;
{
	char cmd[6];
	
	cmd[0] = 0xbf;
	cmd[1] = (~pos & 0xf) | 0x90;
	cmd[2] = (~pos & 0xf) | 0xb0;
	cmd[3] = (~val & 0xf) | 0xa0;
	cmd[4] = (~val & 0xf) | 0xb0;
	cmd[5] = 0xff;
	aic_str_to_reg(SD_AIC_DATAOUT, 6, cmd);
}

void leds_hardclock(usermode)
	int usermode;
{
	static int count = 1;
	if (show_leds_hardclock) {
		led_outdigit(LED_USERMODE, usermode);
		led_outdigit(LED_BEAT1, count >> 4);
		led_outdigit(LED_BEAT2, count);
	}
	count++;
}

void leds_phase(num)
	int num;
{
	led_outdigit(LED_PHASE, num);
}

void leds_int(num)
	int num;
{
	if (num != 0 && show_leds_int) {
		led_outdigit(LED_INT, num);
	}
}

void leds_trap(num)
	int num;
{
	if (show_leds_trap) {
		led_outdigit(LED_TRAP, num);
	}
}

void leds_syscall(num)
	int num;
{
	if (show_leds_syscall) {
		led_outdigit(LED_SYSCALL1, num >> 4);
		led_outdigit(LED_SYSCALL2, num);
	}
}

void leds_f(func, misc)
	int func, misc;
{
	if (show_leds_function) {
		led_outdigit(LED_FUNCTION, func);
		led_outdigit(LED_MISC, misc);
	}
}

void led_outnumber(num)
	int num;
{
	register int i;
	for(i = 0; i < 8; i++) {
		led_outdigit(i + 2, (num >> ((7 - i) * 4)) && 0x0f);
	}
}

void led_points(num)
	int num;
{
	led_outdigit(10, (num >> 8) & 0xf);
	led_outdigit(11, (num >> 4) & 0xf);
	led_outdigit(12, num & 0x3);
}

void led_outraw(num)
	int num;
{
	char cmd;
	
	cmd = num;
	aic_str_to_reg(SD_AIC_DATAOUT, 1, &cmd);
}
