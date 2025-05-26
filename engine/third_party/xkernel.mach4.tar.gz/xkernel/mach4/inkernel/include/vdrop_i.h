/* 
 * $RCSfile: vdrop_i.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.4 $
 * $Date: 1993/02/12 23:13:47 $
 */


#define	VDROP_ACT_MAP_SZ	11
#define	VDROP_PAS_MAP_SZ	11

/* 
 * VDROP_MAX_INTERVAL is the upper bound of the initial interval
 * value.  The actual interval value is randomly selected between 2
 * and VDROP_MAX_INTERVAL.  User-specified intervals may exceed
 * VDROP_MAX_INTERVAL. 
 */
#define VDROP_MAX_INTERVAL	20


typedef struct {
    Map		activeMap;
    Map 	passiveMap;
} PState;

typedef struct {
    int		interval;
    int		count;
} SState;

