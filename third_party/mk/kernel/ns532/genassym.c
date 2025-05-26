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
 * $Log: genassym.c,v $
 */
/*
 * 	File: ns532/genassym.c
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 *	Generate constants for assembler files.
 */

#include <mach_kdb.h>

/*
 * Pass field offsets to assembly code.
 */
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/syscall_emulation.h>
#include <ns532/thread.h>
#include <mach/ns532/vm_param.h>
#include <ns532/pmap.h>
#include <ns532/trap.h>
#include <ns532/mmu.h>
#include <ns532/pic.h>
#if	MACH_KDB
#include <sys/reboot.h>
#endif	MACH_KDB

#include <mach/ns532/thread_status.h>
#include <ns532/fp_reg.h>

/*ARGSUSED*/
main(argc,argv)
	int	argc;
	char	**argv;
{
	struct ns532_kernel_state	*ks = (struct ns532_kernel_state *)0;
	struct ns532_kernel_state	*kss = STACK_IKS(0);
	struct ns532_saved_state	*us = (struct ns532_saved_state *)0;
	thread_t			th = (thread_t)0;
	task_t				task = (task_t)0;
	eml_dispatch_t			eml = (eml_dispatch_t)0;
	struct ns532_thread_state	*ts = (struct ns532_thread_state *)0;
	struct ns532_float_state	*fs = (struct ns532_float_state *)0;
	struct ns532_fp_state		*fps = (struct ns532_fp_state *)0;

	printf("#define TH_KERNEL_STACK\t%d\n", &th->kernel_stack);
	printf("#define TH_SWAP_FUNC\t%d\n", &th->swap_func);

	printf("#define	TH_PCB\t%d\n", &th->pcb);
	printf("#define	TH_TASK\t%d\n", &th->task);
	printf("#define	TH_RECOVER\t%d\n", &th->recover);

	printf("#define	TASK_EMUL\t%d\n", &task->eml_dispatch);
	printf("#define TASK_MAP\t%d\n", &task->map);

	printf("#define DISP_MIN\t%d\n", &eml->disp_min);
	printf("#define	DISP_COUNT\t%d\n", &eml->disp_count);
	printf("#define	DISP_VECTOR\t%d\n", &eml->disp_vector[0]);
	
	printf("#define K_SIZE\t%d\n", sizeof *ks);

	printf("#define	KSS_R3\t%d\n", &kss->k_r3);
	printf("#define	KSS_R4\t%d\n", &kss->k_r4);
	printf("#define	KSS_R5\t%d\n", &kss->k_r5);
	printf("#define	KSS_R6\t%d\n", &kss->k_r6);
	printf("#define	KSS_R7\t%d\n", &kss->k_r7);
	printf("#define	KSS_SP\t%d\n", &kss->k_sp);
	printf("#define KSS_FP\t%d\n", &kss->k_fp);
	printf("#define KSS_PC\t%d\n", &kss->k_pc);

#ifdef MK42
	printf("#define	KSS_IPL\t%d\n", &kss->k_ipl);
#endif MK42

	printf("#define R_SIZE\t%d\n", sizeof *us);
	printf("#define	R_USP\t%d\n", &us->usp);
	printf("#define	R_R0\t%d\n", &us->r0);
	printf("#define	R_TRAPNO\t%d\n", &us->trapno);
	printf("#define	R_PC\t%d\n", &us->pc);
	printf("#define R_PSR\t%d\n", &us->psr);

	printf("#define	NBPG\t%d\n", NS532_PGBYTES);
	printf("#define PGSHIFT\t%d\n", NS532_PGSHIFT);
	printf("#define	VM_MIN_ADDRESS\t%d\n", VM_MIN_ADDRESS);
	printf("#define	VM_MAX_ADDRESS\t%d\n", VM_MAX_ADDRESS);
	printf("#define	KERNELBASE\t%d\n", VM_MIN_KERNEL_ADDRESS);
	printf("#define KERNEL_STACK_SIZE\t%d\n", KERNEL_STACK_SIZE);
	
	printf("#define	PDESHIFT\t%d\n", PDESHIFT);
	printf("#define	PTESHIFT\t%d\n", PTESHIFT);
	printf("#define	PTEMASK\t%d\n", PTEMASK);

	printf("#define	PTES_PER_PAGE\t%d\n", NPTES);
	printf("#define	NS532_PTE_KERNEL\t%d\n",
	       NS532_PTE_VALID|NS532_PTE_WRITE);

	printf("#define MCR_VALUE\t%d\n", MCR_VALUE);

#if	MACH_KDB
	printf("#define	RB_KDB\t%d\n", RB_KDB);
#endif	MACH_KDB

	printf("#define NS532_THREAD_STATE_R0\t%d\n", &ts->r0);
	printf("#define NS532_THREAD_STATE_R1\t%d\n", &ts->r1);
	printf("#define NS532_THREAD_STATE_R2\t%d\n", &ts->r2);
	printf("#define NS532_THREAD_STATE_R3\t%d\n", &ts->r3);
	printf("#define NS532_THREAD_STATE_R4\t%d\n", &ts->r4);
	printf("#define NS532_THREAD_STATE_R5\t%d\n", &ts->r5);
	printf("#define NS532_THREAD_STATE_R6\t%d\n", &ts->r6);
	printf("#define NS532_THREAD_STATE_R7\t%d\n", &ts->r7);
	printf("#define NS532_THREAD_STATE_SB\t%d\n", &ts->sb);
	printf("#define NS532_THREAD_STATE_FP\t%d\n", &ts->fp);
	printf("#define NS532_THREAD_STATE_SP\t%d\n", &ts->sp);
	printf("#define NS532_THREAD_STATE_PC\t%d\n", &ts->pc);
	printf("#define NS532_THREAD_STATE_MOD\t%d\n", &ts->mod);
	printf("#define NS532_THREAD_STATE_PSR\t%d\n", &ts->psr);

	printf("#define NS532_FLOAT_STATE_FP_KIND\t%d\n", &fs->fp_kind);
	printf("#define NS532_FLOAT_STATE_VALID\t%d\n", &fs->valid);
	printf("#define NS532_FLOAT_STATE_FSR\t%d\n", &fs->fsr);
	printf("#define NS532_FLOAT_STATE_L0A\t%d\n", &fs->l0a);
	printf("#define NS532_FLOAT_STATE_L0B\t%d\n", &fs->l0b);
	printf("#define NS532_FLOAT_STATE_L1A\t%d\n", &fs->l1a);
	printf("#define NS532_FLOAT_STATE_L1B\t%d\n", &fs->l1b);
	printf("#define NS532_FLOAT_STATE_L2A\t%d\n", &fs->l2a);
	printf("#define NS532_FLOAT_STATE_L2B\t%d\n", &fs->l2b);
	printf("#define NS532_FLOAT_STATE_L3A\t%d\n", &fs->l3a);
	printf("#define NS532_FLOAT_STATE_L3B\t%d\n", &fs->l3b);
	printf("#define NS532_FLOAT_STATE_L4A\t%d\n", &fs->l4a);
	printf("#define NS532_FLOAT_STATE_L4B\t%d\n", &fs->l4b);
	printf("#define NS532_FLOAT_STATE_L5A\t%d\n", &fs->l5a);
	printf("#define NS532_FLOAT_STATE_L5B\t%d\n", &fs->l5b);
	printf("#define NS532_FLOAT_STATE_L6A\t%d\n", &fs->l6a);
	printf("#define NS532_FLOAT_STATE_L6B\t%d\n", &fs->l6b);
	printf("#define NS532_FLOAT_STATE_L7A\t%d\n", &fs->l7a);
	printf("#define NS532_FLOAT_STATE_L7B\t%d\n", &fs->l7b);

	printf("#define NS532_FP_STATE_FP_KIND\t%d\n", &fps->fp_kind);
	printf("#define NS532_FP_STATE_VALID\t%d\n", &fps->valid);
	printf("#define NS532_FP_STATE_FSR\t%d\n", &fps->fsr);
	printf("#define NS532_FP_STATE_L0A\t%d\n", &fps->l0a);
	printf("#define NS532_FP_STATE_L1A\t%d\n", &fps->l1a);
	printf("#define NS532_FP_STATE_L2A\t%d\n", &fps->l2a);
	printf("#define NS532_FP_STATE_L3A\t%d\n", &fps->l3a);
	printf("#define NS532_FP_STATE_L4A\t%d\n", &fps->l4a);
	printf("#define NS532_FP_STATE_L5A\t%d\n", &fps->l5a);
	printf("#define NS532_FP_STATE_L6A\t%d\n", &fps->l6a);
	printf("#define NS532_FP_STATE_L7A\t%d\n", &fps->l7a);

	return (0);
}

