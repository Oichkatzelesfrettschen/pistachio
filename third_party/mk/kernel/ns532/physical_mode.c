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
 * 13-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: physical_mode.c,v $
 */
/*
 * 	File: ns532/physical_mode.c
 * 	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 * 	Startup code for the pc532 that is run with MMU off.
 */

/*
 * The code in this file is run in physical mode, ie.
 * the MMU is turned OFF.
 *
 * All global symbols are relocated to the addresses where they are
 * located when virtual memory is on. Thus all global symbols are
 * wrong in this file. KERNELBASE is the offset between the mappings.
 *
 * BSS may not be written to before pa_move_bootstrap!
 */

#include <mach/ns532/vm_param.h>
#include <mach/boot_info.h>
#include <sys/varargs.h>
#include <ns532/pmap.h>
#include <ns532/pic.h>
#include <ns532/PC532/board.h>

#define KERNELBASE VM_MIN_KERNEL_ADDRESS

/* Physical Address - convert pointer to variable kvtophys */
#define PA(a) \
    (((char *)a >= (char *)KERNELBASE) \
    ? (char *)a - KERNELBASE \
    : (char *)a)

/* Virtual Address - convert pointer to variable phystokv */
#define VA(a) \
    (((char *)a < (char *)KERNELBASE) \
     ? (char *)a + KERNELBASE \
     : (char *)a)

/* Physical Function */
#define PF(f) ((unsigned int (*)())PA(f))

/* Physical Variable - convert reference to variable kvtophys */
#define PV(v) (*(typeof(&(v)))PA(&(v)))

/* define these to make function calls look normal */
#define PA_printf PF(pa_printf)
#define PA_puts PF(pa_puts)
#define PA_putc PF(pa_putc)
#define PA_putnum PF(pa_putnum)
#define PA_panic PF(pa_panic)
#define PA_ovbcopy_ints PF(pa_ovbcopy_ints)
#define PA_move_bootstrap PF(pa_move_bootstrap)
#define PA_bzero PF(pa_bzero)
#define PA_map_page PF(pa_map_page)
#define PA_map_kernel PF(pa_map_kernel)
#define PA_get_physical_memory_size PF(pa_get_physical_memory_size)

pa_putc(c)
	int c;
{
	unsigned char stat;

	if (c == '\n')
	    PA_putc('\r');

	do {
		stat = *(unsigned char *)(DUART_STAT + DUART_ADDR);
	} while ( (stat & DUART_TX_EMPTY) == 0);

	/* spit out char */
	*(unsigned char *)(DUART_DATA + DUART_ADDR) = c;
}

pa_puts(s)
	char *s;
{
	s = PA(s);
	for ( ; *s; s++)
	    PA_putc(*s);
}

pa_putnum(n)
	unsigned int n;
{
	unsigned int digit, z;
	int i;

	z = 0;
	for (i = 28; i >= 0; i -= 4) {
		digit = n >> i;
		digit &= 0xf;
		if (digit || z) {
			if (digit <= 9)
			    PA_putc(digit + '0');
			else
			    PA_putc(digit + 'a' - 10);
			z = 1;
		}
	}
	if (!z)
	    PA_putc('0');
}

pa_printf(fmt, va_alist)
	char *fmt;
	va_dcl
{
	char *s;
	va_list alist;
	unsigned int x;

	fmt = PA(fmt);

	va_start(alist);
	for ( ; *fmt; fmt++) {
		if (*fmt == '%')
		  garbage:
		    switch (*++fmt) {
			  case 'x':
			    x = va_arg(alist, unsigned int);
			    pa_putnum(x);
			    break;
			  case 's':
			    s = va_arg(alist, char *);
			    PA_puts(s);
			    break;
			  default:
			    goto garbage;
		    }
		else
		    PA_putc(*fmt);
	}
}

pa_panic(s)
	char *s;
{
	PA_printf("pa_panic: %s\n", s);
	asm("bpt");
}

extern vm_offset_t	boot_start;
extern vm_size_t	boot_size;
extern vm_offset_t	load_info_start;
extern vm_size_t	load_info_size;
extern vm_offset_t	kern_sym_start;
extern vm_size_t	kern_sym_size;

extern char	edata[];	/* start of BSS */
extern char	end[];		/* end of BSS */

/*
 *	Moves kernel symbol table, bootstrap image, and bootstrap
 *	load information out of BSS at startup.  Returns the
 *	first unused address.
 */

void pa_ovbcopy_ints(src, dst, size)
	vm_offset_t src;
	vm_offset_t dst;
	vm_size_t   size;
{
	register int *_src;
	register int *_dst;
	register unsigned int count;

	_src = (int *)(src + size);
	_dst = (int *)(dst + size);
	count = size / sizeof(int);

	do {
	    *--_dst = *--_src;
	} while (--count > 0);
}

vm_offset_t pa_move_bootstrap()
{
	struct boot_info *bi = (struct boot_info *)PA(edata);

	vm_offset_t pa_kern_sym_start, pa_boot_start, pa_load_info_start;
	vm_size_t bi_sym_size, bi_boot_size, bi_load_info_size;

	bi_sym_size = bi->sym_size;
	bi_boot_size = bi->boot_size;
	bi_load_info_size = bi->load_info_size;

	if (bi_sym_size == 0)
	    PA_printf("pa_move_bootstrap: kernel symbol size == 0\n");
	if (bi_boot_size == 0) PA_panic("bi_boot_size == 0");
	if (bi_load_info_size == 0) PA_panic("bi_load_info_size == 0");

	pa_kern_sym_start = (vm_offset_t) PA(end);

	/*
	 * Align start of bootstrap on page boundary,
	 * to allow mapping into user space.
	 */
	pa_boot_start = ns532_round_page(pa_kern_sym_start + bi_sym_size);
	pa_load_info_start = pa_boot_start + bi_boot_size;
	
	PA_ovbcopy_ints((vm_offset_t)bi + sizeof(struct boot_info) 
			+ bi_sym_size,
			pa_boot_start,
			bi_boot_size + bi_load_info_size);

	if (bi_sym_size != 0) {
		PA_ovbcopy_ints((vm_offset_t)bi + sizeof(struct boot_info),
				pa_kern_sym_start,
				bi_sym_size);
	}

	/* write values to real variables */
	PV(kern_sym_start) = (vm_offset_t)VA(pa_kern_sym_start);
	PV(boot_start) = (vm_offset_t)VA(pa_boot_start);
	PV(load_info_start) = (vm_offset_t)VA(pa_load_info_start);

	PV(kern_sym_size)  = bi_sym_size;
	PV(boot_size)  = bi_boot_size;
	PV(load_info_size)  = bi_load_info_size;

	return pa_boot_start + bi_boot_size + bi_load_info_size;
}

pa_load_info_print()
{
	struct loader_info *lp 
	    = (struct loader_info *)PA(PV(load_info_start));

	PA_printf("lp=%x\n", lp);
	PA_printf("Load info: text (%#x, %#x, %#x)\n",
		  lp->text_start, lp->text_size, lp->text_offset);
	PA_printf("           data (%#x, %#x, %#x)\n",
		  lp->data_start, lp->data_size, lp->data_offset);
	PA_printf("           bss  (%#x)\n", lp->bss_size);
	PA_printf("           syms (%#x, %#x)\n",
		  lp->sym_offset, lp->sym_size);
	PA_printf("	   entry(%#x, %#x)\n",
		  lp->entry_1, lp->entry_2);
}

pa_bzero(addr, count)
	char *addr;
	unsigned int count;
{
	for ( ; count > 0; count--)
	    addr[count-1] = '\0';
}

/*
 * Map one page. Returns first remaining free page.
 */
vm_offset_t pa_map_page(pa_kpde, phys, virtual, free, print)
	pt_entry_t 	*pa_kpde;   /* PA - first level page table */
	vm_offset_t	phys;	    /* PA */
	vm_offset_t 	virtual;    /* VA */
	vm_offset_t 	free;	    /* PA - first free page before mapping */
	boolean_t	print;	    /* XXX */
{
	pt_entry_t pte, *pdep, *pt, *ptep;

	if (print)
	    PA_printf("pa_map_pg(pa_kpde=%x, phys=%x, virt=%x, free=%x)\n",
		     pa_kpde, phys, virtual, free);

	pdep = &pa_kpde[pdenum(virtual)];
	if ((*pdep & NS532_PTE_VALID) == 0) { /* 2nd level PT missing */
		pt = (pt_entry_t *)free;
		free += NS532_PGBYTES;
		PA_bzero(pt, NS532_PGBYTES);
		*pdep = pa_to_pte((vm_offset_t) pt) 
		    | NS532_PTE_VALID | NS532_PTE_WRITE;
		if (print)
		    PA_printf("pa_map_pagPT<-kpde: pdep=%x, pt=%x, *pdep=%x\n",
			      pdep, pt, *pdep);
	}
	ptep = &((pt_entry_t *)pte_to_pa(*pdep))[ptenum(virtual)];
	if (*ptep & NS532_PTE_VALID) {
		PA_printf("pa_map_page ptep=%x pdep=%x virt=%x phys=%x\n",
			  ptep, pdep, virtual, phys);
		PA_panic("pa_map_page: page already mapped");
	}
	*ptep = pa_to_pte(phys) | NS532_PTE_VALID | NS532_PTE_WRITE;

	if (print)
	    PA_printf("pa_map_page page<--PTE: ptep=%x phys=%x *ptep=%x\n",
		      ptep, phys, *ptep);

	return free;
}
/*
 *	Pass out last_virtual_addr. This is the first free virtual
 * 	memory address for the kernel (just after the mapping of the
 * 	entire physical memory).
 *
 *	last_addr is the first available physical address.
 */
extern int pstart();
extern pt_entry_t *kpde;
extern vm_offset_t last_addr, last_virtual_addr;

vm_offset_t pa_map_kernel(memsize, first_free)
	unsigned int memsize;
	vm_offset_t first_free;
{
	vm_offset_t v_a, p_a, free;
	unsigned int npages, pg;
	pt_entry_t *pa_kpde, *ptep;

	free = ns532_round_page(first_free);

	/* allocate first level PT for kernel */
	pa_kpde = (pt_entry_t *)free;
	PA_printf("pa_map_kernel: Kernel first level page table at %x\n",
		  pa_kpde);
	PA_bzero(pa_kpde, NS532_PGBYTES);
	/* save virtual address in global */
	PV(kpde) = (pt_entry_t *)VA(pa_kpde);
	free += NS532_PGBYTES;

	/*
	 *  Map DRAM at KERNELBASE
	 */

	v_a = (vm_offset_t) KERNELBASE;
	p_a = (vm_offset_t) 0;

	/* map DRAM */
	PA_printf("pa_map_kernel: Mapping DRAM from %x to %x at %x\n",
		  p_a, memsize + p_a, v_a);

	for (pg = 0; pg < memsize; pg += NS532_PGBYTES)
	    free = PA_map_page(pa_kpde, p_a + pg, v_a + pg, free, FALSE);

	/* We've used virtual memory up to here. Tell pmap about it */
	PV(last_virtual_addr) = v_a + pg;

	/* map ICU 1-1 */
	PA_printf("pa_map_kernel: Mapping ICU 1-1 at %x\n", 
		  ns532_trunc_page(ICU_ADDR));
	free = PA_map_page(pa_kpde, ns532_trunc_page(ICU_ADDR), 
			   ns532_trunc_page(ICU_ADDR), free, FALSE);

	/* 
	 * map physical mode code 1-1.
	 *
	 * The transition from physical to virtual memory uses this mapping.
	 * Maps the first 4 MB.
	 */
	PA_printf("pa_map_kernel: Mapping first 4 MB 1-1 at 0\n");
	for (pg = 0; pg < ns532_ptob(1024); pg += NS532_PGBYTES) {
		free = PA_map_page(pa_kpde, pg, pg, free, FALSE);
	}

	PA_printf("pa_map_kernel: Mapping console DUART %x at %x\n", 
		  ns532_trunc_page(DUART_ADDR), DUART_BOOT_VIRTUAL_ADDR);
	free = PA_map_page(pa_kpde, ns532_trunc_page(DUART_ADDR), 
			   DUART_BOOT_VIRTUAL_ADDR, free, FALSE);

	PV(last_addr) = free; /* tell ns532_init about it */

	return (vm_offset_t) pa_kpde;
}

extern unsigned int boothowto;
extern unsigned int mem_size;
extern unsigned int pa_get_physical_memory_size();

physical_mode_start(bootflags)
	int bootflags;
{
	vm_offset_t first_free;	/* first free physical page */
	unsigned int memsize;
	vm_offset_t ptbase;

	PA_puts("Hello World!\n");

	first_free = PA_move_bootstrap();
	PA_printf("physical_mode_start: Moved bootstrap. First free address is %x.\n", first_free);

	PA_printf("physical_mode_start: Clearing BSS from %x to %x\n",
		  PA(edata), PA(end));
	PA_bzero(PA(edata), PA(end) - PA(edata));

	memsize = PA_get_physical_memory_size();
	PA_printf("Physical memory size is %x bytes (0x%x megs)\n",
		  memsize, memsize >> 20);

	/* pass random variables to ns532_init() */
	PV(boothowto) = bootflags;
	PV(mem_size) = memsize;

	ptbase = PA_map_kernel(memsize, first_free);
	PA_printf("returning to assembly code...\n");
	return ptbase;
}
