/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * $Log:	kern_synch.c,v $
 * Revision 2.4  92/07/08  15:47:33  mrt
 * 	Initialize p_timedout and timed_out.
 * 	[92/07/07            rwd]
 * 	Only set p_stat to SRUN if it was SSLEEP.  Could have been stopped
 * 	while asleep.
 * 	[92/07/06            rwd]
 * 
 * Revision 2.3  92/05/25  14:43:55  rwd
 * 	Only set p_stat to SSLEEP if it was SRUN.  May be SSTOP for example
 * 	in which case we don't want to reset it.
 * 	[92/05/23            rwd]
 * 
 * Revision 2.2  92/05/19  15:50:36  rwd
 * 	Fix p->p_lock releasing in resume case.
 * 	[92/05/19            rwd]
 * 
 * Revision 2.1  92/04/21  17:12:18  rwd
 * BSDSS
 * 
 *
 */

/*-
 * Copyright (c) 1982, 1986, 1990 The Regents of the University of California.
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)kern_synch.c	7.18 (Berkeley) 6/27/91
 */

#include <diagnostic.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/buf.h>
#include <sys/signalvar.h>
#include <sys/resourcevar.h>
#include <sys/parallel.h>
#include <sys/synch.h>
#include <sys/queue.h>
#include <uxkern/import_mach.h>

extern void	ux_server_thread_busy();
extern void	ux_server_thread_active();
extern int	timeout_special();

/*
 * Sleeping threads are hashed by p_wchan onto sleep queues.
 */
 
#define	SLEEP_QUEUE_SIZE	64	/* power of 2 */
#define	SLEEP_HASH(x)	(((int)(x)>>16) & (SLEEP_QUEUE_SIZE - 1))

queue_head_t	sleep_queue[SLEEP_QUEUE_SIZE];

struct mutex	master_mutex = MUTEX_INITIALIZER;
struct mutex	sleep_lock = MUTEX_INITIALIZER;

/*
 * Initialize the sleep queues.
 */
rqinit()
{
	register int i;

	for (i = 0; i < SLEEP_QUEUE_SIZE; i++)
	    queue_init(&sleep_queue[i]);
}

thread_wakeup(p)
	register struct proc *p;
{
	register queue_t	q;

	mutex_lock(&sleep_lock);
	mutex_lock(&p->p_lock);
	if (p->p_wchan) {
		q = &sleep_queue[SLEEP_HASH(p->p_wchan)];
		queue_remove(q, p, struct proc *, p_sleep_link);
		p->p_wchan = (char *)0;
	}
	condition_signal(&p->p_condition);
	mutex_unlock(&p->p_lock);
	mutex_unlock(&sleep_lock);
}

tsleep_enter(chan, pri, wmesg, time_out)
	caddr_t chan;
	int pri, time_out;	/* time_out in hz */
        char *wmesg;
{
	register struct proc *p = (struct proc *)cthread_data(cthread_self());
	register queue_t	q;

	/*
	 * Zero is a reserved value, used to indicate
	 * that we have been woken up and are no longer on
	 * the sleep queues.
	 */

	if (chan == 0)
	    panic("tsleep");

	/*
	 * The sleep_lock protects the sleep queues and
	 * the uu_wchan/uu_interruptible fields in threads.
	 */

	mutex_lock(&sleep_lock);
	p->p_wchan = chan;
	p->p_wmesg = wmesg;
	p->p_pri = pri;
	q = &sleep_queue[SLEEP_HASH(chan)];
	queue_enter(q, p, struct proc *, p_sleep_link);
	mutex_unlock(&sleep_lock);

}

int
tsleep_main(chan, pri, time_out)
	caddr_t chan;
	int pri, time_out;	/* time_out in hz */
{
	register struct proc *p = (struct proc *)cthread_data(cthread_self());
	register queue_t	q;
	boolean_t		timed_out = 0;
	int			timeout_block;
	boolean_t 		catch = pri & PCATCH;
	int sig = 0;

	mutex_lock(&sleep_lock);
	if (p->p_wchan == 0) {
	    mutex_unlock(&sleep_lock);
	    goto resume;
	}

	if (catch) {
		p->p_flag |= SSINTR;
		if (sig = CURSIG(p)) {
		    mutex_unlock(&sleep_lock);
		    if (p->p_wchan)
			unsleep(p);
		    if (p->p_stat == SSLEEP)
			p->p_stat = SRUN;
		    goto resume;
		}
		if (p->p_wchan == 0) {
		    mutex_unlock(&sleep_lock);
		    catch = 0;
		    goto resume;
		}
	}

	mutex_lock(&p->p_lock);

	if (pri & (PSPECL|PCATCH)) ux_server_thread_busy();

	mutex_unlock(&sleep_lock);

	mutex_unlock(&master_mutex);
	if (time_out) {
		p->p_timedout = 1;
		timeout_block = timeout_special(thread_wakeup, p ,
						time_out, 1);
	} else {
	    p->p_timedout = 0;
	}
	if (p->p_stat == SRUN) /* May be using tsleep for SSTOP */
	    p->p_stat = SSLEEP;
	condition_wait(&p->p_condition, &p->p_lock);
	timed_out = p->p_timedout;
	if (time_out) untimeout_special(timeout_block);
	mutex_unlock(&p->p_lock);

	if (p->p_stat == SSLEEP)
	    p->p_stat = SRUN;
	mutex_lock(&master_mutex);

	if (pri & (PSPECL|PCATCH)) ux_server_thread_active();

      resume:
	p->p_flag &= ~SSINTR;
	if (timed_out)
	    if (catch == 0 || sig == 0)
		return (EWOULDBLOCK);
	if (catch && (sig != 0 || (sig = CURSIG(p)))) {
		if (p->p_sigacts->ps_sigintr & sigmask(sig))
			return (EINTR);
		return (ERESTART);
	}
	return (0);
}

tsleep(chan, pri, wmesg, time_out)
	caddr_t chan;
	int pri, time_out;	/* time_out in hz */
        char *wmesg;
{
	int			old_spl;
	int			result;

	tsleep_enter(chan, pri, wmesg, time_out);

	/*
	 * Network interrupt code might be called from spl_n.
	 * This code might try to wake us up.  Hence we can't
	 * hold uth->uu_lock or sleep_lock across spl_n,
	 * or we could deadlock with ourself.
	 */
	old_spl = spl_n(0);

	result = tsleep_main(chan, pri, time_out);

	(void) splx(old_spl);

	return (result);
}

/*
 * Sleep on chan at pri.
 *
 * We make a special check for lbolt (the once-per-second timer)
 * to avoid keeping a separate lbolt thread or overloading the
 * timeout queue.
 */
sleep(chan, pri)
	caddr_t chan;
	int pri;
{
	tsleep(chan, pri, "sleep",(chan == (caddr_t)&lbolt) ? hz : 0);
	return;
}

wakeup(chan)
	register caddr_t chan;
{
	register queue_t q;
	struct proc *p, *next;

	if (chan) {
        mutex_lock(&sleep_lock);
	q = &sleep_queue[SLEEP_HASH(chan)];

	p = (struct proc *)queue_first(q);
	while (!queue_end(q, (queue_entry_t)p)) {
	    next = (struct proc *)queue_next(&p->p_sleep_link);
	    if (p->p_wchan == chan) {
		queue_remove(q, p, struct proc *, p_sleep_link);
		p->p_wchan = (char *)0;
		/*
		 * wakeup thread
		 */
		mutex_lock(&p->p_lock);
		p->p_timedout = 0;
		condition_signal(&p->p_condition);
		mutex_unlock(&p->p_lock);
	    }
	    p = next;
	}
	mutex_unlock(&sleep_lock);
    } else
	panic("wakeup");
}

/*
 * Wakeup server thread to make it handle signals.
 */
unsleep(p)
    register struct proc *p;
{
	register int		old_state;
	register queue_t	q;

	mutex_lock(&sleep_lock);
	if ((p->p_flag&SSINTR) && p->p_wchan) {
		q = &sleep_queue[SLEEP_HASH(p->p_wchan)];
		queue_remove(q, p, struct proc *, p_sleep_link);
		p->p_wchan = (char *)0;
		mutex_lock(&p->p_lock);
		p->p_timedout = 0;
		condition_signal(&p->p_condition);
		mutex_unlock(&p->p_lock);
	}
	mutex_unlock(&sleep_lock);
}

/*
 * Stack of spl locks - one for each priority level.
 */
struct spl_lock_struct {
    cthread_t		holder;
    struct condition	condition;
};

struct spl_lock_struct	spl_locks[SPL_COUNT];
struct mutex spl_lock = MUTEX_INITIALIZER;

void
spl_init()
{
    register struct spl_lock_struct *sp;

    for (sp = &spl_locks[SPL_COUNT-1]; sp >= &spl_locks[0]; sp--) {
	sp->holder = NO_CTHREAD;
	condition_init(&sp->condition);
    }
}

int
spl_n(new_level)
	int	new_level;
{
    int	old_level;
    register int	i;
    register struct spl_lock_struct *sp;
    register cthread_t	self = cthread_self();
    register struct proc *p = (struct proc *)cthread_data(self);
    
    if (p->p_ipl < 0) {
	panic("current ipl < 0",p->p_ipl);
	p->p_ipl = 0;
    }
    
    if (new_level < 0) {
	panic("new_level < 0");
	new_level = 0;
    }
    
    old_level = p->p_ipl;
    
    if (new_level > old_level) {
	/*
	 * Raising priority
	 */
	mutex_lock(&spl_lock);
	for (i = old_level + 1; i <= new_level; i++) {
	    sp = &spl_locks[i];
	    
	    while (sp->holder != self && sp->holder != NO_CTHREAD)
		condition_wait(&sp->condition, &spl_lock);
	    sp->holder = self;
	}
	mutex_unlock(&spl_lock);
    }
    else if (new_level < old_level) {
	/*
	 * Lowering priority
	 */
	mutex_lock(&spl_lock);
	for (i = old_level; i > new_level; i--) {
	    sp = &spl_locks[i];
	    
	    sp->holder = NO_CTHREAD;
	    condition_signal(&sp->condition);
	}
	mutex_unlock(&spl_lock);
    }
    p->p_ipl = new_level;
    
	/*
	 * Simulate software interrupts for network.
	 */
	{
	    extern int	netisr;
	    extern void	dosoftnet();

	    if (new_level < SPLNET && netisr) {
		register int s = splnet();
		dosoftnet();
		splx(s);
	    }
	}

    return (old_level);
}

/*
 * Interrupt routines start at a given priority.  They may interrupt other
 * threads with a lower priority (unlike non-interrupt threads, which must
 * wait).
 */
void
    interrupt_enter(level)
int level;
{
    register cthread_t	self = cthread_self();
    register struct spl_lock_struct *sp = &spl_locks[level];
    register struct proc *p = (struct proc *)cthread_data(cthread_self());
    
    /*
     * Grab the lock for the interrupt priority.
     */
    
    mutex_lock(&spl_lock);
    while (sp->holder != self && sp->holder != NO_CTHREAD)
	condition_wait(&sp->condition, &spl_lock);
    sp->holder = self;
    mutex_unlock(&spl_lock);
    
    p->p_ipl = level;
}

void
    interrupt_exit(level)
int level;
{
    register cthread_t	self = cthread_self();
    register struct spl_lock_struct *sp;
    register int		i;
    register struct proc *p = (struct proc *)cthread_data(cthread_self());
    
    /*
     * Release the lock for the interrupt priority.
     */
    
    mutex_lock(&spl_lock);
    for (i = p->p_ipl; i >= level; i--) {
	sp = &spl_locks[i];
	sp->holder = NO_CTHREAD;
	condition_signal(&sp->condition);
    }
    mutex_unlock(&spl_lock);
    
    /*
     * Simulate software interrupts for network.
     */
    {
	extern int	netisr;
	extern void	dosoftnet();

	if (netisr && level >= SPLNET) {
	    /*
	     * Check SPL levels down to splnet.  If none held,
	     * take a softnet interrupt.
	     */
	    for (i = level; i >= SPLNET; i--)
		if (spl_locks[i].holder != NO_CTHREAD)
		    goto exit;

	    interrupt_enter(SPLNET);
	    dosoftnet();
	    interrupt_exit(SPLNET);

	  exit:;
	}
    }

    p->p_ipl = -1;
}

