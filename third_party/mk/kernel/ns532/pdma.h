/*
 * HISTORY
 * 20-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Added PDMA_VERSION 4.
 *
 * 17-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Use PDMA_VERSION instead of NEW/OLD_PDMA since it looks like
 *	being an on going saga finding something which works!
 *
 * 15-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Modify for new PDMA. Get old code by defining OLD_PDMA.
 *
 * 28-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Created.
 *
 */
/* This file contains definitions needed to interface with the pseudo-dma
 * functions
 */
#ifndef _PDMA_H_
#define _PDMA_H_
#define PDMA_VERSION (4)

#if PDMA_VERSION == 1 || PDMA_VERSION == 2
struct pdma_state
{
  int m;
#if PDMA_VERSION == 1
  int phase;
  void *done_label;
#endif
  int done;
};
#endif

#ifdef MACH_KERNEL
#if PDMA_VERSION == 4
int pdma_xfer(volatile char *, volatile char *, int, int,
	      volatile unsigned char *, unsigned char,
	      volatile unsigned char *, unsigned char, int, int);
#elif PDMA_VERSION == 3
int pdma_xfer(volatile char *, volatile char *, int, int,
	      volatile unsigned char *, unsigned char, int);
#elif PDMA_VERSION == 2
void pdma_xfer(volatile char *, volatile char *, int, int,
	       volatile unsigned *, unsigned, struct pdma_state *);
int pdma_xfer_abort(int, struct pdma_state *, struct ns532_saved_state *);
#elif PDMA_VERSION == 1
void pdma_xfer(volatile char *, volatile char *, int,
	       volatile unsigned *, unsigned, struct pdma_state *);
int pdma_xfer_abort(int, struct pdma_state *, struct ns532_saved_state *);
#endif
#endif

#endif _PDMA_H_
