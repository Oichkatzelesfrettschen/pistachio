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
 * $Log: pcb.c,v $
 */
/*
 * 	File: ns532/pcb.c
 *
 * 	Author: Johannes Helander, Helsinki University of Technology 1992.
 * 	Created from i386/pcb.c.
 */

#include <mach/thread_status.h>
#include <mach/kern_return.h>

#include <kern/counters.h>
#include <kern/mach_param.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <mach/vm_param.h>
#include <vm/pmap.h>

#include <ns532/thread.h>
#include <ns532/psl.h>
#include <ns532/fp_reg.h>
#include <ns532/machparam.h>

#include "mach_kdb.h"

extern thread_t Switch_context();
extern void Thread_continue();

zone_t pcb_zone;
zone_t fps_zone;

/*
 *	stack_attach:
 *
 *	Attach a kernel stack to a thread.
 */

#if MACH_KDB && 0

thread_t stack_attach_arg_thread = (thread_t)-1;
vm_offset_t stack_attach_arg_stack = (vm_offset_t)-1;
void (*stack_attach_arg_continuation)() = (void (*)())-1;
thread_t stack_attach_active_thread = (thread_t)-1;
vm_offset_t stack_attach_active_stack = (vm_offset_t)-1;

#endif MACH_KDB && 0

void stack_attach(thread, stack, continuation)
	register thread_t thread;
	register vm_offset_t stack;
     	void (*continuation)();
{
#if MACH_KDB && 0
	stack_attach_arg_thread = thread;
	stack_attach_arg_stack = stack;
	stack_attach_arg_continuation = continuation;
	stack_attach_active_thread = active_threads[0];
	stack_attach_active_stack = active_stacks[0];
	assert(continuation != (void (*)()) 0);
#endif MACH_KDB && 0

 	counter(if(++c_stacks_current > c_stacks_max)
		c_stacks_max = c_stacks_current);

	thread->kernel_stack = stack;

	/*
	 *      We want to run continuation, giving it as an argument
	 *      the return value from Load_context/Switch_context.
	 *      Thread_continue takes care of the mismatch between
	 *      the argument-passing/return-value conventions.
	 *	This function will not return normally,
	 *	so we don't have to worry about a return address.
	 */
	STACK_IKS(stack)->k_pc = (int) Thread_continue;
	STACK_IKS(stack)->k_r3 = (int) continuation;	
	STACK_IKS(stack)->k_sp = (int) STACK_ISS(stack);

	/*
	 *	XXX Copy user's registers from pcb to stack.
	 */
	bcopy((char *) &thread->pcb->iss, (char *) STACK_ISS(stack),
	      sizeof(struct ns532_saved_state));
}

/*
 *	stack_detach:
 *
 *	Detaches a kernel stack from a thread, returning the old stack.
 */

vm_offset_t stack_detach(thread)
	register thread_t	thread;
{
	register vm_offset_t	stack;

	stack = thread->kernel_stack;
	thread->kernel_stack = 0;

	counter(if (--c_stacks_current < c_stacks_min)
		c_stacks_min = c_stacks_current);
	
	/*
	 *	XXX Copy user's registers from stack to pcb.
	 */
	bcopy((char *) STACK_ISS(stack), (char *) &thread->pcb->iss,
	      sizeof(struct ns532_saved_state));

	return stack;
}

/*
 *	stack_handoff:
 *
 *	Move the current thread's kernel stack to the new thread.
 */

void stack_handoff(old, new)
	register thread_t	old;
	register thread_t	new;
{
	register vm_offset_t	stack;

	{
		task_t old_task, new_task;

		if ((old_task = old->task) != (new_task = new->task)) { 
			PMAP_DEACTIVATE_USER(vm_map_pmap(old_task->map),
					     old, cpu_number());
			PMAP_ACTIVATE_USER(vm_map_pmap(new_task->map),
					   new, cpu_number());
		}
	}

	stack = current_stack();
	old->kernel_stack = 0;
	new->kernel_stack = stack;

	active_threads[cpu_number()] = new;

	/*
	 *	XXX Save old user registers back to pcb,
	 *	and replace with new user registers.
	 */

	bcopy((char *) STACK_ISS(stack), &old->pcb->iss,
	      sizeof(struct ns532_saved_state));
	bcopy((char *) &new->pcb->iss, (char *) STACK_ISS(stack),
	      sizeof(struct ns532_saved_state));

}

thread_t switch_context(old, continuation, new)
        register thread_t       old;
        void (*continuation)();
        register thread_t       new;
{
	{
		task_t old_task, new_task;

		if ((old_task = old->task) == (new_task = new->task)) {
			PMAP_CONTEXT(vm_map_pmap(new_task->map), new);
		} else {
			PMAP_DEACTIVATE_USER(vm_map_pmap(old_task->map),
					     old, cpu_number());
			PMAP_ACTIVATE_USER(vm_map_pmap(new_task->map),
					   new, cpu_number());
		}
	}
	return Switch_context(old, continuation, new);
}


void pcb_module_init()
{
	pcb_zone = zinit(sizeof(struct pcb),
			 THREAD_MAX * sizeof(struct pcb),
			 THREAD_CHUNK * sizeof(struct pcb),
			 FALSE, "ns532 pcb state");
	
	fps_zone = zinit(sizeof(struct ns532_fp_state),
			 THREAD_MAX * sizeof(struct ns532_fp_state),
			 THREAD_CHUNK * sizeof(struct ns532_fp_state),
			 FALSE, "ns532_fp_state");
}

void pcb_init(thread)
	register thread_t	thread;
{
	register pcb_t		pcb;
	
	pcb = (pcb_t) zalloc(pcb_zone);
	if (pcb == 0)
	    panic("pcb_init");
	
	counter(if (++c_threads_current > c_threads_max)
		c_threads_max = c_threads_current);
	
        /*
         *      We can't let random values leak out to the user.
         */
        bzero((char *) pcb, sizeof *pcb);
	
        /*
         *      Guarantee that the bootstrapped thread will be in user
         *      mode (this psl assignment above executes the bootstrap
         *      code in kernel mode.
         */
	
	pcb->iss.psr = PSR_U | PSR_I | PSR_S;
	
	/* set default FPU state */
	fp_set_default_state(pcb);
	
        thread->pcb = pcb;
}

void pcb_terminate(thread)
        register thread_t       thread;
{
        register pcb_t          pcb = thread->pcb;
	
	counter(if (--c_threads_current < c_threads_min)
		c_threads_min = c_threads_current);
	
	fps_terminate(thread);
        zfree(pcb_zone, (vm_offset_t) pcb);
        thread->pcb = 0;
}

/*
 *      pcb_collect:
 *
 *      Attempt to free excess pcb memory.
 */

void pcb_collect(thread)
        thread_t thread;
{
}


/*
 *	thread_setstatus:
 *
 *	Set the status of the specified thread.
 */

kern_return_t thread_setstatus(thread, flavor, tstate, count)
	thread_t		thread;
	int			flavor;
	thread_state_t		tstate;
	unsigned int		count;
{
	switch (flavor) {
	      case NS532_COMBINED_STATE:
		{
			register struct ns532_combined_state	*state;
			kern_return_t				kr;
			
			if (count < NS532_COMBINED_STATE_COUNT)
			{
				return KERN_INVALID_ARGUMENT;
			}
			state = (struct ns532_combined_state *) tstate;
			kr = thread_setstatus(thread, NS532_FLOAT_STATE, 
					      (thread_state_t)&state->fs,
					      NS532_FLOAT_STATE_COUNT);
			if(kr != KERN_SUCCESS)
			    return kr;
			return thread_setstatus(thread, NS532_THREAD_STATE, 
						(thread_state_t)&state->ts,
						NS532_THREAD_STATE_COUNT);
		}
		break;
	      case NS532_THREAD_STATE: 
		{
			register struct ns532_thread_state	*state;	
			register struct ns532_saved_state	*saved_state;
			
			if (count < NS532_THREAD_STATE_COUNT) 
			{
				return KERN_INVALID_ARGUMENT;
			}	
			
			state = (struct ns532_thread_state *) tstate;
			saved_state = USER_REGS(thread);
			
			/*
			 * General registers
			 */
			saved_state->r0 = state->r0;
			saved_state->r1 = state->r1;
			saved_state->r2 = state->r2;
			saved_state->r3 = state->r3;
			saved_state->r4 = state->r4;
			saved_state->r5 = state->r5;
			saved_state->r6 = state->r6;
			saved_state->r7 = state->r7;
			saved_state->sb = state->sb;
			saved_state->fp = state->fp;
			saved_state->usp = state->sp;
			saved_state->pc = state->pc;
			saved_state->mod = state->mod;
			saved_state->psr = (saved_state->psr & 0xffffff00)
			    | (state->psr & 0xff);
		}
		break;
	      case NS532_FLOAT_STATE:
		{
			struct ns532_float_state	*float_state;
			struct ns532_fp_state 	*fps;
			int ss;
			
			extern int fp_kind_hard;
			extern thread_t fp_thread;
			
			if (count < NS532_FLOAT_STATE_COUNT)
			    return KERN_INVALID_ARGUMENT;
			
			float_state = (struct ns532_float_state *) tstate;
			fps = thread->pcb->fps;
			if(fps == 0)
			{
				(void)fp_set_default_state(thread->pcb);
				fps = thread->pcb->fps;
				assert(fps != 0);
			}
			switch(float_state->fp_kind)
			{
			      case NS532_FP_NO:
				break;
			      case NS532_FP_NS081:
				switch(fp_kind_hard)
				{
				      case NS532_FP_NS081:
				      case NS532_FP_NS381:
				      case NS532_FP_NS581:
					break;
				      default:
					return KERN_FAILURE;
				}
			      case NS532_FP_NS381:
				switch(fp_kind_hard)
				{
				      case NS532_FP_NS381:
				      case NS532_FP_NS581:
					break;
				      default:
					return KERN_FAILURE;
				}
			      case NS532_FP_NS581:
				switch(fp_kind_hard)
				{
				      case NS532_FP_NS581:
					break;
				      default:
					return KERN_FAILURE;
				}
			      default:
				return KERN_FAILURE;
			}
			ss = splsched();
			if(thread == fp_thread)
			    fp_save();
			splx(ss);
			/* copy in stuff */
			/* XXXX ns532_fp_state === ns532_float_state */
			bcopy((char *)float_state, (char *)fps, sizeof(*fps));
			fps->valid = TRUE;
		}	
		break;
	      default:	
		return KERN_INVALID_ARGUMENT;
	}
	return KERN_SUCCESS;
}

/*
 *	thread_getstatus:
 *
 *	Get the status of the specified thread.
 */

kern_return_t thread_getstatus(thread, flavor, tstate, count)
	register thread_t	thread;
	int			flavor;
	thread_state_t		tstate;	/* pointer to OUT array */
	unsigned int		*count;	/* IN/OUT */
{
	
	switch (flavor)  {
	      case THREAD_STATE_FLAVOR_LIST:
		if (*count < 3)
		    return (KERN_INVALID_ARGUMENT);
		tstate[0] = NS532_THREAD_STATE;
		tstate[1] = NS532_FLOAT_STATE;
		tstate[2] = NS532_COMBINED_STATE;
		*count = 3;
		break;
		
	      case NS532_COMBINED_STATE:
		{
			register struct ns532_combined_state	*state;
			kern_return_t				kr;
			unsigned int				cnt;
			
			if (*count < NS532_COMBINED_STATE_COUNT)
			{
				return KERN_INVALID_ARGUMENT;
			}
			state = (struct ns532_combined_state *) tstate;
			cnt = NS532_FLOAT_STATE_COUNT;
			kr = thread_getstatus(thread,
					      NS532_FLOAT_STATE, 
					      (thread_state_t) &state->fs,
					      &cnt);
			if(kr != KERN_SUCCESS)
			    return kr;
			cnt = NS532_THREAD_STATE_COUNT;
			kr = thread_getstatus(thread,
					      NS532_THREAD_STATE, 
					      (thread_state_t) &state->ts, 
					      &cnt);
			*count = NS532_COMBINED_STATE_COUNT;
			return kr;
		}
		break;
	      case NS532_THREAD_STATE:
		{
			struct ns532_thread_state	*state;
			struct ns532_saved_state	*saved_state;
			
			if (*count < NS532_THREAD_STATE_COUNT)
			    return KERN_INVALID_ARGUMENT;
			
			state = (struct ns532_thread_state *) tstate;
			saved_state = USER_REGS(thread);
			
			/*
			 * General registers.
			 */
			state->r0 = saved_state->r0;
			state->r1 = saved_state->r1;
			state->r2 = saved_state->r2;
			state->r3 = saved_state->r3;
			state->r4 = saved_state->r4;
			state->r5 = saved_state->r5;
			state->r6 = saved_state->r6;
			state->r7 = saved_state->r7;
			state->sb = saved_state->sb;
			state->fp = saved_state->fp;
			state->sp = saved_state->usp;
			state->pc = saved_state->pc;
			state->mod = saved_state->mod;
			state->psr = saved_state->psr;
			*count = NS532_THREAD_STATE_COUNT;
			break;
		}    
	      case NS532_FLOAT_STATE:
		{
			struct ns532_float_state	*float_state;
			struct ns532_fp_state 		*fps;
			int ss;
			
			extern int fp_kind_hard;
			extern thread_t fp_thread;
			
			if (*count < NS532_FLOAT_STATE_COUNT)
			    return KERN_INVALID_ARGUMENT;
			
			float_state = (struct ns532_float_state *) tstate;
			fps = thread->pcb->fps;
			if(fps == 0)
			    float_state->valid = FALSE;
			else 
			{
				ss = splsched();
				if(thread == fp_thread)
				    fp_save();
				splx(ss);
				/* copy out stuff */
				/* XXXX ns532_fp_state === ns532_float_state */
				bcopy(fps, float_state, sizeof(*float_state));
			}
			*count = NS532_FLOAT_STATE_COUNT;
			break;
		}	
	      default:
		return(KERN_INVALID_ARGUMENT);
	}
	return KERN_SUCCESS;
}


/*
 * Alter the thread`s state so that a following thread_exception_return
 * will make the thread return 'retval' from a syscall.
 */
void
thread_set_syscall_return(thread, retval)
	thread_t	thread;
	kern_return_t	retval;
{
	thread->pcb->iss.r0 = retval;
}

/*
 * Return prefered address of user stack.
 * Always returns low address.  If stack grows up,
 * the stack grows away from this address;
 * if stack grows down, the stack grows towards this
 * address.
 */
vm_offset_t
user_stack_low(stack_size)
	vm_size_t	stack_size;
{
	return (VM_MAX_ADDRESS - stack_size);
}

/*
 * Allocate argument area and set registers for first user thread.
 */
vm_offset_t
set_user_regs(stack_base, stack_size, entry, arg_size)
	vm_offset_t	stack_base;	/* low address */
	vm_offset_t	stack_size;
	int		*entry;
	vm_size_t	arg_size;
{
	vm_offset_t	arg_addr;
	register struct ns532_saved_state *saved_state;
	
	arg_size = (arg_size + sizeof(int) - 1) & ~(sizeof(int)-1);
	arg_addr = stack_base + stack_size - arg_size;
	
	saved_state = USER_REGS(current_thread());
	saved_state->usp = (int)arg_addr;
	saved_state->pc = entry[0];
	
	return (arg_addr);
}
