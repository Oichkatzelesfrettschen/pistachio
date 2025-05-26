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
 * $Log: ns532_init.c,v $
 */
/*
 * 	File: ns532/ns532_init.c
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 *	Early initialization code for the pc532.
 *	This code works together with the initializations done in 
 *	ns532/physical_mode.c.
 */

#include "platforms.h"

#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <vm/vm_page.h>
#include <ns532/machparam.h>

vm_size_t	mem_size = 0;
vm_offset_t	last_addr = 0; /* start.s sets to first free phys addr after
				  kernel and kernel page tables */
vm_offset_t	last_virtual_addr = 0;

vm_offset_t	avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;
vm_offset_t     avail_next;
unsigned int    avail_remaining;

int boothowto = 0;

extern char etext, edata, end;

ns532_init()
{

#if 0
	-- now zeroed in physical_mode.c --
	bzero((char *)&edata, (unsigned)(&end - &edata));
#endif 0
	/*
	 *	Initialize kernel physical map, mapping the
	 *	region from loadpt to avail_start.
	 *	Kernel virtual address starts at VM_KERNEL_MIN_ADDRESS.
	 */
	/*
	 * 	Avail_start and avail_end are now initialized from value
	 * 	passed from physical_mode.c through the last_virtual_addr
	 * 	variable.
	 */
	avail_start = last_addr;
	avail_end = mem_size - NS532_PGBYTES; 
	virtual_avail = last_virtual_addr;
	virtual_end = last_virtual_addr;

	vm_set_page_size();
	pmap_bootstrap(0);  /* argument not used on pc532 */

	/* 	Flush tlb so the new mappings take effect */
	_flush_tlb();

	avail_next = avail_start;
	avail_remaining = atop(avail_end - avail_start);

	/*
	 * Initialize the ICU prior to any possible call to an spl.
	 */
	picinit();

	{ 
		extern int delaycount; 
		printf("ns532_init: delaycount=0x%x\n",delaycount); 
	}

	printf("Kernel from 0x%x: code 0x%x, data 0x%x, bss 0x%x bytes\n",
		    VM_MIN_KERNEL_ADDRESS,
		    (int)(&etext - VM_MIN_KERNEL_ADDRESS),
		    (int)(&edata - &etext),
		    (int)(&end - &edata));
	printf("Available space: physical from 0x%x to 0x%x, 0x%x remaining\n",
	       avail_start, avail_end, avail_remaining);
	printf("                 virtual from 0x%x to 0x%x\n",
	       virtual_avail, virtual_end);

	/*
	 * 	Still need to initialize the com driver here. Should fix it to
	 * 	do normal autoconfiguration and write directly to the UART
	 * 	till then. Now there probably are interrupts generated too
	 * 	early.
	 */
	printf("ns532_init: XXX Initializing COM driver (should be done at autoconfig time)\n");
	cominit();
}

unsigned int pmap_free_pages()
{
	return avail_remaining;
}

boolean_t pmap_next_page(addrp)
	vm_offset_t *addrp;
{
	if (avail_next == avail_end)
		return FALSE;

	*addrp = avail_next;
	avail_next += PAGE_SIZE;
	avail_remaining--;
	return TRUE;
}

#include <sys/varargs.h>
#include <ns532/psl.h>
#include <ns532/PC532/board.h>

boot_putc(c)
	int c;
{
	unsigned char stat;
	unsigned int saved_psr, new_psr;

	if (c == '\n')
	    boot_putc('\r');

	/* 
	 * Disable interrupts. ICU might not be initialized and spl()
	 * can't be used.
	 */
	asm("sprw psr, %0" : "=g" (saved_psr) );
	new_psr = saved_psr & ~PSR_I;
	asm("lprw psr, %0" : : "g" (new_psr) );

	do {
	       stat = *(unsigned char *)(DUART_STAT + DUART_BOOT_VIRTUAL_ADDR);
	} while ( (stat & DUART_TX_EMPTY) == 0);

	/* spit out char */
	*(unsigned char *)(DUART_DATA + DUART_BOOT_VIRTUAL_ADDR) = c;
	do {
	       stat = *(unsigned char *)(DUART_STAT + DUART_BOOT_VIRTUAL_ADDR);
	} while ( (stat & DUART_TX_EMPTY) == 0);

	asm("lprw psr, %0" : : "g" (saved_psr) );
}

delay(n) 
{
	
#if 0
	for ( n /= 10; n >= 0; n--)
	    tenmicrosec();
#else
	delay_us(n);
#endif
}

delay_spl0(n)
{
	int s;
	s = spl0();
	delay(n);
	splx_no_check(s);
}

