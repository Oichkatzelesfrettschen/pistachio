/* 
 * join_i.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993  Arizona Board of Regents
 *
 * $Revision: 1.2 $
 * $Date: 1993/06/24 20:48:23 $
 */

#ifndef join_i_h
#define join_i_h

#define MAXID	256
#define JOINID  57
#define DEFAULT_NUMSEG 4
#define MAXPART 5


typedef struct {
    int 	id;
    int 	numSeg;
    int 	len[JOINMAXSEG-1];
} JOINhdr;

typedef XObj 	ActiveId;

typedef struct {
    int		seg;
} SState;    /* session state */

typedef struct {
    int	  	state;
#define JOININIT 0
#define JOINREADY 1
#define JOINDYING 2
    int	  	next;
    XObj  	up[JOINMAXSEG];
    Msg	  	upSeg[JOINMAXSEG];
    int	  	upSegFull[JOINMAXSEG];
    Msg	  	downSeg[JOINMAXSEG];
    int	  	downSegFull[JOINMAXSEG];
    Semaphore 	wait;
    int		numSeg;
    int		order[JOINMAXSEG];
    char  	id[MAXID];
    Bind  	tagBinding;
    XObj	controlSessn;
} TState; /* tag session state */

typedef struct {
    Map		actMap;
    Map 	pasMap;
    Map 	tagMap;
    int		tagIdLen;
    int		pasIdLen;
    int		numSeg;
    int		order[JOINMAXSEG];
} PState;    /* protocol state */

typedef struct {
    int		rcnt;
    XObj	hlpRcv[JOINMAXSEG];
    XObj	hlpType[JOINMAXSEG];
    XObj	saveHlpType;
    int		next;
    Bind	binding;
} JEnable;



#define	JOIN_ACTIVE_MAP_SZ	257
#define	JOIN_PASSIVE_MAP_SZ	13
#define	JOIN_TAG_MAP_SZ		257

#endif
