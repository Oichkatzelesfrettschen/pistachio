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
 * $Log: locore.s,v $
 * Revision 1.88  1992/05/14  18:08:26  jvh
 * 	Don't save last_intr_number in interrupt handler.
 *
 * Revision 1.87  1992/05/14  12:56:53  jvh
 * 	Remove unnecessary check for SPL0 in t_nvi_interrupt.
 *
 * Revision 1.86  1992/05/14  12:11:58  jvh
 * 	Changed #ifdef LEDSNEW to #if HEXPANEL.
 *
 * Revision 1.85  1992/05/14  11:50:14  jvh
 * 	Conditionalize code in t_nvi_interrupt for SPLDEBUG.
 * 	Include ipl.h, spldebug.h, hexpanel.h.
 *
 * Revision 1.84  1992/05/13  16:39:34  jvh
 * 	Fixes to use new spl routines written in C.
 *
 * Revision 1.83  1992/05/11  18:34:02  kivinen
 * 	Added interrupt routine, so ddb can detect interrupts in stack
 * 	trace.
 *
 * Revision 1.82  1992/05/11  11:38:38  jvh
 * 	Call primitive_splsched instead of splsched.
 *
 * Revision 1.81  1992/05/11  04:49:11  jvh
 * 	Moved copyin/copyout back here.
 *
 * Revision 1.80  1992/05/11  04:08:29  jvh
 * 	Moved context switch routines to cswitch.s and copyin/copyout
 * 	to copyout.s.
 *
 * Revision 1.79  1992/05/10  22:28:06  kivinen
 * 	Fixed delay function.
 *
 * Revision 1.78  1992/05/08  21:44:12  jvh
 * 	Fixed off by one error in copyin/copyout.
 * 	Check for addr+count excedding VM_MAX_ADDRESS by bhi, NOT bhs.
 *
 * Revision 1.77  1992/05/05  20:11:42  jvh
 * 	Made interrupt handler save last_intr_number.
 *
 * Revision 1.76  1992/05/04  15:04:03  jvh
 * 	Moved start code out from locore.s to start.s.
 *
 * Revision 1.75  1992/04/24  17:55:15  jvh
 * 	New delay.
 *
 * Revision 1.74  1992/04/17  03:09:34  jvh
 * 	Temporarily rename delay as delay_us.
 *
 * Revision 1.73  1992/04/11  00:09:15  jvh
 * 	Enabled saving of monitor stack etc. drop_to_monitor is still
 * 	broken though.
 *
 * Revision 1.72  1992/04/08  18:22:50  jvh
 * 	Now most of the bootstrap code is written in C.
 *
 * Revision 1.71  1992/04/06  14:05:14  jvh
 * 	Moved bootstrap copying to C code (physical_mode.c).
 *
 * Revision 1.70  1992/03/24  05:34:47  jvh
 * 	Fixes for out of kernel default pager. Copy bootstrap out of
 * 	BSS.
 *
 * Revision 1.69  1992/03/21  01:10:11  jvh
 * *** empty log message ***
 *
 * Revision 1.68  1992/03/21  01:08:37  jvh
 * 	Removed fixed lower limit for emulation vector.
 *
 * Revision 1.67  1991/10/28  00:03:46  jvh
 * 	Made t_nvi_interrupt get unit number from iunit table instead
 * 	of giving the interrupt number directly as argument to
 * 	interrupt routine.
 *
 * Revision 1.66  1991/10/25  03:38:36  jvh
 * 	Added delay().
 *
 * Revision 1.65  1991/08/03  03:19:38  kivinen
 * Fixed bug in memory size test. Loaded wrong address to interrupt
 * table.
 *
 * Revision 1.64  91/07/25  13:27:42  kivinen
 * Tried to fix drop_to_monitor.
 * 
 * Revision 1.63  91/07/23  07:35:39  kivinen
 * Fixed drop_to_monitor code. Added splhi at beginning, and reorganized
 * instructions after drop_to_monitor_novirt.
 * 
 * Revision 1.62  91/07/15  08:17:44  jvh
 * 	Minor cleanup.
 * 
 * Revision 1.61  91/07/15  07:55:57  jvh
 * 	Renamed _flush_instruction_cache --> ns532_flush_instruction_cache.
 * 	Added ns532_flush_instruction_cache.
 * 
 * Revision 1.60  91/07/15  07:48:48  jvh
 * 	Removed unused MK42 (KEEP_STACKS) context switch routines.
 * 
 * Revision 1.59  91/06/30  01:04:16  jvh
 * 	Removed LEDS0 code.
 * 	Added code to output numbers on uart0 during boot.  Old
 * 	ledpout now uses it.  boutnum (C-callable) can be used to
 * 	output numbers while the first page is still mapped.
 * 	//ylo
 * 
 * Revision 1.58  91/05/11  22:45:46  jvh
 * 	Made TRAPENTRY0 save tear and msr but not ksp.
 * 	Removed old (MK42) AST junk.
 * 
 * Revision 1.57  91/05/11  22:43:07  jvh
 * 	Made ABT trap handler enable interrupts in the normal case.
 * 
 * Revision 1.56  91/05/11  03:37:08  jvh
 * 	Fixed copyright.
 * 
 * Revision 1.55  91/05/03  02:37:40  jvh
 * 	Figured out Yet Another CPU bug (Actually two) together with
 * 	juki. MOVUSD is a lot of fun, ain't it?
 * 
 * Revision 1.54  91/04/26  01:18:39  jvh
 * 	Undid kivinen's change to be able to find out where the
 * 	problem is. The correct solution is in any case to use movusd.
 * 	If there are problems with this they are somewhere else.
 * 
 * Revision 1.53  91/04/24  18:33:38  kivinen
 * Fixed syscall_copy_loop, and changed movusd back to movd. movusd did
 * not work.
 * 
 * Revision 1.52  91/04/24  06:04:31  jvh
 * 	Made syscall_copy_loop use movusd instead of movd.
 * 
 * Revision 1.51  91/04/23  22:06:54  jvh
 * 	Made call_continuation disable the FPU.
 * 
 * Revision 1.50  91/04/21  07:00:41  jvh
 * 	Added led display code to context switch routines.
 * 	New copyright.
 * 
 * Revision 1.49  91/04/19  23:04:23  jvh
 * 	The interrupts incorrectly returned without checking ASTs.
 * 	Fixed this.
 * 
 * Revision 1.48  91/04/18  00:22:08  jvh
 * 	changed Thread_continue to use jsr instead of jump to call the
 * 	real continuation.
 * 
 * Revision 1.47  91/04/15  03:53:50  jvh
 * 	Renamed switch_context --> Switch_context. switch_context is
 * 	now written in C.
 * 
 * Revision 1.46  91/04/15  00:11:24  jvh
 * 	Added call_continuation.
 * 	Changed AST interface in accordance with MK43 semantics.
 * 	MK43 changes to load_context, switch_context.
 * 	Added Thread_continue.
 * 
 * Revision 1.45  91/04/14  06:33:37  jvh
 * 	Renamed switch_task_context --> switch_context.
 * 
 * Revision 1.44  91/04/13  03:21:25  jvh
 * 	Minor cleanups.
 * 
 * Revision 1.43  91/04/12  23:59:17  jvh
 * 	Made offsets to ns532_fp_state (in _fpload/_fp_save) symbolic
 * 	(defined in genassym.c). 
 * 
 * Revision 1.42  91/03/30  11:50:30  jvh
 * 	Optimized copyin, copyout.
 * 
 * Revision 1.41  91/03/30  09:38:09  jvh
 * 	Removed unused save_context. Ifdeffed out _fpu_[gs]et_fsr.
 * 	Added comment about TLB1 usage.
 *      Made saving of ksp conditional in traps. If you don't want it
 * 	saved you should define TRAP_DONT_SAVE_KSP. Only the debugger
 * 	needs it saved.
 * 	
 * Revision 1.40  91/03/28  21:24:46  jvh
 * 	Minor FPU fixes.
 * 
 * Revision 1.39  91/03/28  13:49:07  kivinen
 * Fixed ns381 code, added _disable_fpu function.
 * 
 * Revision 1.38  91/03/28  13:45:06  jvh
 * 	Added _fp{load,save}_ns{081,381}
 * 
 * Revision 1.37  91/03/27  21:09:24  jvh
 * 	Added function _enable_fpu, made load_context and
 * 	switch_task_context disable fpu.
 * 
 * Revision 1.36  91/03/27  17:35:41  jvh
 * 	Moved PC patching (increment) in syscall_native to be done
 * 	before doing the syscall_call. The new thread_syscall_return
 * 	otherwise didn't get it's stack patched.
 * 
 * Revision 1.35  91/03/26  20:24:07  jvh
 * 	Fixed load_context and switch_task_context. The new
 * 	context switch implementation now seems to work. Float code is
 * 	still missing.
 * 
 * Revision 1.34  91/03/24  01:25:22  jvh
 * 	Fixed syscall_native to get kernel stack instead of kernel
 * 	state. Made the function better correspond to the i386 version.
 * 
 * Revision 1.33  91/03/23  23:52:13  jvh
 * 	Removed floating point regs from trapentry and trapexit.
 * 
 * Revision 1.32  91/03/15  04:56:28  jvh
 * 	Changes to upgrade to new context switch code:
 * 	Added [Ss]witch_task_context.
 * 	Removed save_context.
 * 	Fixed [Ll]oad_context according to the i386 changes.
 * 
 * Revision 1.31  91/03/13  07:51:25  jvh
 * 	Fixed comment syntax on line 676. 
 * 
 * Revision 1.30  91/03/11  02:12:51  jvh
 * 	Fixed header. Removed personal copyrights.
 * 
 * Revision 1.29  91/03/11  02:09:55  jvh
 * 	Removed T from LEXT and EXT.
 * 
 * Revision 1.28  91/03/11  01:34:51  kivinen
 * Changed to new Mach copyright, __STDC__ support, copyoutmsg, copyinmsg
 * aliases, thread_exception_return, thread_syscall_return.
 * 
 * Revision 1.27  91/03/07  22:48:17  kivinen
 * Removed one led display in drop_to_monitor_novirt.
 * 
 * Revision 1.26  91/03/02  23:48:50  kivinen
 * Fixed returning to monitor by writing first entry to current page
 * table too.
 * 
 * Revision 1.25  91/02/26  23:14:57  kivinen
 * Removed _get_fsr. Renamed fpu_read_fsr to fpu_get_fsr.
 * 
 * Revision 1.24  91/02/26  22:23:22  kivinen
 * Changed interrupts to use TRAPENTRY0 and TRAPEXIT so it will now save
 * floating point registers. This is needed if we do context switch in
 * hardclock.
 * 
 * Revision 1.23  91/02/26  00:46:48  kivinen
 * Fixed code in pstart_memory_nmi, so it will now clear nmi error.
 * 
 * Revision 1.22  91/02/21  00:55:24  jvh
 * removed some copyin/copyout related garbage
 * 
 * Revision 1.21  91/02/12  23:16:17  kivinen
 * Fixed typo.
 * 
 * Revision 1.20  91/02/12  23:14:48  kivinen
 * Added code to clear parity error.
 * 
 * Revision 1.19  91/02/11  23:06:25  kivinen
 * Removed second set_mcr.
 * 
 * Revision 1.18  91/02/11  23:04:01  kivinen
 * Added new functions to set/get special ns532 registers, like debug
 * registers etc.
 * 
 * Revision 1.17  91/02/09  21:06:39  kivinen
 * Added more led displays.
 * 
 * Revision 1.16  91/02/09  20:15:53  kivinen
 * Added svc to print system call to led panel with new led routines.
 * 
 * Revision 1.15  91/02/09  09:02:11  kivinen
 * Modified t_nvi_interrupt to use set_spl_mask instead of set_spl.
 * 
 * Revision 1.14  91/02/08  18:42:41  kivinen
 * Added moving past svc in syscall_emul.
 * 
 * Revision 1.13  91/02/07  00:20:54  kivinen
 * Added flushing of tlb for ptb1 too in flush_tlb, and invalidate_page. 
 * 
 * Revision 1.12  91/02/06  16:24:10  kivinen
 * Fixed typo.
 * 
 * Revision 1.11  91/02/06  16:22:31  kivinen
 * Rewrote copyin/copyout to use movsub/movusb.
 * 
 * Revision 1.10  91/02/06  13:52:02  kivinen
 * Testing if movsu/movus will work if we copy ptb0 to ptb1. Modified all
 * loads to ptb0 to load ptb1 too. Ifdeffed jvh's code out, and tatus
 * code in...
 * 
 * Revision 1.9  91/02/06  05:55:35  jvh
 * Rewrote copyin, copyout.
 * 
 * Revision 1.8  91/02/05  03:40:26  jvh
 * Changed movw in emulator support code to movd. I guess it was a typo.
 * 
 * Revision 1.7  91/02/04  22:41:09  ylo
 * fixed bugs in copyin, copyout (screwed up with stack frames)
 * 
 * Revision 1.6  91/02/04  22:22:08  ylo
 * fixed a couple of bugs in my previous fix for copyin and copyout.  //ylo
 * 
 * Revision 1.5  91/02/04  21:59:01  jvh/ylo
 * fixed kivinen's typo in clearing debug registers
 * 
 * Revision 1.4  91/02/04  21:58:18  ylo
 * changed copyin and copyout so that they don't use rdval and wrval 
 * (we cannot use them since they don't page in and thus don't 
 * lazy-evaluate the pte's).  //ylo
 * 
 * Revision 1.3  91/02/04  17:06:47  kivinen
 * Added clearing of debug registers. 
 * 
 * Revision 1.2  91/02/02  05:16:50  jvh
 * Changed the order in which psr + return address is set up for return 
 * from emulator. The emulator naturally can't return with rett since it 
 * is a privileged instruction. 
 * 
 * Revision 1.1  91/01/26  18:39:18  ylo
 * Initial revision
 * 
 */
/*
 * 	File: ns532/locore.s
 *	Author: Johannes Helander, Tero Kivinen, Tatu Ylonen
 *	Helsinki University of Technology 1992.
 *
 *	Various low level functions for the 32532.
 */

#include <ns532/asm.h>
#include <ns532/pic.h>
#include <ns532/psl.h>
#include <ns532/ipl.h>
#include <ns532/trap.h>
#include "assym.s"

#include "mach_kdb.h"
#include "spldebug.h"
#include "hexpanel.h"

/*
 * Traps and interrupts.
 */

/*
 * Fault recovery.
 */

#define RECOVER_TABLE_START	\
	.text	2		;\
	.globl	EX(recover_table) ;\
LEX(recover_table)		;\
	.text

#define	RECOVER(addr)		\
	.text	2		;\
	.long	9f		;\
	.long	addr		;\
	.text			;\
9:

#define	RECOVER_TABLE_END		\
	.text	2			;\
	.globl	EX(recover_table_end)	;\
LEX(recover_table_end)			;\
	.text

/*
 * Allocate recovery table.
 */
	RECOVER_TABLE_START

#define TRAPENTRY0(name,code) \
ENTRY(name)			;\
	save	[r0,r1,r2,r3,r4,r5,r6,r7] ;\
       	sprd	usp,tos ;\
	sprd	fp,tos  ;\
	sprd	sb,tos  ;\
	sprd	mod,tos ;\
	smr	tear,tos ;\
	smr	msr,tos ;\
	movd	code,tos

#if HEXPANEL
#define TRAPENTRY(name,code) \
	TRAPENTRY0(name,code) ;\
	lprd	sb,0 ;\
	movd	code,tos ;\
	bsr	EX(leds_trap) ;\
	adjspb	-4 ;\
	br	EX(trap_common)
#else HEXPANEL
#define TRAPENTRY(name,code) \
	TRAPENTRY0(name,code) ;\
	br	EX(trap_common)
#endif HEXPANEL

#define TRAPEXIT \
	adjspb	-12 ;\
	lprd	mod,tos ;\
	lprd	sb,tos ;\
	lprd	fp,tos ;\
	lprd	usp,tos ;\
	restore	[r0,r1,r2,r3,r4,r5,r6,r7] ;\
	rett	0

TRAPENTRY(t_nmi,T_NMI)
/* TRAPENTRY(t_abt,T_ABT) special case -- enable interrupts if ok */
TRAPENTRY(t_slave,T_SLAVE)
TRAPENTRY(t_ill,T_ILL)
/* TRAPENTRY(t_svc,T_SVC) handled specially below */
TRAPENTRY(t_dvz,T_DVZ)
TRAPENTRY(t_flg,T_FLG)
TRAPENTRY(t_bpt,T_BPT)
TRAPENTRY(t_trc,T_TRC)
TRAPENTRY(t_und,T_UND)
TRAPENTRY(t_rbe,T_RBE)
TRAPENTRY(t_nbe,T_NBE)
TRAPENTRY(t_ovf,T_OVF)
TRAPENTRY(t_dbg,T_DBG)
TRAPENTRY(t_reserved,T_RESERVED)

/* All traps must create a save area matching the structure in thread.h
 * on kernel stack (sp points to the beginning of the structure).
 * This must include all registers which can be changed by a C function
 * call, and also any other registers that might be of interest in
 * debugging or for other purposes as well as any register that should 
 * be contexty switched (the FPU is an exception -- it is handled specially).
 */

	.globl	EX(trap_common)
LEX(trap_common)
	lprd	sb,0		/* some C code may expect this */
	sprd	sp,tos		/* arg: a pointer to reg structure */
	bsr	EX(trap)	/* call generic trap handler */
	adjspb	-4		/* pop parameter */

	.globl	EX(kernel_return)
/*	.globl	EX(thread_bootstrap) */
LEX(kernel_return)
/* LEX(thread_bootstrap) */
	movw	PSR_U,r0	/* user mode flag mask */
	andw	R_PSR(sp),r0	/* test the flag in caller psr */
	cmpqw	0,r0		/* no ast in system mode */
	beq	EX(trap_return)
trap_call_ast:
	cmpqd	0,@EX(need_ast) /* if we need AST */
	beq	EX(trap_return)

	bsr	EX(primitive_splhi)	/* disable interrupts */
	bsr	EX(ast_taken)	/* take AST */
	br	trap_call_ast	/* and check again (rare) */
	/* ASTs after this point will have to wait */

	.globl	EX(trap_return)
LEX(trap_return) 
	TRAPEXIT


/* 
 * t_abt is like any normal trap except that it enables interrupts 
 * iff they were enabled before the trap. ns32k CPUs clear the PSR_I
 * bit in ABT traps. We want to save tear and msr and then enable 
 * interrupts again. An exception is the case where interrupts were 
 * already disabled when the trap occured. What in fact matters is 
 * if we are on spl7, otherwise interrupts will anyways be enabled when
 * vm_map_lookup calls splx().
 */
TRAPENTRY0(t_abt, T_ABT)
	tbitw	PSR_I_BIT_NUMBER, R_PSR(sp)
	bfc	EX(t_abt_dont_enable_interrupts)
	bispsrw	PSR_I
LEX(t_abt_dont_enable_interrupts)
#if HEXPANEL
	lprd	sb,0 
	movd	T_ABT,tos 
	bsr	EX(leds_trap)
	adjspb	-4 
#endif HEXPANEL
	br	EX(trap_common)

/*
 *    Called as a function, makes the current thread
 *    return from the kernel as if from an exception.
 */

	.globl	EX(thread_exception_return)
	.globl	EX(thread_bootstrap_return)
LEX(thread_exception_return)
LEX(thread_bootstrap_return)
	sprd	sp,r0				/* get kernel stack */
	ord	KERNEL_STACK_SIZE-1,r0
	addd	-K_SIZE-R_SIZE+1,r0
	lprd	sp,r0
	br	EX(kernel_return)

/*
 *    Called as a function, makes the current thread
 *    return from the kernel as if from a syscall.
 *    Takes the syscall's return code as an argument.
 */

	.globl	EX(thread_syscall_return)
LEX(thread_syscall_return)
	movd	S_ARG0,r1
	sprd	sp,r0              	/* perform stack unwind */
	ord	KERNEL_STACK_SIZE-1,r0
	addd	-K_SIZE-R_SIZE+1,r0    	/* r0 now points to the lowest byte */
					/* of ns532_saved_state */
	lprd	sp,r0
	movd	r1,R_R0(sp)
	br	EX(kernel_return)


	.globl	EX(call_continuation)
LEX(call_continuation)
	movd 	S_ARG0, r0 		/* get continuation */
	sprd 	sp, r2 			/* get kernel stack */
	ord 	KERNEL_STACK_SIZE-1, r2
	addd 	-K_SIZE-R_SIZE+1, r2
	lprd	sp, r2 			/* pop the stack */
	lprd	fp, 0 			/* zero frame pointer */

	sprd	cfg, r1				/* invalidate FPU */
	bicd	2, r1				/* clear F (float) bit */
	lprd	cfg, r1

#if HEXPANEL
	movd	r0, tos
	movd	0xc, tos
	movd	LED_F_CSW, tos
	jsr	EX(leds_f)
	adjspb	-8
	movd	tos, r0
#endif HEXPANEL
	jump 	0(r0) 			/* goto continuation */


/*
 * All interrupts enter here.
 */
TRAPENTRY0(t_nvi_interrupt,T_NVI)
	lprd	sb,0			/* some C code may expect this */
	movd	@EX(pic_addr),r7	/* get icu addr */
	movb	ICU_SVCT(r7),r6		/* get interrupt vector number */
	andd	0xf,r6
#if HEXPANEL
	movd	r6,tos
	bsr	EX(leds_int)
	cmpqd	0,tos
#endif HEXPANEL
	movzbd	@EX(intpri)[r6:b],r3	/* get interrupt priority */
#if SPLDEBUG
	movd	r3,tos
       	bsr	EX(primitive_spl_with_check) /* returns old ipl in r0 */
	cmpqd	0, tos
#else SPLDEBUG
#if 0	/* Never SPL0. Check pic.c. */
	cmpqd	SPL0, r3
	bne	notsplzero
	bsr	EX(primitive_spl0)
	br	spldone
notsplzero:
#endif 0
	cmpd	SPLHI, r3
	bne	notsplhi
	bsr	EX(primitive_splhi)
	br	spldone
notsplhi:
	movd	r3, tos
	bsr	EX(primitive_spln)
	cmpqd	0, tos
spldone:
#endif SPLDEBUG
	movb	ICU_HVCT(r7),r1		/* fake end of interrupt to icu */
	sprd	sp,tos 		/* pointer to thread_state (see hardclock.c) */
	movd	r0,tos			/* old ipl level (arg 2) */
	movd	r6,tos			/* vector */
	bsr	EX(interrupt)		/* Call interrupt handler */
	cmpqd	0,tos			/* Remove vector */
	movd	tos,r0		    	/* old ipl */
	bsr	EX(set_spl_mask)    	/* reset icu mask register to old val*/
	cmpqd	0,tos			/* remove psr from stack */
	/* TRAPEXIT */		    /* Note this uses rett to return */
				    /* sic! We have already acknowledged */
				    /* the interrupt and thus cannot use */
				    /* reti */
	br	EX(kernel_return)   /* we must check ast here.  kernel_return
				       does just that. */

/*

call interrupt handler.
interrupt(vector_number, old_spl, pointer_to_thread_state)

*/

ENTRY(interrupt)
	FRAME
	movd	B_ARG2,tos		/* pointer to thread_state (arg 3) */
	movd	B_ARG1,tos	    	/* old spl level (arg 2) */
	movd	B_ARG0,r0		/* get vector */
	movd	@EX(iunit)[r0:d],tos	/* look up unit from vector (arg 1) */
	movd	@EX(ivect)[r0:d],r0	/* get handler address */
	jsr	0(r0) 		  	/* call interrupt handler */
	adjspb	-12		    	/* remove arguments from stack */
	EMARF
	ret 0
		
/*

System calls come through an svc trap.  r0 on entry must contain system
call number (it is saved in its respective position on stack).

*/

TRAPENTRY0(t_svc,T_SVC)
        /* Check for MACH or emulated system call */

	lprd	sb,0			/* some C code may expect this */
#if HEXPANEL
	movd	R_R0(sp),tos
	bsr	EX(leds_syscall)
	adjspb	-4
#endif HEXPANEL
	movd	@EX(active_threads),r2	/* point to current thread */
	movd	TH_TASK(r2),r0		/* point to task */
	movd	TASK_EMUL(r0),r0	/* get emulation vector */
	cmpqd	0,r0			/* if none, */
	beq	EX(syscall_native)        /*   do native system call */
	movd	R_R0(sp),r1		/* get system call number */
	subd	DISP_MIN(r0),r1		/* get displacement into syscall */
					/* vector table */
	cmpd	r1,0
	blt	EX(syscall_native)	/* too low - native system call */
	cmpd	r1,DISP_COUNT(r0)	/* check range */
	bge	EX(syscall_native)	/* too high - native system call */
	movd	DISP_VECTOR(r0)[r1:d],r0
	cmpqd	0,r0
	bne	EX(syscall_emul)	/* emulate the call */

/* Native system call. */

	.globl	EX(syscall_native)
LEX(syscall_native)
	/* r0 = _active_threads */
	movd	TH_KERNEL_STACK(r2),r0	/* point to kernel stack */
	sprd	sp,r3			/* save sp in r3  across system call */
	addqd	1,R_PC(r3)	/* move past the svc instruction */
	movd	R_R0(sp),r1		/* get system call number */
	negd	r1,r1			/* negate it */
	cmpd	r1,0			/* if was positive */
	blt	EX(syscall_range_error)	/*   then error */
	cmpd	r1,@EX(mach_trap_count) /* check range */
	bge	EX(syscall_range_error)

	/* Each struct describing a syscall has 4 ints */
	ashd	4,r1                    /* multiply by 16 */
	movd	EX(mach_trap_table+4)(r1),r0	/* get proc */
	movd	EX(mach_trap_table)(r1),r1	/* get number of args */

	cmpqd	0,r1
	beq	EX(syscall_call)
	addr	(R_USP(sp))[r1:d],r2	/* address of last argument */
	cmpd	r2,VM_MAX_ADDRESS	/* error if not in user space */
	bgt	EX(syscall_addr_error)

	.globl	EX(syscall_copy_loop)
LEX(syscall_copy_loop)
#if 0
	sprd	sp, r5		/* save sp so we can look and feel cpu bug */

	movusd	0(r2), r4	/* Use user mode protections for user space. */
	movd	r4, tos		/* Use two instructions to decrement sp */
				/* CPU undocumented BUG number 523 :-) */
				/* movusd destination can't be tos */
#endif 0
	adjspb	4		/* decrement manually */
	RECOVER(EX(syscall_addr_error))
	movusd	0(r2), 0(sp)
				/* CPU undocumented BUG number 524 :-( */
				/* movusd destination can't be a register */

	addqd	-4,r2
	acbd	-1,r1,EX(syscall_copy_loop)
	.globl	EX(syscall_call)
LEX(syscall_call)		/* r0=addr of call, r3=sp without args */
	jsr	0(r0)
	lprd	sp,r3
	movd	r0,R_R0(r3)
#if HEXPANEL
	movd	0,tos
	bsr	EX(leds_syscall)
	adjspb	-4
#endif HEXPANEL
	br	EX(kernel_return)

/* Address out of range.  Change to page fault.  r2 is faulting address
   r3 points to reg struct */

	.globl	EX(syscall_addr_error)
LEX(syscall_addr_error)			/* emulate page fault */
	lprd	sp,r3
	lmr	tear,r2			/* set error address */
	movd	T_ABT,R_TRAPNO(r3)	/* set trap number */
	br	EX(trap_common)	/* treat as a trap */

/* System call out of range.  Treat as invalid-instruction trap. */

	.globl	EX(syscall_range_error)
LEX(syscall_range_error)		/* emulate invalid instruction */
	movd	T_UND,R_TRAPNO(sp)	/* set error address */
	br	EX(trap_common)

/* user space emulation of system calls.  r0 is vector */

	.globl	EX(syscall_emul)
LEX(syscall_emul)
	movd	R_USP(sp),r2		/* get user stack pointer */
	cmpd	r2,VM_MAX_ADDRESS	/* in user space? */
	bgt	EX(syscall_addr_error)
	addqd	-4,r2
	cmpd	r2,(VM_MIN_ADDRESS+4)	/* check if still space on stack */
	blt	EX(syscall_addr_error)
	movd	R_PC(sp),r1
	addqd	1,r1			/* move past the svc instruction */
	RECOVER(EX(syscall_addr_error))
	movd	r1,0(r2)	/* set up special frame on user stack */
		/* changed movw above to movd //jvh */
		/* emul return: params, retfromlibr, retfromemul, psr */
	addqd	-4,r2			/* r2 must be on correct page */
	RECOVER(EX(syscall_addr_error))
	movd	R_PSR(sp),0(r2)
	movd	r2,R_USP(sp)		/* set new user stack pointer */
	movd	r0,R_PC(sp)		/* set new user ip */
	br	EX(kernel_return)

/* Hardware interrupt/trap vector table.  Intbase points here. */

	.globl	EX(intvectors)
LEX(intvectors)
	.long	EX(t_nvi_interrupt)
	.long	EX(t_nmi)
	.long	EX(t_abt)
	.long	EX(t_slave)
	.long	EX(t_ill)
	.long	EX(t_svc)
	.long	EX(t_dvz)
	.long	EX(t_flg)
	.long	EX(t_bpt)
	.long	EX(t_trc)
	.long	EX(t_und)
	.long	EX(t_rbe)
	.long	EX(t_nbe)
	.long	EX(t_ovf)
	.long	EX(t_dbg)
	.long	EX(t_reserved)
/* since we use non-vectored mode with interrupts, we should never get any
   other interrupts. */

/*
 * FPU routines.
 */

/* 
 * Copy FPU regs from FPU to ns532_machine_state. 
 * Invalidate FPU.
 * Should be done at splsched or higher.
 *
 * _fpsave(struct ns532_fp_state *)
 */

#ifndef NS32081
ENTRY(_fpsave_ns381)
	bsr	EX(_enable_fpu)
	movd	S_ARG0, r1
	sfsr	NS532_FP_STATE_FSR(r1)	     /* save fsr first before we possibly modify it */
	movl	f0, NS532_FP_STATE_L0A(r1)
	movl	f2, NS532_FP_STATE_L2A(r1)
	movl	f4, NS532_FP_STATE_L4A(r1)
	movl	f6, NS532_FP_STATE_L6A(r1)
	movl	f1, NS532_FP_STATE_L1A(r1)
	movl	f3, NS532_FP_STATE_L3A(r1)
	movl	f5, NS532_FP_STATE_L5A(r1)
	movl	f7, NS532_FP_STATE_L7A(r1)
	bsr	EX(_disable_fpu)
	ret	0
#endif NS32081

ENTRY(_fpsave_ns081)
	bsr	EX(_enable_fpu)
	movd	S_ARG0, r1
	/* save fsr first before we possibly modify it */
	sfsr	NS532_FP_STATE_FSR(r1) 
	movl	f0, NS532_FP_STATE_L0A(r1)
	movl	f2, NS532_FP_STATE_L2A(r1)
	movl	f4, NS532_FP_STATE_L4A(r1)
	movl	f6, NS532_FP_STATE_L6A(r1)
	bsr	EX(_disable_fpu)
	ret	0

/* 
 * Copy FPU regs from ns532_machine_state to FPU.
 * Validate FPU.
 * Should be done at splsched or higher.
 *
 * _fpload(struct ns532_fp_state *)
 */

#ifndef NS32081
ENTRY(_fpload_ns381)
	bsr	EX(_enable_fpu)
	movd	S_ARG0, r1
	movl	NS532_FP_STATE_L0A(r1), f0
	movl	NS532_FP_STATE_L2A(r1), f2
	movl	NS532_FP_STATE_L4A(r1), f4
	movl	NS532_FP_STATE_L6A(r1), f6
	movl	NS532_FP_STATE_L1A(r1), f1
	movl	NS532_FP_STATE_L3A(r1), f3
	movl	NS532_FP_STATE_L5A(r1), f5
	movl	NS532_FP_STATE_L7A(r1), f7
	lfsr	NS532_FP_STATE_FSR(r1)
	/* leave FPU enabled */
	ret	0
#endif NS32081

ENTRY(_fpload_ns081)
	bsr	EX(_enable_fpu)
	movd	S_ARG0, r1
	movl	NS532_FP_STATE_L0A(r1), f0
	movl	NS532_FP_STATE_L2A(r1), f2
	movl	NS532_FP_STATE_L4A(r1), f4
	movl	NS532_FP_STATE_L6A(r1), f6
	lfsr	NS532_FP_STATE_FSR(r1)
	/* leave FPU enabled */
	ret	0

/* enable fpu in cfg register */

ENTRY(_enable_fpu)
	sprd	cfg,r0
	orb	2,r0
	lprd	cfg,r0
	ret	0

/* disable fpu in cfg register */

ENTRY(_disable_fpu)
	sprd	cfg,r0
	bicb	2,r0
	lprd	cfg,r0
	ret	0

/* 
 * set page table; this automatically flushes TLB 
 * We need to load ptb1 because MOVUSi and MOVSUi do use it (weird).
 *                              (undocumented CPU "feature").
 */

ENTRY(_set_ptb)
	lmr	ptb0,S_ARG0
	lmr	ptb1,S_ARG0
	ret	0

/* get page table */

ENTRY(_get_ptb)
	smr	ptb0,r0
	ret	0

/* 
 * flush TLB 
 *
 * Flush TLB1 to make MOVSUi and MOVUSi work correctly.
 */

ENTRY(_flush_tlb)
	smr	ptb0,r0
	lmr	ptb0,r0
	lmr	ptb1,r0
	ret	0

/* set intbase register */

ENTRY(_set_intbase)
	lprd	intbase,S_ARG0
	ret	0

/* get intbase register */

ENTRY(_get_intbase)
	sprd	intbase,r0
	ret	0

/* set configuration register */

ENTRY(_set_cfg)
	lprd	cfg,S_ARG0
	ret	0

/* get configuration register */

ENTRY(_get_cfg)
	sprd	cfg,r0
	ret	0

/* set kernel stack pointer */

ENTRY(_set_ksp)
	lprd	sp,S_ARG0
	ret	0

/* get kernel stack pointer */

ENTRY(_get_ksp)
	sprd	sp,r0
	ret	0

/* set memory configuration register */

ENTRY(_set_mcr)
	lmr	mcr,S_ARG0
	ret	0

/* get memory configuration register */

ENTRY(_get_mcr)
	smr	mcr,r0
	ret	0

/* get memory status register */

ENTRY(_get_msr)
	smr	msr,r0
	ret	0

/* set memory status register */

ENTRY(_set_msr)
	lmr	msr,S_ARG0
	ret	0

/* get tear register value (faulting address in memory fault) */

ENTRY(_get_tear)
	smr	tear,r0
	ret	0

/* set tear register value (faulting address in memory fault) */

ENTRY(_set_tear)
	lmr	tear,S_ARG0
	ret	0

/* set debug configuration register */

ENTRY(_set_dcr)
	lprd	dcr,S_ARG0
	ret	0

/* get debug configuration register */

ENTRY(_get_dcr)
	sprd	dcr,r0
	ret	0

/* set debug status register */

ENTRY(_set_dsr)
	lprd	dsr,S_ARG0
	ret	0

/* get debug status register */

ENTRY(_get_dsr)
	sprd	dsr,r0
	ret	0

/* set compare address register */

ENTRY(_set_car)
	lprd	car,S_ARG0
	ret	0

/* get compare address register */

ENTRY(_get_car)
	sprd	car,r0
	ret	0

/* set breakpoint program counter register */

ENTRY(_set_bpc)
	lprd	bpc,S_ARG0
	ret	0

/* get breakpoint program counter register */

ENTRY(_get_bpc)
	sprd	bpc,r0
	ret	0

/* disable data cache.  NOTE: This is also called very early during boot,
   when we are still in physical addressing mode. */

ENTRY(_disable_data_cache)
	sprd	cfg,r0
	andd	0xfffffdff,r0   /* clear dc bit */
	lprd	cfg,r0
	ret	0

/* enable data cache */

ENTRY(_enable_data_cache)
	sprd	cfg,r0
	ord	0x00000200,r0	/* set dc bit */
	lprd	cfg,r0
	ret	0

/* 
 * invalidate tlb entries for the page
 * We invalidate ivar1 too to be sure that MOVUSi and MOVSUi work correctly.
 */

ENTRY(_invalidate_page)
	lmr	ivar0,S_ARG0
	lmr	ivar1,S_ARG0
	ret	0

/* copyin(uaddr,kaddr,count) */ 

ENTRY(copyin)
Entry(copyinmsg)
	FRAME
	movd	B_ARG0,r0		/* uaddr */
	movd	B_ARG2,r2		/* count */
	cmpd	r0, VM_MIN_ADDRESS	/* start address within bounds? */
	blo	copy_fail
	cmpd	r0, VM_MAX_ADDRESS
	bhs	copy_fail
	movd	r0, r1
	addd	r2, r1			/* user end address */
	cmpd	r1, VM_MAX_ADDRESS
	bhi	copy_fail
	movd	B_ARG1,r1		/* kaddr */
/* copyin_dwords: */
	andd	~3, r2			/* strip off low 2 bits */
	cmpd	r2,0	
	blo	copy_fail		/* negative -- illegal */
	ble	copyin_modulo_four	/* only 0-3 bytes */
copyin_dword_loop:
	RECOVER(copy_fail)
	movusd	0(r0),0(r1)
	addqd	4,r0
	addqd	4,r1
	acbd	-4,r2,copyin_dword_loop
copyin_modulo_four:
	movd	B_ARG2,r2
	andd	3,r2			/* count mod 4 */
	cmpd	r2,0
	ble	copy_done
copyin_modulo_four_loop:
	RECOVER(copy_fail)
	movusb	0(r0),0(r1)
	addqd	1,r0
	addqd	1,r1
	acbd	-1,r2,copyin_modulo_four_loop
	movd	B_ARG2,r2
copy_done:
	movqd	0,r0			/* SUCCESS */
	EMARF
	ret	0
copy_fail:
	movqd	1,r0			/* return 1 for failure */
	EMARF
	ret	0

/* copyout(kaddr,uaddr,count) */

ENTRY(copyout)
Entry(copyoutmsg)
	FRAME
	movd	B_ARG1,r0		/* uaddr */
	movd	B_ARG2,r2		/* count */
	cmpd	r0, VM_MIN_ADDRESS	/* start address within bounds? */
	blo	copy_fail
	cmpd	r0, VM_MAX_ADDRESS
	bhs	copy_fail
	movd	r0, r1
	addd	r2, r1			/* user end address */
	cmpd	r1, VM_MAX_ADDRESS
	bhi	copy_fail
	movd	B_ARG0,r1		/* kaddr */

/* copyout_dwords: */
	andd	~3, r2			/* strip off low 2 bits */
	cmpd	r2,0	
	blo	copy_fail		/* negative -- illegal */
	ble	copyout_modulo_four	/* only 0-3 bytes */
copyout_dword_loop:
	RECOVER(copy_fail)
	movsud	0(r1),0(r0)
	addqd	4,r0
	addqd	4,r1
	acbd	-4,r2,copyout_dword_loop
copyout_modulo_four:
	movd	B_ARG2,r2
	andd	3,r2			/* count mod 4 */
	cmpd	r2,0
	ble	copy_done
copyout_modulo_four_loop:
	RECOVER(copy_fail)
	movsub	0(r1),0(r0)
	addqd	1,r0
	addqd	1,r1
	acbd	-1,r2,copyout_modulo_four_loop
	movd	B_ARG2,r2
	br	copy_done

/*
 * Done with recovery table.
 */
	RECOVER_TABLE_END

/*
 * delay(N): waste N microseconds.
 * delay(0) must be at least 400 ns
 * delay(1) must be at least 1.2 us
 */
ENTRY(delay_us)			/* bsr  2 */
	cmpqd	0,S_ARG0	/*	2 */
	beq	delayout	/*	2 */
	movd	S_ARG0,r0	/* 	2 */
delayloop:
/*	mulw	r1, r1		/* 13 + 2*4 cycles; 840 ns @ 25MHz */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	acbd	-1,r0,delayloop /* 	5 cycles; 200 ns @ 25 MHz */
delayout:
	ret	0		/* 	4 */
/*
 * Waste 10 microseconds.
 */
ENTRY(tenmicrosec)
	FRAME
	movd	@EX(microdata),r0
tenmicroloop:
	acbd	-1,r0,tenmicroloop
	EMARF
	ret	0

/* This flushes the given address from the instruction cache. */

ENTRY(_flush_instruction_cache_addr)
	FRAME
	cinv	i,B_ARG0
	EMARF
	ret	0

/* Flush entire instruction cache */
ENTRY(ns532_flush_instruction_cache)
	FRAME
	cinv	ai,0
	EMARF
	ret	0

/* Flush entire data cache */
ENTRY(ns532_flush_data_cache)
	FRAME
	cinv	ad,0
	EMARF
	ret	0

/*************************** EOF ********************************/
