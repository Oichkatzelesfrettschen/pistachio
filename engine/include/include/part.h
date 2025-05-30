/* 
 * part.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.14 $
 * $Date: 1993/03/01 08:30:05 $
 */

#ifndef part_h
#define part_h

#ifndef xtype_h
#include "xtype.h"

/* 
 * Participant library
 */

/********************  private/opaque declarations ****************/

#define	PART_MAX_STACK	20

typedef struct {
    struct {
	VOID	*ptr;
	int	len;
    } arr[PART_MAX_STACK];
    int		top;
} PartStack;

typedef struct {
    int		len;
    PartStack	stack;	/* A stack of void* pointers */
} Part;


void	partStackPush( PartStack *s, void *data, int );
void *	partStackPop( PartStack *s );

/********************  public declarations ****************/

#define LOCAL_PART  1
#define REMOTE_PART 0

/* 
 * Initialize a vector of N participants
 */
void	partInit( Part *p, int N );

/* 
 * push 'data' onto the stack of participant 'p'.  
 */
#define partPush( p, data, len ) partStackPush( &(p).stack, data, len );

/* 
 * pop off and return the top element of p's stack.  return NULL if
 * there are no more elements
 */
#define partPop( p ) partStackPop( &(p).stack )

#define partLen( partPtr ) (partPtr->len)

xkern_return_t	partExternalize(
				Part *, VOID *, int *
				);

void partInternalize(
				Part *, VOID *
				);

#define partExtLen( _bufPtr )	( *(int *)(_bufPtr) )

int partStackTopByteLen(
			Part
			);


#define ANY_HOST	((VOID *) -1)
#define ANY_PORT	-1

#endif part_h
