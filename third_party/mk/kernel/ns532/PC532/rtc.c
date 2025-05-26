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
 * $Log: rtc.c,v $
 */
/*
 * 	File: ns532/PC532/rtc.c
 * 
 *	Real time clock handling. Created from i386at/rtc.c
 */

/*
  Copyright 1988, 1989 by Intel Corporation, Santa Clara, California.

		All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appears in all
copies and that both the copyright notice and this permission notice
appear in supporting documentation, and that the name of Intel
not be used in advertising or publicity pertaining to distribution
of the software without specific, written prior permission.

INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
IN NO EVENT SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <sys/types.h>
#include <ns532/pmap.h>
#include <ns532/machparam.h>

#ifdef	MACH_KERNEL
#include <kern/time_out.h>
#include <sys/time.h>
#else	MACH_KERNEL
#include <sys/param.h>
#include <sys/kernel.h>
#endif	MACH_KERNEL

#define ROM_ORIGIN 0x10000000 /* address of ROM in pc532 */

#ifdef __STDC__
static volatile unsigned char *rom_p = NULL; /* set by rtcinit */
#else
static unsigned char *rom_p = NULL; /* set by rtcinit */
#endif /* __STDC__ */

/* map rtc into memory */

rtcinit()
{
	static int rtc_inited = 0;
	char *io_map();
	
	if (rtc_inited)
	    return;
	rtc_inited = 1;
	rom_p = (unsigned char *) io_map(ROM_ORIGIN, 10);
}

/* This talks with the rtc chip */

int rtc_rw(write, buf)
	int write;
	unsigned char *buf;
{
	/* Read or write to the real time chip. Address line A0 functions as
	 * data input, A2 is used as the /write signal. Accesses to the RTC
	 * are always done to one of the addresses:
	 *
	 * 0x10000000  -  write a '0' bit
	 * 0x10000001  -  write a '1' bit
	 * 0x10000004  -  read a bit
	 *
	 * Data is output from the RTC using D0. To read or write time
	 * information, the chip has to be activated first, to distinguish
	 * clock accesses from normal ROM reads. This is done by writing,
	 * bit by bit, a magic pattern to the chip. Before that, a dummy read
	 * assures that the chip's pattern comparison register pointer is
	 * reset. The RTC register file is always read or written wholly,
	 * even if we are only interested in a part of it.
	 */
  
	static unsigned char magic[8] =
	{0xc5, 0x3a, 0xa3, 0x5c, 0xc5, 0x3a, 0xa3, 0x5c};

	unsigned char *bp;
	unsigned char dummy;         /* To defeat optimization */
	int s;
	int i;

	s = splhi();
	
	rtcinit();
	
	/* Activate the real time chip */
	dummy = rom_p[4];            /* Synchronize the comparison reg. */
	
	for (bp = magic; bp < magic + 8; bp++) {
		for (i = 0; i < 8; i++)
		    dummy = rom_p[ (*bp >> i) & 0x01 ];
	}
 
	if (!write) {
		/* Read the time from the RTC. */
		
		for (bp = buf; bp < buf + 8; bp++) {
			for (i = 0; i < 8; i++) {
				*bp >>= 1;
				*bp |= ((rom_p[4] & 0x01) ? 0x80 : 0x00);
			}
		}
	} else {
		/* Write to the RTC */
		for (bp = buf; bp < buf + 8; bp++) {
			for (i = 0; i < 8; i++)
			    dummy = rom_p[ (*bp >> i) & 0x01 ];
		}
	}
	splx(s);
}

extern struct timeval time;
extern struct timezone tz;

static int month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

yeartoday(year)
	int year;
{
	return((year % 4) ? 365 : 366); /* The chip won't last till 2100 */
}

hexdectodec(n)
	char n;
{
	return(((n >> 4) & 0x0F) * 10 + (n & 0x0F));
}

char
dectohexdec(n)
	int n;
{
	return((char)(((n / 10) << 4) & 0xF0) | ((n % 10) & 0x0F));
}

struct rtc_st {
	char	rtc_asec; /* 100th of second */
	char	rtc_sec;  /* seconds */
	char	rtc_min;  /* minutes */
	char	rtc_hr;   /* hours */
	char	rtc_dow;  /* day of week */
	char	rtc_dom;  /* day of month */
	char	rtc_mon;  /* month */
	char	rtc_yr;   /* year */
};

readtodc(tp)
	time_t	*tp;
{
	struct rtc_st rtclk;
	time_t n;
	int sec, min, hr, dom, mon, yr;
	int i, ospl, days = 0;
	
	ospl = splhi();
	rtc_rw(0,(unsigned char *)&rtclk);
	splx (ospl);

	sec = hexdectodec(rtclk.rtc_sec);
	min = hexdectodec(rtclk.rtc_min);
	hr = hexdectodec(rtclk.rtc_hr);
	dom = hexdectodec(rtclk.rtc_dom);
	mon = hexdectodec(rtclk.rtc_mon);
	yr = hexdectodec(rtclk.rtc_yr);
	yr = (yr < 70) ? yr+100 : yr;

	n = sec + 60 * min + 3600 * hr;
	n += (dom - 1) * 3600 * 24;

	if (yeartoday(yr) == 366)
		month[1] = 29;
	for (i = mon - 2; i >= 0; i--)
		days += month[i];
	month[1] = 28;
	for (i = 70; i < yr; i++)
		days += yeartoday(i);
	n += days * 3600 * 24;

	*tp = n;

	return 0;
}

writetodc()
{
	struct rtc_st rtclk;
	time_t n;
	int ospl, diff, i, j;

	ospl = splhi();
	rtc_rw(0,&rtclk);
	splx(ospl);

	diff = 0;
	n = (time.tv_sec - diff) % (3600 * 24);   /* hrs+mins+secs */
	rtclk.rtc_sec = dectohexdec(n % 60);
	n /= 60;
	rtclk.rtc_min = dectohexdec(n % 60);
	rtclk.rtc_hr = dectohexdec(n / 60);

	n = (time.tv_sec - diff) / (3600 * 24);	/* days */
	rtclk.rtc_dow = (n + 4) % 7;  /* 1/1/70 is Thursday */

	for (j = 1970, i = yeartoday(j); n >= i; j++, i = yeartoday(j)) {
		n -= i;
	}
	rtclk.rtc_yr = dectohexdec(j - 1900);

	if (i == 366)
		month[1] = 29;
	for (i = 0; n >= month[i]; i++)
		n -= month[i];
	month[1] = 28;
	rtclk.rtc_mon = dectohexdec(++i);

	rtclk.rtc_dom = dectohexdec(++n);

	ospl = splhi();
	rtc_rw(1, &rtclk);
	splx(ospl);

	return 0;
}
