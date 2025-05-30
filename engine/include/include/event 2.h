/*
 * event.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.20 $
 * $Date: 1993/06/04 06:30:07 $
 */

#ifndef event_h
#define event_h

#include "idmap.h"
#include "xtime.h"

/*
 *   Return values for evCancel
 */
typedef enum {
    EVENT_FINISHED = -1,
    EVENT_RUNNING = 0,
    EVENT_CANCELLED = 1
} EvCancelReturn;

/*
 * This structure is to be opaque.  We have to define it here, but protocol
 * implementors are not allowed to know what it contains.
 */
typedef enum {
    E_PENDING,
    E_SCHEDULED,
    E_RUNNING,
    E_BLOCKED,
    E_FINISHED
} EvState;


#if defined(XK_DEBUG)
#  define XK_THREAD_TRACE
#endif


#ifdef XK_THREAD_TRACE
#  define EV_CHECK_STACK(_str)	evCheckStack(_str)
#else
#  define EV_CHECK_STACK(_str)	do {} while(0)
#endif


typedef struct Event {
    struct Event *next, *prev;
    unsigned 	deltat;
    void	(* func)();
    VOID	*arg;
    EvState	state;
    unsigned 	flags;
#ifdef XK_THREAD_TRACE
    XTime	startTime;	/* Time started */
    XTime	stopTime;	/* Time stopped or blocked */
    Bind	bind;
    VOID	*earlyStack;
#endif
} *Event;



typedef void (*EvFunc)(Event, void *);

/* schedule an event that executes f w/ argument a after delay t usec; */
/* t may equal 0, which implies createprocess */
Event evSchedule(EvFunc f, void *a, unsigned t);


/*
 * releases a handle to an event; as soon f completes, the resources
 * associated with the event are freed
 */
void evDetach(Event e);

/* cancel event e:
 *  returns EVENT_FINISHED if it knows that the event has already executed
 *     to completion
 *  returns EVENT_RUNNING if the event is currently running
 *  returns EVENT_CANCELLED if the event has been successfully cancelled
 */
EvCancelReturn evCancel(Event e);


/* 
 * returns true (non-zero) if an 'evCancel' has been performed on the event
 */
int evIsCancelled(Event e);


/* 
 * Displays a 'ps'-style listing of x-kernel threads
 */
void evDump(void);


/* 
 * Platform-specific check to see if this event is in danger of
 * overrunning the stack, triggering warning/error messages if so. 
 */
void evCheckStack(char *);


#endif
