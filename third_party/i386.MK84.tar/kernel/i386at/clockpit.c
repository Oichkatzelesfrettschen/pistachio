/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * $Log:	clockpit.c,v $
 * Revision 2.3  93/12/23  10:05:01  dbg
 * 	Added missing include files.
 * 	[93/12/14            dbg]
 * 
 * Revision 2.2  93/11/17  16:41:54  dbg
 * 	Used common routines from device/clock_dev.c
 * 	[93/06/02            dbg]
 * 
 * 	Changed mmap routines to return physical address instead of
 * 	physical page number.
 * 	[93/05/24            dbg]
 * 
 * 	Added to microkernel mainline.
 * 
 * 	Created
 * 	[92/06/22	savage]
 * 
 */
#include <clockpit.h>

#include <mach/boolean.h>
#include <mach/time_spec.h>
#include <kern/clock.h>
#include <kern/kern_io.h>
#include <vm/vm_kern.h>

#include <sys/types.h>
#include <chips/busses.h>
#include <device/device_types.h>
#include <device/clock_status.h>
#include <device/clock_dev.h>

#include <i386/machspl.h>
#include <i386/pio.h>
#include <i386/pit.h>

/*
 *	Adjust for PIT clock round off... Ugly, but true...
 */
#define	HZ	100			/* default frequency */

#define CLOCKPIT_SKEW(res)		(((CLKNUM%(NANOSEC_PER_SEC/res)) \
					       * res) / CLKNUM)
#define CLOCKPIT_DEFAULT_RESOLUTION	(NANOSEC_PER_SEC/HZ - \
					 CLOCKPIT_SKEW((NANOSEC_PER_SEC/HZ)))

#define CLOCKPIT_MAX_RESOLUTION 	(10000000)

/*
 *	Minimum set to 1/2 a millisecond... if you want to play with lower
 *	then change this number.
 */
#define CLOCKPIT_MIN_RESOLUTION		(500000)


mach_clock_data_t		clockpit0;

mach_clock_t			clockpit[] = {&clockpit0};

void	clockpit_setresolution(mach_clock_t);	/* forward */
void	clockpit_write(mach_clock_t, time_spec_t);
void	clockpit_enable_interrupts(mach_clock_t);

struct clock_ops clockpit_ops = {
	clockpit_setresolution,
	clockpit_write,
	clockpit_enable_interrupts
};

void	clockpitsysintr(int);

boolean_t clockpitprobe(
	vm_offset_t		port,
	struct bus_device 	*dev);
void clockpitattach(
	struct bus_device	*dev);

static vm_offset_t		clockpit_std[NCLOCKPIT] = { 0 };
static struct bus_device	*clockpit_info[NCLOCKPIT];
struct bus_driver		clockpitdriver = {
	clockpitprobe, 0, clockpitattach, 0, clockpit_std, "clockpit",
	clockpit_info,0, 0, 0};

boolean_t clockpitprobe(
	vm_offset_t		port,
	struct bus_device 	*dev)
{
	int		unit = dev->unit;

	if (unit < 0 || unit >= NCLOCKPIT)
	    return FALSE;

	/*
	 *	Always have one.
	 */
	return TRUE;
}

void clockpitattach(
	struct bus_device	*dev)
{
	int		unit = dev->unit;
	mach_clock_t	clock = clockpit[unit];

	clock_init(clock, &clockpit_ops);
	clock->resolution = CLOCKPIT_DEFAULT_RESOLUTION;

	if (sys_clock == 0) {
	    /*
	     *	If no system clock already, use this as system clock.
	     */
	    sys_clock = &clockpit0;
	    dev->intr = clockpitsysintr;    /* use system clock interrupt */
	    dev_change_indirect("clock_priv", "clockpit", unit);
					    /* set generic clock device */
	    set_clock_unpriv();		    /* and unprivileged clock */
	}

	printf(", time = %u secs : %u nsecs, resolution = %u nsecs",
	       clock->time.seconds,
	       clock->time.nanoseconds,
	       clock->resolution);
}

/*
 *	Enable interrupts from PIT clock
 */
void clockpit_enable_interrupts(
	mach_clock_t clock)
{
	take_dev_irq(clockpit_info[0]);
}

io_return_t clockpitopen(
	int	dev)
{
	if (dev < 0 || dev >= NCLOCKPIT) {
	    return D_NO_SUCH_DEVICE;
	}
	return clock_open(clockpit[dev]);
}

io_return_t clockpitclose(
	int	dev)
{
	return D_SUCCESS;
}

io_return_t clockpitgetstat(
	int		dev,
	int	       	flavor,
	dev_status_t	stat,
	natural_t	*count)
{
	return clock_getstat(clockpit[dev], flavor, stat, count);
}

io_return_t clockpitsetstat(
	int		dev,
	int		flavor,
	dev_status_t	stat,
	natural_t	count)
{
	mach_clock_t	clock = clockpit[dev];
	unsigned int	frequency;
	unsigned int	skew;
	spl_t		s;

	switch (flavor) {
	    case CLOCK_RESOLUTION:
	    {
		clock_resolution_t	request;

		if (count < CLOCK_RESOLUTION_COUNT)
		    return D_INVALID_SIZE;

		request = (clock_resolution_t)stat;
		if (request->resolution < CLOCKPIT_MIN_RESOLUTION ||
		    request->resolution > CLOCKPIT_MAX_RESOLUTION)
		{
		    return D_INVALID_SIZE;
		}
			
		frequency = NANOSEC_PER_SEC/request->resolution;
		skew = CLOCKPIT_SKEW(request->resolution);
			
		if (skew > request->skew) {
		    return D_INVALID_SIZE;
		}

		s = splsched();
		clock_queue_lock(clock);

		clock->new_resolution = request->resolution - skew;
		clock->new_skew = request->skew + skew;	/* ? */

		clock_queue_unlock(clock);
		splx(s);
		return D_SUCCESS;
	    }

	    default:
		return clock_setstat(clock, flavor, stat, count);
	}
}

vm_offset_t clockpitmmap(
	int		dev,
	vm_offset_t	off,
	vm_prot_t	prot)
{
	return clock_map_page(clockpit[dev], off, prot);
}

io_return_t clockpitinfo(
	int		dev,
	int		flavor,
	mach_clock_t	*info)
{
	if (flavor == D_INFO_CLOCK) {
	    *info = clockpit[dev];
	    return D_SUCCESS;
	}
	else {
	    return D_INVALID_OPERATION;
	}
}

void clockpit_setresolution(
	mach_clock_t	clock)
{
	unsigned int	pitval;
	unsigned int	frequency;

	frequency = NANOSEC_PER_SEC/(clock->resolution);

	pitval = CLKNUM/frequency;
	outb(PITCTL_PORT,  PIT_C0|PIT_SQUAREMODE|PIT_READMODE);
	outb(PITCTR0_PORT, pitval);
	outb(PITCTR0_PORT, pitval>>8);
}

void clockpit_write(
	mach_clock_t	clock,
	time_spec_t	new_time)
{
	/* XXX no permanent clock storage XXX */
}

