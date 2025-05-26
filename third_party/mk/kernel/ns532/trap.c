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
 * 17-Apr-93  Ian Dall (ian) at University of Adelaide
 *	Added interrupted_pc().
 *
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created.
 *
 * $Log: trap.c,v $
 * Revision 1.35  1992/05/11  02:51:39  jvh
 * 	Purged history.
 *
 */
/*
 * 	File: ns532/trap.c
 *	Author: Johannes Helander, Tatu Ylonen, Tero Kivinen
 *	Helsinki University of Technology 1992.
 *
 *	Hardware trap handler.
 */

#include <cpus.h>

#include <mach_kdb.h>
#include <mach_pcsample.h>

#include <sys/types.h>
#include <ns532/psl.h>
#include <ns532/trap.h>
#include <ns532/mmu.h>
#include <ns532/inst.h>
#include <ns532/machparam.h>

#include <mach/exception.h>
#include <mach/kern_return.h>
#include <mach/vm_param.h>

#include <vm/vm_kern.h>
#include <vm/vm_map.h>

#include <kern/thread.h>
#include <kern/task.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>

extern void exception();
extern void thread_exception_return();

#if	MACH_KDB
boolean_t	debug_all_traps_with_kdb = FALSE;
boolean_t	debug_user_page_faults = FALSE;
boolean_t	debug_user_unrecoverable_page_fault = FALSE;
boolean_t	debug_show_user_unrecoverable_page_fault = TRUE;
boolean_t	debug_show_sbiti = FALSE;
boolean_t	debug_sbiti = FALSE;
boolean_t	debug_show_movus_fault = FALSE;
boolean_t	debug_user_overflow = FALSE;
boolean_t	debug_ignore_overflow = TRUE;
boolean_t	debug_on_user_bpt = FALSE;

extern struct db_watchpoint *db_watchpoint_list;
extern boolean_t db_watchpoints_inserted;
#endif

void
user_page_fault_continue(kr)
      kern_return_t kr;
{
	register thread_t thread;
	register struct ns532_saved_state *regs;
	register int msr, ss;


	ss = splsched();
	thread = current_thread();
	regs = USER_REGS(thread);
	msr = regs->msr;

#if	MACH_KDB
	if(!(regs->tear >= VM_MIN_ADDRESS
	     && regs->tear < VM_MAX_ADDRESS
	     && regs->pc >= VM_MIN_ADDRESS
	     && regs->pc < VM_MAX_ADDRESS)) {
printf("user_pfc: tear=%x pc=%x regs=%x pcb=%x th=%x kr=%x st=%x\n",
		       regs->tear, regs->pc, regs, thread->pcb, 
		       thread, kr, thread->kernel_stack);
		assert(FALSE);
	}
#endif	MACH_KDB

	if (kr == KERN_SUCCESS) {
		/* Check for CBITIi and SBITIi instructions. (32532 bug) */
		if((msr & MSR_DDT) == MSR_DDT_READ
		   && (msr & MSR_STT) == MSR_STT_DT
		   && IS_ILOCK1(regs->pc)) {
			char offending_instruction[2];
			if (copyin(regs->pc, offending_instruction, 2)
			    == KERN_SUCCESS) {
				if (IS_ILOCK2(offending_instruction)) {
					msr &= ~MSR_STT;
					regs->msr = msr | MSR_STT_RMW;
#if					MACH_KDB
					if(debug_show_sbiti) {
printf("user_pf_c: SBITIi tear=%x msr=%x pc=%x th=%x\n",
       regs->tear, msr, regs->pc, thread);
						assert(!debug_sbiti);
					}
#endif					MACH_KDB
					splx(ss);
					(void) vm_fault(thread->task->map,
						   trunc_page(
						     (vm_offset_t) regs->tear),
						   VM_PROT_READ|VM_PROT_WRITE,
						   FALSE,
						   FALSE,
						   user_page_fault_continue);
					/*NOTREACHED*/
				}
			}
		}
		splx(ss);
#if		MACH_KDB
		if (db_watchpoint_list
		    && db_watchpoints_inserted
		    && ((msr & MSR_DDT) == MSR_DDT_WRITE
			|| (msr & MSR_STT) == MSR_STT_RMW)
		    && db_find_watchpoint(thread->task->map,
					  (vm_offset_t)regs->tear,
					  regs))
		    kdb_trap(T_WATCHPOINT, 0, regs);
#endif		MACH_KDB
		thread_exception_return();
		/*NOTREACHED*/
	}
	splx(ss);
	
#if	MACH_KDB
	if (debug_all_traps_with_kdb && kdb_trap(regs->trapno, msr, regs)) {
		thread_exception_return();
		/*NOTREACHED*/
	}
	
	if (debug_show_user_unrecoverable_page_fault)
	{
		printf("user_page_fault_continu: kr=%x pc=%x tear=%x msr=%x\n",
		       kr, regs->pc, regs->tear, msr);
	}
	if (debug_user_unrecoverable_page_fault)
	    kdb_trap(regs->trapno, msr, regs);
#endif	MACH_KDB
	
	exception(EXC_BAD_ACCESS, kr, regs->tear);
	/*NOTREACHED*/
}

#if MACH_KDB
void
thread_kdb_return()
{
	register thread_t thread = current_thread();
	register struct ns532_saved_state *regs = USER_REGS(thread);
	
	if (kdb_trap(regs->trapno, /* regs->err */ 0 , regs)) {
		thread_exception_return();
		/*NOTREACHED*/
	}
}
#endif MACH_KDB

/*
 * Fault recovery in copyin/copyout routines.
 */
struct recovery {
	int	fault_addr;
	int	recover_addr;
};

extern struct recovery	recover_table[];
extern struct recovery	recover_table_end[];

char *	trap_type[] = {
	"Non-vectored interrupt",
	"NMI",
	"Abort",
	"Coprocessor",
	"Illegal operation",
	"Supervisor call",
	"Divide by zero",
	"Flag instruction",
	"Breakpoint",
	"Trace",
	"Undefined instruction",
	"Restartable bus error",
	"Non-restartable bus error",
	"Integer overflow",
	"Debug"
};
int	TRAP_TYPES = sizeof(trap_type)/sizeof(trap_type[0]);

int trapcount = 0, prevsubcode = 0;
thread_t prevthread = NULL;

/*
 * Trap handler.  Called from locore when a processor trap occurs.
 */

void trap(regs)
	register struct ns532_saved_state *regs;
{
	int 			exc;
	int 			subcode;
	int 			code;
	register int 		type;
	vm_map_t 		map;
	kern_return_t 	result;
	register thread_t 	thread;
	struct recovery 	*rp;
	
	type = regs->trapno;
	thread = current_thread();
	code = 0;
	
	if ((regs->psr & PSR_U) == 0) { /* trap in system mode. */
		switch (type) {
		      case T_NMI: 	/* nmi trap */
			printf("NMI in system mode\n");
			panic("memory parity error in system mode");
			/*NOTREACHED*/
		      case T_BPT:
		      case T_TRC:
		      case T_DBG:
#if			MACH_KDB
			kdb_trap(type,code,regs);
			return;
#else			MACH_KDB
		      panic("unexpected trace/breakpoint trap in system mode");
#endif			MACH_KDB
			/*NOTREACHED*/
		      case T_ABT: /* page fault */
			/*
			 * If the current map is a submap of the kernel map,
			 * and the address is within that map, fault on that
			 * map.  If the same check is done in vm_fault
			 * (vm_map_lookup), we may deadlock on the kernel map
			 * lock.
			 */
			subcode = regs->tear;
			if (thread == THREAD_NULL) {
				map = kernel_map;
			} else {
				map = thread->task->map;
				if ((vm_offset_t)subcode < vm_map_min(map) ||
				    (vm_offset_t)subcode >= vm_map_max(map))
				    map = kernel_map;
			}
			code = regs->msr;
			
			/* 
			 * Do not allow MOVUSi/MOVSUi to change
			 * mappings -- fail the page fault in this
			 * case instead. The proper action would be to
			 * fail the system call that caused the
			 * situation.
			 */
			if((code & MSR_USR) && (map == kernel_map)) {
				result = KERN_INVALID_ADDRESS;
#if				MACH_KDB
				if (debug_show_movus_fault)
printf("trap: MOVSUi/MOVUSi faulted on kernel map\n");
#endif				MACH_KDB
			} else {
				result = vm_fault(map,
					   trunc_page((vm_offset_t) subcode),
					   ((code & MSR_DDT) == MSR_DDT_WRITE
					    || (code & MSR_STT) == MSR_STT_RMW)
						  ? VM_PROT_READ|VM_PROT_WRITE
						  : VM_PROT_READ,
					    FALSE,
					    FALSE,
					    (void (*)()) 0);
			}
			/* 
			 * Check for SBITIi and CBITIi instructions in
			 * ns532. See cpu bug list.
			 */
			if (result == KERN_SUCCESS &&
			    (code & MSR_DDT) == MSR_DDT_READ &&
			    (code & MSR_STT) == MSR_STT_DT &&
			    prevsubcode == subcode && prevthread == thread)
			{
				trapcount++;
				if (trapcount > MAXTRAPCOUNT) {
					if (IS_ILOCK(regs->pc)) {
						result = vm_fault(map,
								  trunc_page((vm_offset_t) subcode),
								  VM_PROT_READ|VM_PROT_WRITE,
								  FALSE,
								  FALSE,
								  (void (*)()) 0);
						trapcount = 0;
					} else {
						if (trapcount > PANICTRAPCOUNT) {
							printf("trap.c: trap: page_fault loop. Yet another cpu bug?\n");
							printf("trap.c: trap: inst %08x: %08x %08x %08x %08x\n",
							       regs->pc, *((unsigned int *) regs->pc),
							       *(((unsigned int *) regs->pc)+1),
							       *(((unsigned int *) regs->pc)+2),
							       *(((unsigned int *) regs->pc)+3));
							printf("trap.c: trap: data %08x: %08x %08x %08x %08x\n",
							       subcode, *((unsigned int *) subcode),
							       *(((unsigned int *) subcode)+1),
							       *(((unsigned int *) subcode)+2),
							       *(((unsigned int *) subcode)+3));
							panic("trap.c: trap: kernel mode page_fault loop.");
						}
					}
				}
			} else {
				trapcount=0;
				prevsubcode=subcode;
				prevthread=thread;
			}
			if (result == KERN_SUCCESS)
			    return;
			
			for (rp = recover_table;
			     rp < recover_table_end;
			     rp++)
			{
				if (regs->pc == rp->fault_addr) {
					regs->pc = rp->recover_addr;
					return;
				}
			}
			/* Unanticipated page-fault errors in kernel
			   should not happen. */
			printf("trap: Kernel page fault at 0x%x, pc=0x%x\n",
			       subcode,regs->pc);
			/* fall through */
			
		      default:
printf("trap type 0x%x (%s), code = 0x%x, subcode=0x%x, pc = 0x%x\n",
			       type,
			       (type >= 0 && type < TRAP_TYPES)?
			       trap_type[type]:"unknown",
			       code, subcode, regs->pc);
#if			MACH_KDB
			kdb_trap(type, code, regs);
#endif			MACH_KDB
			panic("trap");
			/*NOTREACHED*/
		}
	}
	
	/*
	 *	Trap from user mode.
	 */
#if		MACH_KDB
	if (debug_on_user_bpt)
	  /* temporary, for debugging the unix server
	   * Otherwise these traps get handled below
	   */
	  if (type == T_BPT || type == T_TRC || type == T_DBG) {
	    kdb_trap(type,code,regs);
	    return;
	  }
#endif		MACH_KDB
	
	if ((type != T_ABT) && (type != T_UND))
	    printf("trap: trap in user mode: %d %s, pc=0x%x\n",
		   type,trap_type[type],regs->pc);
	
	switch (type) {
	      case T_NVI:
		panic("non-vectored interrupt trap being handled in trap()");
		/*NOTREACHED*/
	      case T_NMI:
		printf("NMI trap in user mode\n");
		panic("memory parity error in user mode");
		/*NOTREACHED*/
	      case T_SLAVE:
		printf("trap.c: T_SLAVE being processed\n");
		fpintr();
		return;
	      case T_ILL:
		{
			char offending_instruction[3];
			if (copyin(regs->pc, offending_instruction, 3) 
			    == KERN_SUCCESS)
			{
				if (IS_CINV(offending_instruction)) {
					if (IS_CINV_DATA(offending_instruction)) {
						ns532_flush_instruction_cache();
					}
					if (IS_CINV_INSTR(offending_instruction)) {
						ns532_flush_data_cache();
					}
					return;
				}
			}
			exc=EXC_BAD_INSTRUCTION;
			code=EXC_NS532_ILL;
			break;
		}
	      case T_SVC:
		panic("supervisor call being handled in trap()");
		/*NOTREACHED*/
	      case T_DVZ:
		exc=EXC_ARITHMETIC;
		code=EXC_NS532_DVZ;
		break;
	      case T_FLG:
		exc=EXC_SOFTWARE;
		code=EXC_NS532_FLG;
		break;
	      case T_BPT:
		exc=EXC_BREAKPOINT;
		code=EXC_NS532_BPT;
		break;
	      case T_TRC:
		exc=EXC_BREAKPOINT;
		code=EXC_NS532_TRC;
		break;
	      case T_UND:
		{
			char offending_instruction[1];
			if (copyin(regs->pc, offending_instruction, 1)
			    == KERN_SUCCESS) {
				if (IS_FLOAT_INST(offending_instruction)) {
					if (fp_disabled_trap() == KERN_SUCCESS)
					    return;
				}
			}
			printf("trap: UND trap in user mode: %d %s, pc=0x%x\n",
			       type,trap_type[type],regs->pc);
			exc=EXC_BAD_INSTRUCTION;
			code=EXC_NS532_UND;
			break;
		}
	      case T_RBE:
		panic("restartable bus error being handled in trap()");
		/*NOTREACHED*/
	      case T_NBE:
		panic("non-restartable bus error being handled in trap()");
		/*NOTREACHED*/
	      case T_OVF:
		exc=EXC_ARITHMETIC;
		code=EXC_NS532_OVF;
#if		MACH_KDB
		if (debug_user_overflow) {
printf("trap: user mode interger overflow trap, addr=0x%x type=0x%x\n",
			       subcode, code);
			(void) kdb_trap(type, code, regs);
		}
		if (debug_ignore_overflow)
		  return;
#endif		MACH_KDB
		break;
	      case T_DBG:
		exc=EXC_BREAKPOINT;
		code=EXC_NS532_DBG;
		break;
	      case T_ABT:
		subcode = regs->tear;
		code = regs->msr;
		
#if		MACH_KDB
		if (debug_user_page_faults) {
printf("trap: user mode page fault, addr=0x%x type=0x%x msr=0x%x\n",
			       subcode, code, subcode);
			(void) kdb_trap(type, code, regs);
		}
#endif		MACH_KDB
		/* This check was missing in the i386 version */
		if(subcode >= VM_MAX_ADDRESS || subcode < VM_MIN_ADDRESS) {
			exc = EXC_BAD_ACCESS;
			code = KERN_INVALID_ADDRESS;
#if			MACH_KDB
			if(debug_show_user_unrecoverable_page_fault)
printf("trap: user mode bad access: tear=%x msr=%x pc=%x\n",
				   subcode, code, regs->pc);
			if (debug_user_unrecoverable_page_fault)
			    kdb_trap(type, code, regs);
#endif			MACH_KDB
			break;
		}
		(void) vm_fault(thread->task->map,
				trunc_page((vm_offset_t)subcode),
				((code & MSR_DDT) == MSR_DDT_WRITE || 
				 (code & MSR_STT) == MSR_STT_RMW)
				? VM_PROT_READ|VM_PROT_WRITE
				: VM_PROT_READ,
				FALSE,
				FALSE, user_page_fault_continue);
		/*NOTREACHED*/
		break;
	}
	
#if	MACH_KDB
	if (debug_all_traps_with_kdb && 
	    kdb_trap(type, code, regs))
	    return;
#endif	MACH_KDB
	exception(exc, code, subcode);
	/*NOTREACHED*/
}

#if	MACH_PCSAMPLE
/*
 * return saved state for interrupted user thread
 */
unsigned
interrupted_pc(thread_t t)
{
	struct ns532_saved_state *regs;
	regs = USER_REGS(t);

 	return regs->pc;
}
#endif	/*MACH_PCSAMPLE*/
