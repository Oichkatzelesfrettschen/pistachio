/*     
 * $RCSfile: config.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.1 $
 * $Date: 1993/05/11 04:30:32 $
 */

/* 
 * Configuration-tunable parameters
 */


/* 
 * Default number of shepherd threads started up.
 * These threads are shared by all instances of ethernet drivers.  This
 * number can be overridden by a ROM file option of the form:
 *
 *	shepherd threads N
 */
#define XK_SHEPHERD_THREADS	16

/* 
 * Default number of input buffers.  There should be at least as many
 * buffers as input threads.  If there are more buffers than input
 * threads, incoming packets can be queued by the driver if all
 * shepherd threads are busy.  This number can be overridden by a ROM
 * file option of the form:
 *
 *	shepherd buffers N
 */
#define XK_SHEPHERD_BUFFERS	32
