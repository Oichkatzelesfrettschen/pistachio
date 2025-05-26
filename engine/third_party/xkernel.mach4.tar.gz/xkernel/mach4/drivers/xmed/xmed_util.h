/* 
 * $RCSfile: xmed_util.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1991  Arizona Board of Regents
 *
 *
 * $Revision: 1.2 $
 * $Date: 1992/11/18 04:21:19 $
 */

#ifndef xmed_util_h
#define xmed_util_h

#include <sys/types.h>
#include "xkernel.h"
#include "eth_i.h"

typedef struct {
    u_int	*next_word;		/* next word to be written */
    int		remaining_space;	/* # of free bytes in partial word */
    u_int	partial_word;		/* partial word that's written next */
} xmed_copy_state_t;

extern bool xmed_copy_out(u_char *src, long len, xmed_copy_state_t *state);
extern void xmed_copy_in(u_int *src, long len, ETHhdr *hp, u_int *dst);

#endif /* xmed_util_h */
