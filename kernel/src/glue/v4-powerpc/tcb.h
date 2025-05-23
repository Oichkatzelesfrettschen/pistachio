/*********************************************************************
 *                
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *                
 * File path:     glue/v4-powerpc/tcb.h
 * Description:   
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id$
 *                
 ********************************************************************/
#pragma once

#include INC_ARCH(ppc_registers.h)
#include INC_ARCH(msr.h)
#include INC_API(syscalls.h)
#include INC_API(ipc.h)
#include INC_GLUE(resource_functions.h)

#define TRACE_TCB(x...)
//#define TRACE_TCB(x...)	TRACEF(x)

/********************************************************************** 
 *
 *                      processor state
 *
 **********************************************************************/

/* NOTE: The size must be a multiple of 8-bytes.
 * Important: the ordering here is rather important.  It matches hard coded
 * offsets in the inlined assembler thread switch code, offsets in the
 * threadswitch fast path, and offsets in the notify inlined assembler.
 */
typedef struct {
    word_t ip;
    word_t unused;
    word_t r31;
    word_t r30;
} tswitch_frame_t;

/* NOTE: The size must be a multiple of 8-bytes.
 * Important: ordering for back_chain and lr_save is important, as they
 * must sit on the stack in positions defined by the eabi.
 */
typedef struct {
    word_t back_chain;	// eabi prior stack location
    word_t lr_save;	// eabi return address
    word_t unused;
    word_t arg2;
    word_t arg1;
    void (*func)( word_t, word_t );
} notify_frame_t;


INLINE addr_t get_kthread_ip( tcb_t *tcb )
{
    tswitch_frame_t *tswitch_frame = (tswitch_frame_t *)tcb->stack;
    return (addr_t)tswitch_frame->ip;
}

INLINE syscall_regs_t *get_user_syscall_regs( tcb_t *tcb )
{
    return (syscall_regs_t *)
	(word_t(tcb->get_stack_top()) - sizeof(syscall_regs_t));
}

INLINE except_regs_t *get_user_except_regs( tcb_t *tcb )
{
    return (except_regs_t *)
	(word_t(tcb->get_stack_top()) - sizeof(except_regs_t));
}

/********************************************************************** 
 *
 *                      tcb methods
 *
 **********************************************************************/

INLINE void tcb_t::set_utcb_location( word_t utcb_location )
{
    utcb_t *dummy = (utcb_t *)nullptr;
    myself_local.set_raw( utcb_location + (word_t)dummy->mr );
}

INLINE word_t tcb_t::get_utcb_location()
{
    utcb_t *dummy = (utcb_t *)nullptr;
    return myself_local.get_raw() - (word_t)dummy->mr;
}

INLINE void tcb_t::set_cpu( cpuid_t cpu )
{
    this->cpu = cpu;
    get_utcb()->processor_no = cpu;
#if defined(CONFIG_PPC_MMU_TLB)
    if (get_space() != get_kernel_space())
	this->pdir_cache = (word_t)space->get_asid(cpu);
#endif
}

/**
 * tcb_t::get_mr: returns value of message register
 * @index: number of message register
 */
INLINE word_t tcb_t::get_mr(word_t index)
{
    return get_utcb()->mr[index];
}

/**
 * tcb_t::set_mr: sets the value of a message register
 * @index: number of message register
 * @value: value to set
 */
INLINE void tcb_t::set_mr(word_t index, word_t value)
{
    get_utcb()->mr[index] = value;
}

/**
 * copies a set of message registers from one UTCB to another
 * @param dest destination TCB
 * @param start MR start index
 * @param count number of MRs to be copied
 */
INLINE void tcb_t::copy_mrs(tcb_t * dest, word_t start, word_t count)
{
    ASSERT(start + count <= IPC_NUM_MR);
    ASSERT(count > 0);

    asm volatile (
	    "mtctr	%0 ;"	/* Initialize the count register. */
	    "1:"
	    "lwzu	%0, 4 (%1) ;"	/* Load from src utcb. */
	    "stwu	%0, 4 (%2) ;"	/* Store to dest utcb. */
	    "bdnz	1b ;"	/* Decrement ctr and branch if not zero. */
	    : /* outputs */
	      "+r" (count)
	    : /* inputs */
	      /* Handle pre-increment with -1 offset. */
	      "r" (&this->get_utcb()->mr[start-1]), 
	      "r" (&dest->get_utcb()->mr[start-1])
	    : /* clobbers */
	      "ctr"
	    );
}

/**
 * tcb_t::get_br: returns value of buffer register
 * @index: number of buffer register
 */
INLINE word_t tcb_t::get_br(word_t index)
{
    return get_utcb()->br[32-index];
}

/**
 * tcb_t::set_br: sets the value of a buffer register
 * @index: number of buffer register
 * @value: value to set
 */
INLINE void tcb_t::set_br(word_t index, word_t value)
{
    get_utcb()->br[32-index] = value;
}

#ifdef CONFIG_DYNAMIC_TCBS
INLINE void tcb_t::allocate()
{
    // Write to the tcb, to ensure that the kernel maps this tcb
    // with write access.  Write to the bottom of the stack.
    // TODO: should we do this?  It wastes a cache line.
    *(word_t *)( (word_t)this + sizeof(tcb_t) ) = 0;
}
#endif


INLINE void tcb_t::set_space(space_t * space)
{
    this->space = space;

    if (!space)
	return;

    if( EXPECT_FALSE(space == get_kernel_space()) )
    {
	/* Thread switch expects pdir_cache to be 0 for kernel threads.
	 */
	this->pdir_cache = 0;
	this->resources.set_kernel_thread( this );
	return;
    }

#ifdef CONFIG_PPC_MMU_SEGMENT
    this->pdir_cache = (word_t)space->get_segment_id().raw;
    TRACE_TCB("set_space(), space 0x%p, tcb 0x%p, kernel_space 0x%p\n", 
	      space, this, get_kernel_space() );

    space->sync_kernel_space( this );	/* Map this tcb into the space. */
    space->handle_hash_miss( this );	/* Install this tcb into the pg hash. */
    space->handle_hash_miss( space );	/* TODO: is this the solution? */
#endif
}

INLINE word_t * tcb_t::get_stack_top()
{
    word_t stack;
    /* The powerpc eabi stack must be 8-byte aligned. */
    stack = ((word_t)this + TOTAL_TCB_SIZE) & ~(8-1);
    return (word_t *)stack;
}

INLINE void tcb_t::init_stack()
{
    this->stack = get_stack_top();
    TRACE_TCB( "stack = %p, tcb bottom = %p, tcb size = %d\n", 
	       this->stack, this, sizeof(tcb_t) );
}



/********************************************************************** 
 *
 *                      thread switch routines
 *
 **********************************************************************/


/**
 * tcb_t::switch_to: switches to specified tcb
 */
INLINE void tcb_t::switch_to(tcb_t * dest)
{
    ASSERT(dest->stack);
    ASSERT(get_cpu() == dest->get_cpu());
    ASSERT(dest != this);

    // TODO: adjust the thread switch return address to load 
    // resources.  Thus the common path need not check for a load.
    if( EXPECT_FALSE(this->resource_bits) )
	this->resources.save( this );

#ifdef CONFIG_PPC_MMU_SEGMENTS
    /* NOTE: pdir_cache holds the segment ID. */
    if ( (dest->pdir_cache != current->pdir_cache) && (dest->pdir_cache != 0) )
    {
	word_t dummy;
	asm volatile (
#if defined(CONFIG_PPC_SEGMENT_LOOP)
	    /* Here is a loop to set the segment registers.  It is 4 cycles
	     * slower than the nonlooped version.  But it has 17 fewer
	     * instructions totalling 68 bytes, and thus saves 
	     * over 2 cache lines.
	     */
	    "li %%r3, 12 ;"	// Init the loop count.
	    "mtctr %%r3 ;"	// Load the loop count.
	    "li %%r3, 0 ;"	// Init the segment register index.
	    "99:" 
	    "mtsrin %0, %%r3 ;"	// Set the segment register.
	    "addi %0, %0, 1 ;"	// Increment the segment ID.
	    "extlwi %%r3, %0, 4, 28; "	// Extract the segment register index.
	    "bdnz 99b ;"	// Loop.
#else
	    "mtsr 0, %0 ; addi %0, %0, 1 ;"
	    "mtsr 1, %0 ; addi %0, %0, 1 ;"
	    "mtsr 2, %0 ; addi %0, %0, 1 ;"
	    "mtsr 3, %0 ; addi %0, %0, 1 ;"
	    "mtsr 4, %0 ; addi %0, %0, 1 ;"
	    "mtsr 5, %0 ; addi %0, %0, 1 ;"
	    "mtsr 6, %0 ; addi %0, %0, 1 ;"
	    "mtsr 7, %0 ; addi %0, %0, 1 ;"
	    "mtsr 8, %0 ; addi %0, %0, 1 ;"
	    "mtsr 9, %0 ; addi %0, %0, 1 ;"
	    "mtsr 10, %0 ; addi %0, %0, 1 ;"
	    "mtsr 11, %0 ;"
#endif
	    "isync ;"
	    : "=r"(dummy)
	    : "0"(dest->pdir_cache));
    }
#elif defined(CONFIG_PPC_MMU_TLB)
    if (dest->pdir_cache)
    {
	asid_t *asid = (asid_t*)dest->pdir_cache;
	word_t current_pid = ppc_get_pid();
	if (asid->get() != current_pid)
	{
	    // AS switch...
	    if ( EXPECT_FALSE(!asid->is_valid()) )
		dest->get_space()->allocate_asid();
	    word_t dest_pid = get_asid_manager()->reference(asid);
	    ASSERT(dest_pid);
	    ppc_set_pid(dest_pid);
	}
    }
#endif

    word_t dummy0 asm("r27");
    word_t dummy1 asm("r28");
    word_t dummy2 asm("r29");

    asm volatile (
	    "stw %%r30, -4(%%r1) ;"		/* Preserve r30 on the stack. */
	    "stw %%r31, -8(%%r1) ;"		/* Preserve r31 on the stack. */
	    "mtsprg " MKSTR(SPRG_CURRENT_TCB) ", %[dest_tcb] ;"	/* Save the tcb pointer in sprg1. */
	    "lis %[dest_tcb], 1f@ha ;"		/* Grab the return address. */
	    "la  %[dest_tcb], 1f@l(%[dest_tcb]) ;"	/* Put the return address in %3. */
	    "stwu %[dest_tcb], -16(%%r1) ;"	/* Store (with update) the return address on the
						   current stack. */
	    "stw %%r1, 0(%[this_sp]) ;"		/* Save the current stack in old_stack. */
	    "addi %%r1, %[dest_sp], 16 ;"	/* Install the new stack. */
	    "lwz %%r3, -16(%%r1) ;"		/* Grab the new thread's address. */
	    "lwz %%r30, -4(%%r1) ;"		/* Restore r30. */
	    "lwz %%r31, -8(%%r1) ;"		/* Restore r31. */
	    "mtctr %%r3 ;"			/* Prepare to jump. */
	    "bctr ;"				/* Jump to the new thread. */
	    "1:"				/* The return address. */
	    : "=b" (dummy0), "=b" (dummy1), "=b" (dummy2)
	    : [dest_sp] "0" (dest->stack),
	      [this_sp] "1" (&this->stack),
	      [dest_tcb] "2" (dest)
	    : "memory", "r0", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", 
	      "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18",
	      "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",
	      "ctr", "lr", 
	      "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7", "xer"
        );

    if( EXPECT_FALSE(this->resource_bits) )
	this->resources.load( this );
}

/**********************************************************************
 *
 *                        notification functions
 *
 **********************************************************************/
INLINE void tcb_t::notify( void (*func)() )
{
    this->notify( (void (*)(word_t, word_t))func, 0, 0 );
}

INLINE void tcb_t::notify( void (*func)(word_t), word_t arg1 )
{
    this->notify( (void (*)(word_t, word_t))func, arg1, 0 );
}

/* 
 * access functions for ex-regs'able registers
 */
INLINE addr_t tcb_t::get_user_ip()
{
    return addr_t(get_user_syscall_regs(this)->srr0_ip);
}

INLINE addr_t tcb_t::get_user_sp()
{
    return addr_t(get_user_syscall_regs(this)->r1_stack);
}

INLINE word_t tcb_t::get_user_flags()
{
    return get_user_syscall_regs(this)->srr1_flags & MSR_USER_MASK;
}

INLINE void tcb_t::set_user_ip(addr_t ip)
{
    get_user_syscall_regs(this)->srr0_ip = word_t(ip);
}

INLINE void tcb_t::set_user_sp(addr_t sp)
{
    get_user_syscall_regs(this)->r1_stack = word_t(sp);
}

INLINE void tcb_t::set_user_flags(const word_t flags)
{
    get_user_syscall_regs(this)->srr1_flags = 
	(flags & MSR_USER_MASK) | MSR_USER;
}

INLINE void tcb_t::return_from_ipc (void)
{
    return_ipc_abort();
}

INLINE void tcb_t::return_from_user_interruption (void)
{
    word_t return_stack;
    extern word_t _except_return_shortcircuit[];

    // We want to short-circuit the return trip, to the exception
    // exit path.  So we jump to the point in assembler code which
    // starts restoring the user's full exception context.

    return_stack = (word_t)this->get_stack_top() - 
	(sizeof(except_regs_t) + EABI_STACK_SIZE);

    // Install the stack, and jump to the context store code.
    asm volatile (
	    "mtlr %0 ;"
	    "mr %%r1, %1 ;"
	    "blr ;"
	    :
	    : "r" ((word_t)_except_return_shortcircuit), "r" (return_stack)
	    : "r0"
	    );
}

/**********************************************************************
 *
 *                        in-kernel IPC invocation
 *
 **********************************************************************/

/**
 * tcb_t::do_ipc: invokes an in-kernel IPC
 * @param to_tid destination thread id
 * @param from_tid from specifier
 * @param timeout IPC timeout
 * @return IPC message tag (MR0)
 */
INLINE msg_tag_t tcb_t::do_ipc( threadid_t to_tid, threadid_t from_tid, timeout_t timeout )
{
    this->resources.set_kernel_ipc( this );

    word_t r3 asm("r3") = to_tid.get_raw();
    word_t r4 asm("r4") = from_tid.get_raw();
    word_t r5 asm("r5") = timeout.raw;

    /* ABI stack */
    asm volatile (
	    "addi %%r1, %%r1, -16 ;"	/* Allocate stack space for r30 and r31,
					   and the ABI stack space for calling
					   a function. */
	    "stw %%r30, 8(%%r1) ;"	/* Preserve r30. */
	    "stw %%r31, 12(%%r1) ;"	/* Preserve r31. */
	    "bl sys_ipc ;"		/* Call sys_ipc(). */
	    "lwz %%r31, 12(%%r1) ;"	/* Restore r31. */
	    "lwz %%r30, 8(%%r1) ;"	/* Restore r30. */
	    "addi %%r1, %%r1, 16 ;"	/* Clean-up the stack. */
	    : "+r" (r3), "+r" (r4), "+r" (r5)
	    : 
	    : "r0", "r2", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", 
	      "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", 
	      "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", 
	      "ctr", "lr", "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", 
	      "cr6", "cr7", "memory"
#if (__GNUC__ >= 3)
	      , "xer"
#endif
	    );

    this->resources.clr_kernel_ipc( this );

    msg_tag_t tag = this->get_mr(0);
    return tag;
}


/**********************************************************************
 *
 *                  copy-area related functions
 *
 **********************************************************************/

INLINE void tcb_t::adjust_for_copy_area( tcb_t * dst, addr_t * s, addr_t * d )
{
    resources.setup_copy_area( this, s, dst, d );
    resources.enable_copy_area( this );
}

INLINE void tcb_t::release_copy_area( void )
{
    resources.disable_copy_area( this );
}

INLINE addr_t tcb_t::copy_area_real_address( addr_t addr )
{
    return resources.copy_area_real_address( this, addr );
}

/**********************************************************************
 *
 *                        ctrlxfer tcb functions
 *
 **********************************************************************/
#if defined(CONFIG_X_CTRLXFER_MSG)
EXTERN_TRACEPOINT(IPC_CTRLXFER_ITEM_DETAILS);

INLINE void tcb_t::set_fault_ctrlxfer_items(word_t fault, ctrlxfer_mask_t mask)
{
    word_t idx = fault - 2;
    if (idx < IPC_CTRLXFER_STDFAULTS + arch_ktcb_t::fault_max)
	this->fault_ctrlxfer[idx] = mask;
}

INLINE ctrlxfer_mask_t tcb_t::get_fault_ctrlxfer_items(word_t fault)
{  
    word_t idx = fault - 2;
    return (idx < IPC_CTRLXFER_STDFAULTS + arch_ktcb_t::fault_max) ?
	this->fault_ctrlxfer[idx] : ctrlxfer_mask_t(0);
}

INLINE word_t tcb_t::append_ctrlxfer_item(msg_tag_t tag, word_t offset)
{
    word_t fault = (0x1000 - (tag.get_label() >> 4));
    if (get_fault_ctrlxfer_items(fault))
    {
	TRACE_CTRLXFER_DETAILS( "append ctrlxfer item %d", fault);
	flags += kernel_ctrlxfer_msg;
	msg_item_t item = ctrlxfer_item_t::kernel_fault_item(fault);
	set_mr( offset++, item.raw);
	return 1;
    }
    return 0;
}
#endif /* CONFIG_X_CTRLXFER_MSG */


/**********************************************************************
 *
 *                        global tcb functions
 *
 **********************************************************************/
INLINE void set_sprg_tcb( tcb_t *tcb )
{
    ppc_set_sprg( SPRG_CURRENT_TCB, (word_t)tcb );
}

__attribute__ ((const)) INLINE tcb_t *get_sprg_tcb()
{
    return (tcb_t *)ppc_get_sprg( SPRG_CURRENT_TCB );
}

__attribute__ ((const)) INLINE tcb_t * get_current_tcb()
{
    return addr_to_tcb( __builtin_frame_address(0) );
}

#if defined(CONFIG_SMP)
INLINE cpuid_t get_current_cpu()
{
    return get_idle_tcb()->get_cpu();
}
#endif

/**
 * initial_switch_to: switch to first thread
 * @param tcb TCB of initial thread.
 *
 * Switches to the initial thread.  The stack is expected to contain a
 * notify frame.
 * We use this function, rather than tcb_t::switch_to(), because the outgoing
 * stack isn't a valid tcb.
 */
INLINE void NORETURN initial_switch_to( tcb_t *tcb )
{
    // Store the target thread's tcb in the appropriate sprg.
    set_sprg_tcb( tcb );

    // Activate the thread switch frame.
    asm volatile (
	    "mtctr %0 ;"	// Prepare to branch.
	    "mr %%r1, %1 ;"	// Install the new stack.
	    "bctr ;"		// Branch to the instruction pointer.
	    : /* outputs */
	    : /* inputs */
	      "r" (get_kthread_ip(tcb)), "b" (tcb->stack)
	    );

    while( 1 );
}

/**********************************************************************
 *
 *                  architecture-specific functions
 *
 **********************************************************************/

/**
 * initialize architecture-dependent root server properties based on
 * values passed via KIP
 * @param space the address space this server will run in   
 * @param ip the initial instruction pointer           
 * @param sp the initial stack pointer
 */
INLINE void tcb_t::arch_init_root_server (space_t * space, word_t ip, word_t sp)
{ 
}

