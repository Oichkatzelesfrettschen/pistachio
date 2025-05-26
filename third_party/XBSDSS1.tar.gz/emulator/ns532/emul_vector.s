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
 * $Log:$
 */
/*
 * 	File: ns532/emul_vector.s
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 */

#include <sys/syscall.h>
#define KERNEL
#include <sys/errno.h>
#undef KERNEL
#include <machine/psl.h>
#include <machine/asm.h>

/*
 * Emulator vector entry - allocate new stack
 */
	.data
	.globl	_emul_stack_lock
_emul_stack_lock:
	.long	0
	.text

/*
 * Both of these sequences must be 8 bytes long, and leave the stacked
 * PC pointing at the end of the sequence.
 */
/*
 * Generic emulator entry
 *
 * SP->		psr	
 *		pc to return to
 * system call number is in r0
 */

/*
 * Generic call
 */
	.globl	_emul_common
_emul_common:
	movd	r0,tos			/* save registers that C does not */
	movd	r1,tos
	movd	r2,tos
_emul_common_lock_loop:
        sbitib  0,@_emul_stack_lock     /* Move old bit to f, and set it to 1*/
        bfs     _emul_common_lock_loop  /* List locked try again... */
        movd    @_emul_stack_list,r0    /* grab an emulator stack */
        cmpqd   0,r0
        bne     _emul_common_havelists
        movd    0,@_emul_stack_lock     /* release lock */
        bsr     _emul_stack_alloc
        br      _emul_common_gotlist
_emul_common_havelists:
        movd    0(r0),@_emul_stack_list
        movd    0, @_emul_stack_lock    /* release lock */
_emul_common_gotlist:
        sprd    sp,0(r0)
        lprd    sp,r0
        movd    r3,tos
        movd    r4,tos
        movd    r5,tos
        movd    r6,tos
        movd    r7,tos
        sprd    fp,tos
        sprd    sb,tos
        lprd    sb,0                    /* the C-compiler may need this */
        sprd    sp,tos                  /* pointer to struct (above) */
        bsr     _emul_syscall
        adjspb  -4

/*
 * Return
 */
	.globl	emul_exit
emul_exit:
        lprd    sb,tos			/* restore registers */
        lprd    fp,tos
        movd    tos,r7
        movd    tos,r6
        movd    tos,r5
        movd    tos,r4
        movd    tos,r3
        sprd    sp,r2
        lprd    sp,0(sp)
_emul_exit_lock_loop:
        sbitib  0,@_emul_stack_lock     /* Move old bit to f, and set it to 1*/
        bfs     _emul_exit_lock_loop    /* List locked try again... */
        movd    @_emul_stack_list,r0
        movd    r0,0(r2)
        movd    r2,@_emul_stack_list
        movd    0,@_emul_stack_lock     /* Release lock */
        
        movd    tos,r2
        movd    tos,r1
        movd    tos,r0
        lprd    us, tos                 /* load psr */
        ret     0

/*
 * Child starts executing here with return values in r0/r1,
 * stack pointing to saved regs.
 */
	.globl	_child_fork
_child_fork:
        movd    r0,8(sp)
        movd    r1,4(sp)
        bsr     _child_init
        movd    tos,r2
        movd    tos,r1
        movd    tos,r0
        lprd    us, tos 
        bicpsrb PSR_C           /* clear carry: success */
        ret     0       

/*
 * The easiest way to exec /etc/init is to take
 * an emulator trap (it sets up the registers nicely).
 */
	.globl	_emul_execve
_emul_execve:
        movd    SYS_execve,r0           /* exec system call number */
        movd    emul_exec_error,tos     /* push error return address */
        sprd    us,tos                  
        br      _emul_common            /* take a system call 'trap' */

	/* we only get here on failure */
emul_exec_error:
	movd	-1,r0			/* failure return */
	ret	0

/* Get floating point registers
        in = int[9]
        out = fsr, l0a, l0b ....

        We do not support long floats regs 1,3,5,7
*/

ENTRY(get_fp_regs)
        FRAME
        movd    B_ARG0,r0
        sfsr    0(r0)
        movl    f0,4(r0)
        movl    f2,12(r0)
        movl    f4,20(r0)
        movl    f6,28(r0)
        EMARF
        ret     0

/* Get mod register */

ENTRY(get_mod_reg)
        FRAME
        sprd    mod,r0
        EMARF
        ret     0
