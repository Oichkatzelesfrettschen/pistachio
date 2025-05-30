/*
 * Mach Operating System
 * Copyright (c) 1993-1991 Carnegie Mellon University
 * Copyright (c) 1991 OMRON Corporation
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON AND OMRON ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND OMRON DISCLAIM ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	clock.c,v $
 * Revision 2.8  93/11/17  17:34:59  dbg
 * 	Converted for new clocks.
 * 	[93/06/08            dbg]
 * 
 * Revision 2.7  93/01/26  18:02:31  danner
 * 	ANSIfied. Cleaned up includes.
 * 	Inlined and removed clock.h
 * 	[93/01/25            jfriedl]
 * 
 * Revision 2.6  92/08/03  17:40:57  jfriedl
 * 	Fixed the clock/calendar code [inui@omron.co.jp]
 * 	Fixed CPU speed code [jfriedl]
 * 
 * Revision 2.4.2.1  92/05/27  14:35:25  danner
 * 	Header file change support.
 * 	[92/05/27            danner]
 * 
 * Revision 2.5  92/05/21  17:17:48  jfriedl
 *      Cleaned up and de-linted.
 * 	Fixed call to gmtime() in resettodr() to pass the _address_.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.4  92/02/18  18:00:31  elf
 * 	Since sprintf disappeared, just special case it.
 * 	[92/01/19            danner]
 * 
 * Revision 2.3  91/08/24  12:01:52  af
 * 	Changed snprintf to sprintf, upped buffer size.
 * 	[91/07/20  15:34:04  danner]
 * 
 * Revision 2.2.3.1  91/08/19  13:45:57  danner
 * 	Changed snprintf to sprintf, upped buffer size.
 * 	[91/07/20  15:34:04  danner]
 * 
 * Revision 2.2  91/07/09  23:16:42  danner
 * 	cputypes.h -> platforms.h
 * 	[91/06/24            danner]
 * 
 * 	Butchered for 3.0
 * 	[91/05/06            danner]
 * 
 * Revision 2.2  91/04/05  14:01:16  mbj
 * 	Initial code from the Omron 1.10 kernel release corresponding to X130.
 * 	The Copyright has been adjusted to correspond to the understanding
 * 	between CMU and the Omron Corporation.
 * 	[91/04/04            rvb]
 * 
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)clock.c	7.1 (Berkeley) 6/5/86
 */

#include <luna88k/machdep.hpp>	/* standard goodies */
#include <luna88k/nvram.hpp>	/* NVRAM_ADDR, etc. */
#include <m88k/cpu_number.h>    /* master_cpu       */
#include <m88k/pmap.h>		/* pmap_extract()   */
#include <m88k/mach_param.h>	/* HZ (to set clock rate) */
#include <kern/kern_io.h>
#include <kern/strings.h>
#include <kern/time_out.h>
#include <device/clock_dev.h>

/*
 * Machine-dependent clock routines.
 *
 * Startrtclock restarts the real-time clock, which provides
 * hardclock interrupts to kern_clock.c.
 *
 * Inittodr initializes the time of day hardware which provides
 * date functions.  Its primary function is to use some file
 * system information in case the hardare clock lost state.
 *
 * Resettodr restores the time of day hardware after a time change.
 */
static int battery_clock;
static int battery_chkfg;	/* battery checked flag */

/*
 * The routine that sets cycles_per_microsecond is called pretty late in
 * the boot.  However, cycles_per_microsecond is used by
 * the delay_in_microseconds() routine for timing, and is used before
 * determine_cpu_speed() is called.
 *
 * Although it's not a great fix, it's easy.  We'll assume, for the limited
 * time that it's needed before really set, that we're on a really fast
 * processor (33Mhz).  That way, we'll wait too long on slower ones,
 * which is likely to be less painful than not waiting long enough
 * (waiting for whatever...)
 */
#define INITIAL_MHZ_GUESS 33.0
double cycles_per_microsecond = INITIAL_MHZ_GUESS;

static void timeout_routine(volatile int *timeoutflag)
{
    *timeoutflag = 1;
}

/*
 * Called from "luna88k/machdep.c"
 */
void determine_cpu_speed(void)
{
    static char *num[] = {"zero(?!)", "ONE", "TWO", "THREE", "FOUR"};
    static volatile int timeoutflag;
    unsigned cycles;
    double clock_mhz;
    char *speed, buffer[6];

    timeoutflag = 0;
    timeout(timeout_routine, (void *) &timeoutflag, hz);/* wait 1 sec*/
    cycles = measure_pause(&timeoutflag);

    /*
     * Now have approximate number of cycles for the last second.
     * However, this doesn't count the cycles used servicing the 99 clock
     * interrupts, and the cycles spent between setting the timeout
     * and starting counting.
     *
     * By observation, I have noted that the time lost while starting
     * the timeout is insignificant when the length of the timeout
     * is over about 0.5 seconds.
     *
     * The number of cycles reported is about 99.17% of the true clock
     * speed.  Close'nuff. If we want to make up for this, we'll
     * multiply by 1.008377.
     */
    cycles = cycles * 1.008377;	/* make up difference */
    clock_mhz = cycles_per_microsecond = cycles / 1000000.0;
    if (clock_mhz >= 24.3 && clock_mhz <= 25.2)
	    speed = "25";
    else if (clock_mhz >= 19.7 && clock_mhz <= 20.1)
	    speed = "20";
    else if (clock_mhz >= 15.7 && clock_mhz <= 16.1)
	    speed = "16";
    else if (clock_mhz >= 32.7 && clock_mhz <= 33.1)
	    speed = "33";
    else if (clock_mhz >= 39.7 && clock_mhz <= 40.1)
	    speed = "40";
    else
    {
	unsigned a, b, c;
	static char digits[] = "0123456789";

	clock_mhz += 0.05; /* round to nearest single decimal place */
	a = ((unsigned)clock_mhz) / 10;
	b = ((unsigned)clock_mhz) % 10;
	c = (unsigned)((clock_mhz - (unsigned)clock_mhz) * 10.0);
	if (c > 9) /* over 99.9 MHZ! */
	    speed = ">99.9!";
	else
	{
	    speed = buffer;
	    buffer[0] = digits[a];
	    buffer[1] = digits[b];
	    buffer[2] = '.';
	    buffer[3] = digits[c];
	    buffer[4] = '\0';
	}
    }
    #define plural(num)     ((num)==1 ? "" : "s")
    printf("%s MC88100 CPU%s RUNNING AT %sMHz.\n",
	    num[initialized_cpus], plural(initialized_cpus), speed);
}

static void batterychk(void)
{
    static char btchkdata[] = "chk";
    struct nvram *nr = (struct nvram *)NVRAM_ADDR;
    char buf[sizeof(btchkdata)];

    /* if already checked, return */
    if (battery_chkfg)
	    return;

    battery_chkfg = 1;
    if (badaddr((vm_offset_t)nr, 2))
	    return;

    strcpy(buf, btchkdata);
    nvram_write(buf, (vm_offset_t)nr->nv_testwrite, sizeof(btchkdata));
    nvram_read(buf, (vm_offset_t)nr->nv_testwrite, sizeof(btchkdata));
    if (strncmp(buf, btchkdata, sizeof(btchkdata))) {
	    printf("WARNING: calendar clock battery down\n");
	    return;
    }
    /* shouldn't battery_clock be set based upon the above strncmp? */
    battery_clock = 1;
}

#define RTC_FRQ         0x40            /* Frequency test (in day) */
#define RTC_KICK        0x80            /* Kick start (in hour) */
#define RTC_STOP        0x80            /* Stop bit (in seconds) */
#define RTC_WRT         0x80            /* Write bit (in control) */
#define RTC_RD          0x40            /* Read bit (in control) */
#define RTC_SGN         0x20            /* Sign bit (in control) */
#define RTC_DELAY       180             /* delay time */

/*
 * Initialize the time keeper device.
 */
static void tmkeepinit(void)
{
    static char rtcstrings[] = "RTC";
    struct nvram *nr;
    char buf[sizeof(rtcstrings)];

    batterychk();
    if (!battery_clock)
	    return;

    nr = (struct nvram *)NVRAM_ADDR;
    nvram_read(buf, (vm_offset_t)nr->nv_calclock, sizeof(rtcstrings));
    if (!strncmp(buf, rtcstrings, sizeof(rtcstrings))) {
	    if (!((*(volatile unsigned char*)OBIO_CAL_SEC) & RTC_STOP))
		    return;
    }

    printf("Initialize Battery Backup Clock\n");
    *(volatile char *)OBIO_CAL_CTL |= RTC_WRT;
    *(volatile char *)OBIO_CAL_SEC &= ~RTC_STOP;
    *(volatile char *)OBIO_CAL_HOUR |= RTC_KICK;
    *(volatile char *)OBIO_CAL_DOW &= ~RTC_FRQ;
    *(volatile char *)OBIO_CAL_CTL &= ~RTC_WRT;
    delay_in_microseconds(RTC_DELAY);
    *(volatile char *)OBIO_CAL_CTL |= RTC_WRT;
    *(volatile char *)OBIO_CAL_HOUR &= ~RTC_KICK;
    *(volatile char *)OBIO_CAL_CTL &= ~RTC_WRT;
    strcpy(buf,rtcstrings);
    nvram_write(buf, (vm_offset_t)nr->nv_calclock, sizeof(rtcstrings));
}

__inline__ static unsigned int binary_to_bcd(unsigned int i)
{
    return (i / 10) << 4 | (i % 10);
}


__inline__ static unsigned int bcd_to_binary(unsigned int i)
{
    return (i >> 4) * 10 + (i & 0x0f);
}


/*
 *	Read the time from the real-time clock chip.
 */
time_spec_t luna_clock_read(void)
{
    unsigned int year, month, day, hour, mins, secs;

    time_spec_t time;

    /*
     * read the calendar clock
     */
    spl_t s = splimp();
    
    *(volatile char *)OBIO_CAL_CTL |= RTC_RD;
    year =  bcd_to_binary(*(volatile unsigned char *)OBIO_CAL_YEAR);
    month = bcd_to_binary(*(volatile unsigned char *)OBIO_CAL_MON);
    day =   bcd_to_binary(*(volatile unsigned char *)OBIO_CAL_DAY);
    hour =  bcd_to_binary(*(volatile unsigned char *)OBIO_CAL_HOUR);
    mins =  bcd_to_binary(*(volatile unsigned char *)OBIO_CAL_MIN);
    secs =  bcd_to_binary(*(volatile unsigned char *)OBIO_CAL_SEC);
    *(volatile char *)OBIO_CAL_CTL &= ~RTC_RD;
    splx(s);

    year += 1900;

    time.seconds = ymd_to_seconds(
	year, month, day, hour, mins, secs);
    time.nanoseconds = 0;

    return time;
}

/*
 * Reset the TODR based on the time value; used when the TODR
 * has a preposterous value and also when the time is reset
 * by the stime system call.  Also called when the TODR goes past
 * TODRZERO + 100*(SECYEAR+2*SECDAY) (e.g. on Jan 2 just after midnight)
 * to wrap the TODR around.
 *
 * Called from "kern/mach_clock.c".
 */
void	luna_clock_write(
	mach_clock_t	clock,
	time_spec_t	new_time)
{
    unsigned int year, month, day, hour, mins, secs, dow;
    spl_t	s;

    if (!battery_clock)
	    return;

    seconds_to_ymd(new_time.seconds,
	&year, &month, &day, &hour, &mins, &secs, &dow);

    year %= 100;	/* lose century */

    /* I do not know whether I can change year, month,...
     * without setting somethine to the control register.
     */

    s = splimp();
    *(volatile unsigned char *)OBIO_CAL_CTL |= RTC_WRT;
    *(volatile unsigned char *)OBIO_CAL_YEAR = binary_to_bcd(year);
    *(volatile unsigned char *)OBIO_CAL_MON  = binary_to_bcd(month);
    *(volatile unsigned char *)OBIO_CAL_DAY  = binary_to_bcd(day);
    *(volatile unsigned char *)OBIO_CAL_DOW  = binary_to_bcd(dow);
    *(volatile unsigned char *)OBIO_CAL_HOUR = binary_to_bcd(hour);
    *(volatile unsigned char *)OBIO_CAL_MIN  = binary_to_bcd(mins);
    *(volatile unsigned char *)OBIO_CAL_SEC  = binary_to_bcd(secs);
    *(volatile unsigned char *)OBIO_CAL_CTL &= ~RTC_WRT;
    splx(s);
}

/*
 *	Clock 'device' code.
 */

mach_clock_data_t	luna_clock;

void	luna_clock_setresolution(mach_clock_t);
void	luna_clock_enable_interrupts(mach_clock_t);

struct clock_ops luna_clock_ops = {
	luna_clock_setresolution,
	luna_clock_write,
	luna_clock_enable_interrupts
};

void luna_clock_init(void)
{
	clock_init(&luna_clock, &luna_clock_ops);
	luna_clock.resolution = NANOSEC_PER_SEC / HZ;	/* 100 hz */

	sys_clock = &luna_clock;

	tmkeepinit();		/* start the hardware */
	luna_clock.time = luna_clock_read();
}

void luna_clock_setresolution(
	mach_clock_t	clock)
{
	/* Cannot change resolution */
}

void luna_clock_enable_interrupts(
	mach_clock_t	clock)
{
	start_clock();
}

io_return_t clk_open(
	int	dev)
{
	if (dev != 0)
	    return D_NO_SUCH_DEVICE;

	return clock_open(&luna_clock);
}

io_return_t clk_close(
	int	dev)
{
	return D_SUCCESS;
}

io_return_t clk_getstat(
	int		dev,
	int	       	flavor,
	dev_status_t	stat,
	natural_t	*count)
{
	return clock_getstat(&luna_clock, flavor, stat, count);
}

io_return_t clk_setstat(
	int		dev,
	int		flavor,
	dev_status_t	stat,
	natural_t	count)
{
	return clock_setstat(&luna_clock, flavor, stat, count);
}

vm_offset_t clk_mmap(
	int		dev,
	vm_offset_t	off,
	vm_prot_t	prot)
{
	return clock_map_page(&luna_clock, off, prot);
}

io_return_t clk_info(
	int		dev,
	int		flavor,
	mach_clock_t	*info)
{
	if (flavor == D_INFO_CLOCK) {
	    *info = &luna_clock;
	    return D_SUCCESS;
	}
	else {
	    return D_INVALID_OPERATION;
	}
}

