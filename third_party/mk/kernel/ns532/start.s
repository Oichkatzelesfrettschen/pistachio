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
 * 26-May-93  Johannes Helander (jvh) at Helsinki University of Technology
 *	Added some ICU reinitialization in drop_to_monitor.
 *
 * 24-May-93  Tero Kivinen (kivinen) at Helsinki University of Technology
 *	Yet another attempt to fix drop_to_monitor.
 *
 * 10-May-93  Johannes Helander (jvh) at Helsinki University of Technology
 *	pstart_memory_nmi did in fact modify the saved psr 4(sp) instead
 *	of the pc 0(sp) as it should have. The bug was found by
 *	mason@reed.edu in BSD (?).
 *
 * 10-May-93  Johannes Helander (jvh) at Helsinki University of Technology
 *	Tried to fix _drop_to_monitor. The ptb was loaded with a wrong
 *	value.
 *
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: start.s,v $
 */
/*
 * 	File: 
 *	Author: Tatu Ylonen, Tero Kivinen, Johannes Helander
 *	Helsinki University of Technology 1992.
 *
 * Startup code for ns532.
 * The kernel is loaded starting at 0x1fe0 and started at 0x2020 (_pstart).
 *
 * When we enter, dram is refreshing, cpu is in sensible mode, mmu is not
 * in use.
 */

#include <ns532/asm.h>
#include <ns532/pic.h>
#include <ns532/psl.h>
#include <ns532/trap.h>
#include <ns532/PC532/board.h>
#include "assym.s"

#include "mach_kdb.h"

#define BOOT_PRINTS

#define BOOTMODE 0x03  /* boot single user */

#define MEGABYTE (1024*1024)

/* interrupt stack */
	.data
	.align	2
	.globl	EX(intstack)
LEX(intstack)
        .space	4096
	.globl	EX(eintstack)
LEX(eintstack)

/*
 * This code expects that _pstart is the first symbol of the kernel
 * image, and that the image is loaded at a page boundary.  Only an a.out
 * header comes before _pstart (actually two: mach.boot and kernel).
 */
	.text
	.align	2
	.globl	EX(pstart)
LEX(pstart)
	movd	BOOTMODE,tos		/* force single-user for now */
	lprd	sb, 0			/* The C compiler expects this */
	jsr	EX(physical_mode_start)-KERNELBASE
	cmpd	tos, 0			/* pop arg */
	/* r0 contains returned pa_kpde */

	sprd	psr, @EX(monitor_psr)-KERNELBASE
	sprd	cfg, @EX(monitor_cfg)-KERNELBASE
	smr	mcr, @EX(monitor_mcr)-KERNELBASE
	sprd	intbase, @EX(monitor_intbase)-KERNELBASE
	sprd	sp, @EX(monitor_sp)-KERNELBASE
	movd	0(r0), @EX(orig_low_map)-KERNELBASE

	lprd	dcr, 0
	lprd	car, 0
	lprd	bpc, 0

	lprw	psr, 0x0000		/* set flags (disables interrupts) */
	lprd	cfg, 0x00000bf6		/* set configuration: f m de dc ic */
#ifdef BOOT_PRINTS
	movd	0x10, r7
	bsr	EX(ledpout)
	movd	r0, r7
	bsr	EX(ledpout)
#endif
	lmr	ptb0, r0		/* set ptb */
	lmr	ptb1, r0		/* set ptb */
	lmr	mcr, MCR_VALUE		/* enable virtual memory translation */
	jump	@EX(kstart)		/* jump to new address */


ENTRY(pa_get_physical_memory_size)
	save	[r3,r4,r5,r6,r7]
	movd	MEGABYTE, r0
	sprd	intbase, r4
	movd	(4*T_NMI)(r4), r5
	movd	EX(pstart_memory_nmi), (4*T_NMI)(r4)
	subd	KERNELBASE, (4*T_NMI)(r4) /* kvtophys */
	movd	@0, r2
	sprd	cfg, r6
	lprd	cfg, 0x1f7		/* de m f i */
pstart_loop:
	movd	0(r0),  r3		/* Save old value */
	movd	0xa5a5a5a5, 0(r0)
	sprd	cfg, r7			/* flush write buffer */
       	lprd	cfg, r7
	cmpd	0xa5a5a5a5, 0(r0)
	bne	EX(pstart_knows_memory)
	movd	0x96969696, 0(r0)
	sprd	cfg, r7			/* flush write buffer */
       	lprd	cfg, r7
	cmpd	0x96969696, 0(r0)
	bne	EX(pstart_knows_memory)
	cmpd	0x96969696, @0		/* watch out for wrap around */
	bne	pstart_loop_continue
	movd	0x12345678, 0(r0)
	sprd	cfg, r7			/* flush write buffer */
       	lprd	cfg, r7
	cmpd	0x12345678, @0
	beq	EX(pstart_knows_memory)
pstart_loop_continue:
	movd	r3, 0(r0)
	addd	MEGABYTE, r0
	br	pstart_loop
	.globl  EX(pstart_knows_memory)
LEX(pstart_knows_memory)
	lprd	cfg, r6
	movd	r3, 0(r0)
	movd	r2, @0			/* don't want to corrupt low memory */
	movd	r5, (4*T_NMI)(r4)	/* restore nmi trap */
	restore [r3,r4,r5,r6,r7]
	ret	0			/* Return memory size in r0 */

	.globl	EX(pstart_memory_nmi)
LEX(pstart_memory_nmi)
	movd	r0, tos
	movd	PARCLU_PHYS, r0
	cmpqd	0, 0(r0)		/* Clear parity error */
	movd	tos, r0
	movd	EX(pstart_knows_memory), 0(sp) /* modify return address */
	subd	KERNELBASE,  0(sp)	/* kvtophys */
	rett	0

#ifdef BOOT_PRINTS

/* 
 * Output r7 on the led display (only while in physical mode).
 * Corrupts no registers.
 */
	.globl	EX(ledpout)
LEX(ledpout)
	save	[r0,r1,r2,r3,r4,r5,r6,r7]
	movd	r7,tos
	movd	DUART_ADDR,r7   /* duart addr */
	movb	0x50,r1
	bsr	EX(poutputc)
	movd	tos,r0
	bsr	EX(poutnum)
	movd	13,r1
	bsr	EX(poutputc)
	movd	10,r1
	bsr	EX(poutputc)
	restore	[r0,r1,r2,r3,r4,r5,r6,r7]
	ret	0

#endif /* BOOT_PRINTS */

/*
 * Outputs r0 in hex on uart0 during boot.
 * r0=num to output, r7=duart addr.  Corrupts r0 and r1.
 */
LEX(poutnum)
	cmpd	r0,15
	bls	EX(poutnum_out)
	movd	r0,tos
	lshd	-4,r0
	bsr	EX(poutnum)
	movd	tos,r0
LEX(poutnum_out)
	andd	15,r0
	cmpd	r0,9
	bls	EX(poutnum_digit)
	addd	55,r0  /* 'A'-10 :-) */
	br	EX(poutnum_do_out)
LEX(poutnum_digit)
	addd	48,r0  /* '0' */
LEX(poutnum_do_out)
	movb	r0,r1
	bsr	EX(poutputc)
	ret	0

/*
 * Outputs a character on uart 0 during boot.
 *  r1=char, r7=duart addr.  Corrupts r0.
 */
LEX(poutputc)
	movb	DUART_STAT(r7),r0
	andb	DUART_TX_EMPTY,r0
	cmpb	0,r0
	beq	EX(poutputc)
	movb	r1,DUART_DATA(r7)
	ret	0

/*
 * This outputs the number in hex on console.  This uses the C calling 
 *  convention.
 *  This can only be used during boot (in virtual mode, first page still
 *  mapped).
 */
	.globl	EX(boutnum)
LEX(boutnum)
	FRAME
	movd	B_ARG0,r0
	movd	r7,tos
	movd	r0,tos
#ifdef DUART_AT_ZERO			/* ylo kludge */
	movd	0,r7
#else					/* jvh kludge */
	movd	DUART_BOOT_VIRTUAL_ADDR, r7
#endif
	movd	0x42,r1
	bsr	EX(poutputc)
	movd	tos,r0
	bsr	EX(poutnum)
	movd	13,r1
	bsr	EX(poutputc)
	movd	10,r1
	bsr	EX(poutputc)
	movd	tos,r7
	EMARF
	ret	0

/*
 * We are now running virtual with correct addresses
 */
	.globl	EX(kstart)
LEX(kstart)
	lprd	intbase, EX(intvectors)	/* set interrupt base */
	lprd	sp, EX(eintstack)	/* set kernel stack */
	lprd	sb, 0			/* some C code may expect this */
	lprd	fp, 0			/* to make debugger traces cleaner */
#ifdef BOOT_PRINTS
	movd	0x20, tos
	bsr	EX(boutnum)
	adjspb	-4
#endif /* BOOT_PRINTS */
	bsr	EX(machine_startup)	/* call C code - never returns */
	dia

ENTRY(_drop_to_monitor)
	FRAME
	bicpsrw	(PSR_I | PSR_T)
 	movd	@EX(kpde), r0		/* Load the kernel pde */
	movd	@EX(orig_low_map),0(r0)	/* Restore the 1st level pte */
	subd	KERNELBASE, r0		/* Convert kernel pde to phys */
	lmr	ptb1, r0		/* Change to kernel mapping */
	lmr	ptb0, r0
	/* make pc point to low memory */
	movd	EX(drop_to_monitor_novirt)-KERNELBASE,r4
	jump	0(r4)

	.globl	EX(drop_to_monitor_novirt)
LEX(drop_to_monitor_novirt)
	/* This code executes using "wrong" pc addresses. */
	movd	ICU_ADDR, r5		/* ICU base */
	orb	0x80, ICU_PDAT(r5)	/* Select aic */
	movd	PC532_SCSI_BASE, r6	/* scsi base */
	lmr	mcr, @EX(monitor_mcr)	/* after this we are in REAL MODE! */
	movb	0xd, 0(r6)		/* select scsi register 13 */
	movb	0xe, 1(r6)		/* 1 on hex panel, 00001110 on leds */
	lprd	sp, @EX(monitor_sp)-KERNELBASE
	movb	0xd, 1(r6)		/* 2 on hex panel, 00001101 on leds */
	lprd	cfg, @EX(monitor_cfg)-KERNELBASE
	movb	0xc, 1(r6)		/* 3 on hex panel, 00001100 on leds */
	lprd	intbase, @EX(monitor_intbase)-KERNELBASE
	movb	0xb, 1(r6)		/* 4 on hex panel, 00001011 on leds */
	movb	0xff, ICU_IMSK(r5)	/* mask off intrs */
	movb	0xff, ICU_IMSK+1(r5)
	movb	0xa, 1(r6)		/* 5 on hex panel, 00001010 on leds */
	lprd	psr, @EX(monitor_psr)-KERNELBASE
	movb	0x09, 1(r6)		/* 6 on hex panel, 00001001 on leds */
	andb	0x7f, ICU_PDAT(r5)	/* Select dp */
	bpt				/* back to monitor */
	dia

	.data
	.align ALIGN
DATA(monitor_cfg)
	.long 0
DATA(monitor_mcr)
	.long 0
DATA(monitor_intbase)
	.long 0
DATA(monitor_sp)
	.long 0
DATA(orig_low_map)
	.long 0
DATA(orig_disp)
	.long 0
DATA(monitor_psr)
	.long 0
	.text

/* EOF */
