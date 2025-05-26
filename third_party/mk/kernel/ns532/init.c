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
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: init.c,v $
 */
/*
 * 	File: ns532/init.c
 *	Author: Tatu Ylonen, Johannes Helander
 *	Helsinki University of Technology 1992.
 */

#include <mach_kdb.h>

#include <mach/machine.h>
#include <kern/time_out.h>
#include <kern/zalloc.h>
#include <ns532/thread.h>
#include <ns532/machparam.h>

#include <sys/time.h>
#if	MACH_KDB
#include <sys/reboot.h>
#endif	MACH_KDB


/*
 *	Cpu initialization.  Running virtual, but without MACH VM
 *	set up.  First C routine called.
 */
void machine_startup()
{
	extern char	version[];
	extern void	setup_main();

	/*
	 * Do basic VM initialization
	 */
	ns532_init(); /* this will also set led display if appropriate */

#if	MACH_KDB

	/*
	 * Initialize the kernel debugger.
	 */
	ddb_init();

	/*
	 * Cause a breakpoint trap to the debugger before proceeding
	 * any further if the proper option bit was specified in
	 * the boot flags.
	 *
	 * XXX use -a switch to invoke kdb, since there's no
	 *     boot-program switch to turn on RB_HALT!
	 */
	if (boothowto & RB_ASKNAME) {
	  printf("Test 1 of db_printf\n");
	  db_printf("%30s %20d %20d\n", "db print test1", 1, 2);
	  db_printf("%30s %-*X %-*X\n", "db print test2", 20, 1, 20, 2);
          printf("test done...\nBreaking to debugger...\n");
	    Debugger();
	  }
#endif	MACH_KDB

	printf(version);

	machine_slot[0].is_cpu = TRUE;
	machine_slot[0].running = TRUE;
	machine_slot[0].cpu_type = CPU_TYPE_NS32532;
	machine_slot[0].cpu_subtype = CPU_SUBTYPE_PC532;

	/*
	 * Start the system.
	 */
	setup_main();
}

/*
 * Find devices.  The system is alive.
 */
void machine_init()
{
	extern void inittodr();	/* forward */

	/* get time */
	inittodr();
	
	initialize_devices();

	get_root_device();
}

/*
 * Halt a cpu.
 */
halt_cpu()
{
	printf("Trying to return to monitor...\n");
	pic_stop_rtclock();
#if MACH_KDB
	Debugger();
#endif
	_drop_to_monitor();
	printf("Failed to drop to monitor, stopping in infinite loop.\n");
	for (;;)
	    ;
}

/*
 * Halt the system or reboot.
 */
halt_all_cpus(reboot)
	boolean_t	reboot;
{
	delay(2000000);
	printf("System halted.\n");
	if (reboot)
	    printf("Sorry, not able to reboot yet.\n");
	printf("Press RESET or power off and on to restart.\n");
	halt_cpu();
	/* NOTREACHED */
}

#include <mach/vm_prot.h>
#include <vm/pmap.h>
#include <mach/time_value.h>

timemmap(dev,off,prot)
	vm_prot_t prot;
{
	extern time_value_t *mtime;

	if (prot & VM_PROT_WRITE) return (-1);

	return (ns532_btop(pmap_extract(pmap_kernel(), (vm_offset_t) mtime)));
}

void
inittodr()
{
	time_value_t	new_time;

	new_time.seconds = 0;
	new_time.microseconds = 0;

	(void) readtodc(&new_time.seconds);

	{
	    int	s = splhigh();
	    time = new_time;
	    splx(s);
	}
}

void
resettodr()
{
	writetodc();
}

startrtclock()
{
	pic_start_rtclock();
}
