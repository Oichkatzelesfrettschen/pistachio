/*     
 * $RCSfile: config.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.2 $
 * $Date: 1993/05/09 00:35:16 $
 */

/* 
 * Configuration-tunable parameters
 */


/* 
 * Default number of shepherd threads started by the simeth driver.
 * These threads are shared by all instances of the driver.  This
 * number can be overridden by a ROM file option of the form:
 *
 *	shepherd threads N
 */
#define SIMETH_SHEPHERD_THREADS	32
