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
 * $Log: fpu.c,v $
 */
/*
 * 	File: ns532/fpu.c
 *	Author: Johannes Helander, Helsinki University of Technology 1992.
 *
 *	Floating point support for the 32081, 32381, and 32581 FPUs
 *	together with the 32532 CPU.
 */
/* 
 * ns532 floating point semantics (MK42):  (jvh@cs.hut.fi 910328)
 * NOTE: Should check this note for MK43 and later //jvh 910511
 * 
 * Each threads float state is initialized (in pcb) when the thread is created.
 * A threads float state is stored in only one place (at any one time): 
 *   Either the FPU or pcb.
 * A threads fp state is always read from the pcb and written there.
 * fp_thread:s pcb version of the FPU state is always illegal. Use fp_save.
 *
 * There are three FPU states: 
 * - A) nobody uses it (THREAD_NULL uses it)
 * - B) the current thread is using it
 * - C) another thread uses it.
 * 
 * Transitions:
 * A->B fp_load
 * A->C -           (illegal)
 * B->A fp_save or fp_thread_float_terminate
 * B->C -           (illegal)
 * C->A fp_save or fp_thread_float_terminate
 * C->B -           (use the sequence C->A, A->B)
 *
 * The FPU is invalidated in each context switch. It is validated in two cases:
 * - Through fp_load (transition A->B)
 * - If fp_thread == current_thread
 * In any case the FPU will be validated only as a consequence of an UND trap.
 *
 * There are three kinds of float states: FP_NO <<< FP_NS081 <<< FP_NS381.
 * "<<<" means "is a subset of". FP_NS380 is not supported (we don't have one).
 * The kernel variable fp_kind_hard tells us what kind of FPU is actually 
 * installed. 
 *
 * Each thread has its associated own fp_kind. It may be FP_NO or the value of
 * fp_kind_hard. By setting the threads fp_kind to FP_NO it will get
 * an UND exception if it uses any FPU instruction. The default value for a
 * threads fp_kind is fp_kind_hard. Setting a threads fp_kind >>> fp_kind_hard
 * will not succeed.
 *
 * Note: If a thread is terminated, we need to do C->A or B->A if that 
 * thread is fp_thread.
 *
 * Initialize fp state when needed.
 */

#include <platforms.h>

#include <mach/exception.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <ns532/thread.h>
#include <ns532/fp_reg.h>
#include <ns532/machparam.h>

extern zone_t fps_zone;
thread_t	fp_thread = THREAD_NULL;

/* We assume NS32381 is standard and default to it. */
#ifdef NS32081
int 		fp_kind_hard = NS532_FP_NS081;
#else
int		fp_kind_hard = NS532_FP_NS381; 
#endif

/*
 * FPU error.
 */
void fpintr()
{
	int ss;
	
	ss = splsched();
	fp_save();
	splx(ss);
	exception(EXC_ARITHMETIC, EXC_NS532_SLAVE, 
		  (int)current_thread()->pcb->fps->fsr);
	/*NOTREACHED*/
}

/*
 * Initialize threads float state. Called from pcb_init.
 */
void fp_set_default_state(pcb)
	pcb_t pcb;
{
	register struct ns532_fp_state *fps;
	
	fps = (struct ns532_fp_state *) zalloc(fps_zone);
	bzero((char *)fps, sizeof(*fps));
	fps->fp_kind = fp_kind_hard;
	fps->valid = TRUE;
	pcb->fps = fps;
}

/*
 * Save FPU state.
 * For simplicity we always invalidate the FPU here.
 *
 * Entry conditions: splsched or higher.
 */
kern_return_t fp_save()
{
	register struct ns532_fp_state *fps;
	
	fps = fp_thread->pcb->fps;
	assert(fps != 0);
	fp_thread = THREAD_NULL;
	switch(fps->fp_kind)
	{
	      case NS532_FP_NO:
		return KERN_FAILURE;
		break;
	      case NS532_FP_NS081:
		if(fp_kind_hard == NS532_FP_NO)
		    panic("fp_save: case NS532_FP_NS081: No FPU present");
		_fpsave_ns081(fps);
		break;
	      case NS532_FP_NS381:
		switch(fp_kind_hard) 
		{
		      case NS532_FP_NO:
		      case NS532_FP_NS081:
panic("fp_save: case NS532_FP_NS381: Not supported by this kernel");
			break;
		      case NS532_FP_NS381:
		      case NS532_FP_NS581:
			_fpsave_ns381(fps);
			break;
		      default:
			panic("fp_save: unkown fp_kind_hard");
			break;
		}
		break;
	      case NS532_FP_NS581:
		switch(fp_kind_hard) 
		{
		      case NS532_FP_NO:
		      case NS532_FP_NS081:
		      case NS532_FP_NS381:
panic("fp_save: case NS532_FP_NS581: Not supported by this kernel");
			break;
		      case NS532_FP_NS581:
			_fpsave_ns381(fps);
			break;
		      default:
			panic("fp_save: unkown fp_kind_hard");
			break;
		}
		break;

	      default:
		panic("fp_save: unknown fp_kind");
		break;
	}
	return KERN_SUCCESS;
}

/*
 * Restore FPU state from PCB.
 *
 * Entry conditions: splsched or higher
 */
kern_return_t fp_load()
{
	register struct ns532_fp_state *fps;
	thread_t thread;
	
	assert(fp_thread == THREAD_NULL);
	
	thread = current_thread();
	fps = thread->pcb->fps;
	assert(fps != 0);
	switch(fps->fp_kind)
	{
	      case NS532_FP_NO:
		return KERN_FAILURE;
		break;
	      case NS532_FP_NS081:
		if(fp_kind_hard == NS532_FP_NO)
		    panic("fp_load: case FP_NS081: No FPU present");
		_fpload_ns081(fps);
		break;
	      case NS532_FP_NS381:
		switch(fp_kind_hard) 
		{
		      case NS532_FP_NO:
		      case NS532_FP_NS081:
panic("fp_load: case NS532_FP_NS381: Not supported by this kernel");
			break;
		      case NS532_FP_NS381:
		      case NS532_FP_NS581:
			_fpload_ns381(fps);
			break;
		      default:
			panic("fp_load: unkown fp_kind_hard");
			break;
		}
		break;
	      case NS532_FP_NS581:
		switch(fp_kind_hard) 
		{
		      case NS532_FP_NO:
		      case NS532_FP_NS081:
		      case NS532_FP_NS381:
panic("fp_load: case NS532_FP_NS581: Not supported by this kernel");
			break;
		      case NS532_FP_NS581:
			_fpload_ns381(fps);
			break;
		      default:
			panic("fp_load: unkown fp_kind_hard");
			break;
		}
		break;
	      default:
		panic("fp_load: unknown fp_kind");
		break;
	}
	fp_thread = thread;
	return KERN_SUCCESS;
}

/* 
 * Throw away threads float state. Called from pcb_terminate.
 * Input conditions: none
 */
void fps_terminate(thread)
	thread_t thread;
{
	int ss;
	struct ns532_fp_state *fps;
	
	ss = splsched();	
	if(fp_thread == thread){
#ifdef FP_DEBUG
		printf("fp_thread_float_terminate: terminating %x\n", thread);
#endif FP_DEBUG
		fp_thread = THREAD_NULL;	
	}		
	splx(ss);
	
	fps = thread->pcb->fps;
	thread->pcb->fps = 0;
	if(fps != 0)
	    zfree(fps_zone, (vm_offset_t) fps);
}


/*
 * FPU was disabled. Handle the trap.
 */
kern_return_t fp_disabled_trap()
{
	int ss;
	int r;
	/*
	 * Give FPU to this thread.
	 */
	
	if(fp_kind_hard == NS532_FP_NO) {
#ifdef FP_DEBUG
printf("fp_disabled_trap: No FPU present. thread=%x\n", current_thread());
#endif FP_DEBUG
		return KERN_FAILURE;	/* no FPU present */
	}
	
	ss = splsched();
	if (fp_thread != current_thread()) {
#ifdef FP_DEBUG
		printf("fp_disabled_trap: FPU thread changed from %x to %x\n", 
		       fp_thread, current_thread());
#endif FP_DEBUG
		if (fp_thread != THREAD_NULL) {
			fp_save(); 	/* save fp_thread float state */
		}
		r = fp_load();		/* load current thread float state */
		splx(ss);
		return r;
	} else {
#ifdef FP_DEBUG
printf("fp_disabled_trap: enabling FPU for thread %x\n", fp_thread);
#endif FP_DEBUG
		_enable_fpu();
		splx(ss);
		return KERN_SUCCESS; 	/* SUCCESS. Floats enabled, go ahead */
	}
}	


#ifdef NS32081
/* dummy functions. These functions are normally defined in locore.s */
int _fpsave_ns381()
{
printf("_fpsave_ns381: NS32381 FPU not supported in this configuration\n");
}

int _fpload_ns381()
{
printf("_fpload_ns381: NS32381 FPU not supported in this configuration\n");
}
#endif NS32381

