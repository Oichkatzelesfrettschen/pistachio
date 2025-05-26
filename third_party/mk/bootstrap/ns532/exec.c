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
 * 28-May-93  Johannes Helander (jvh) at Helsinki University of Technology
 *	Include mach.h instead of mach/mach.h.
 *
 * 11-May-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Created from i386/exec.c.
 *
 * $Log:$
 */

#include <mach.h>

#include "file_io.h"
#include "loader_info.h"

#include <ns532/exec.h>
#include <ns532/psl.h>

/*
 *	Machine-dependent portions of execve() for the ns532.
 */

int ex_get_header(fp, lp)
	struct file *fp;
	register struct loader_info *lp;
{
	struct exec	x;
	register int	result;
	vm_size_t	resid;

	result = read_file(fp, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result)
		return (result);
	if (resid)
		return (EX_NOT_EXECUTABLE);

	switch ((int)x.a_magic) {

	    case 0407:
		lp->text_start  = 0;
		lp->text_size   = 0;
		lp->text_offset = 0;
		lp->data_start  = 0x10000;
		lp->data_size   = x.a_text + x.a_data;
		lp->data_offset = sizeof(struct exec);
		lp->bss_size    = x.a_bss;
		break;

	    case 0410:
		if (x.a_text == 0) {
			return(EX_NOT_EXECUTABLE);
		}
		lp->text_start  = 0x10000;
		lp->text_size   = x.a_text;
		lp->text_offset = sizeof(struct exec);
		lp->data_start  = lp->text_start + lp->text_size;
		lp->data_size   = x.a_data;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a_bss;
		break;

	    case 0413:
		if (x.a_text == 0) {
			return(EX_NOT_EXECUTABLE);
		}
		lp->text_start  = 0x10000;
		lp->text_size   = sizeof(struct exec) + x.a_text;
		lp->text_offset = 0;
		lp->data_start  = lp->text_start + lp->text_size;
		lp->data_size   = x.a_data;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a_bss;
		break;
	    default:
		return (EX_NOT_EXECUTABLE);
	}
	lp->entry_1 = x.a_entry;
	lp->entry_2 = 0;

	return(0);
}

#define	STACK_SIZE	(64*1024)

char *set_regs(user_task, user_thread, lp, arg_size)
	mach_port_t	user_task;
	mach_port_t	user_thread;
	struct loader_info *lp;
	int		arg_size;
{
	vm_offset_t	stack_start;
	vm_offset_t	stack_end;
	struct ns532_thread_state	regs;
	unsigned int		reg_size;

	/*
	 * Add space for 5 ints to arguments, for
	 * PS program. XXX (from i386).
	 */
	arg_size += 5 * sizeof(int);

	/*
	 * Allocate stack.
	 */
	stack_end = VM_MAX_ADDRESS;
	stack_start = VM_MAX_ADDRESS - STACK_SIZE;
	(void)vm_allocate(user_task,
			  &stack_start,
			  (vm_size_t)(stack_end - stack_start),
			  FALSE);

	reg_size = NS532_THREAD_STATE_COUNT;
	(void)thread_get_state(user_thread,
				NS532_THREAD_STATE,
				(thread_state_t)&regs,
				&reg_size);

	regs.pc = lp->entry_1;
	regs.sp = (int)((stack_end - arg_size) & ~(sizeof(int)-1));
	regs.psr = PSR_U;
	regs.sb = 0;
	regs.fp = 0;

	/* FPU state now initialized in kernel for new threads //jvh 910511 */

	(void)thread_set_state(user_thread,
				NS532_THREAD_STATE,
				(thread_state_t)&regs,
				reg_size);
	return ((char *)regs.sp);
}

boolean_t
get_symtab(fp, symoff_p, symsize_p, header, header_size)
	struct file	*fp;
	vm_offset_t	*symoff_p;	/* out */
	vm_size_t	*symsize_p;	/* out */
	char		header[];	/* out array */
	vm_size_t	*header_size;	/* out */
{
	register int		result;
	vm_offset_t		resid;
	vm_offset_t		sym_off;
	vm_offset_t		str_off;
	vm_size_t		sym_size;
	vm_size_t		str_size;
	struct exec		x;

	result = read_file(fp, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result || resid)
	    return (FALSE);

	sym_off = sizeof(struct exec)
		  + x.a_text + x.a_data + x.a_trsize + x.a_drsize;
	sym_size = x.a_syms;
	str_off  = sym_off + sym_size;
	result = read_file(fp, str_off,
			&str_size, sizeof(int), &resid);
	if (result || resid)
	    return (FALSE);
	/*
	 * Return the entire symbol table + string table.
	 * Add a header:
	 *   size of symbol table
	 */

	*symoff_p = sym_off;
	*symsize_p = sym_size + str_size;
	*(int *)header = sym_size;
	*header_size = sizeof(int);

	return (TRUE);
}
