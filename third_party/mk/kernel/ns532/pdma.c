/*
 * HISTORY
 * 20-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Added PDMA_VERSION 4. Essential difference: code to allow waiting
 *	for DRQ on the dp8490 chip.
 *
 * 17-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Use PDMA_VERSION instead of NEW/OLD_PDMA since it looks like
 *	being an on going saga finding something which works!
 *
 * 17-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Run pdma at ipl of 1 to stop softclock.
 *
 * 15-Sep-92  Ian Dall (ian) at University of Adelaide
 *	New PDMA code which copies in chunks (of block_size) with
 *	interrupts disabled.
 *
 * 14-Sep-92  Ian Dall (ian) at University of Adelaide
 *	Don't lower spl to see if it fixes an oddity.
 *
 * 28-Aug-92  Ian Dall (ian) at University of Adelaide
 *	Created.
 *
 */

/* This file contains code to do pseudo-dma for the PC532 scsi
 * devices.  Essentially, pseudo-dma involves doing a block copy to or
 * from a special region of memory. The hardware is organized so the
 * peripheral thinks it is talking to a DMA controller, but really it
 * is the CPU.
 *
 * The complications come from a) needing to be able to abort the
 * transfer in response to an interrupt, b) needing to be able to find
 * out how many bytes were transferred at the time of an interrupt and
 * c) avoiding side effects due to the chips pre-fetching of operands
 * from volatile memory.
 */
#ifdef MACH_KERNEL
#include <machine/thread.h>
#include <machine/machparam.h>
#include <ns532/pdma.h>
#include "mach_kdb.h"
#else
#include "pdma.h"
#endif

#if PDMA_VERSION == 4

/* We achieve c) by copying with interrupts disabled. We copy in
 * chunks and check between chunks to see if an interrupt has
 * happened.  The chunks are presumably block_size or smaller.  The
 * board logic allows us to keep reading (garbage) if an interrupt is
 * pending. So long as we can tell how many bytes really got
 * transferred we are still OK.  That is scsi chip dependent, if it
 * can be done at all, and the code isn't in this file. If the chip
 * has no on board counter (eg dp8490) we can still work so long as
 * the scsi target doesn't want to interrupt within a block. If it
 * does, too bad.  Disable disconnects for that target.
 *
 * When called from the dp8490 driver, done and cont will typically
 * point at the BSR register. Done mask will be for the INT bit
 * and cont_mask will be for DRQ bit. When reading DRQ true means
 * we really can continue. On writing, DRQ tends to be true immediately.
 * If we continue, we tend to over write by a whole block. This doesn't
 * go to the target and the dp driver fixes that up, but it is
 * more efficient to look for interupts for delay microseconds
 * before wasting time on a whole block of aborted writes.
 * On reads, delay will normally be zero.
 *
 * There is only one externally accessable entry points, pdma_xfer
 * which returns the number of bytes left (not transferred).
 */
int pdma_xfer(volatile char *from , volatile char * to, int count, int chunk_size,
	      volatile unsigned char *done, unsigned char done_mask,
	      volatile unsigned char *cont, unsigned char cont_mask, int timeout, int delay_count)
{
  while (count)
    {
      int i, s;
      int temp_count = count > chunk_size? chunk_size: count;
      for (i = delay_count; i > 0; i--)
	{
	  if(*done & done_mask)
	    {
	      return count;
	    }
	  delay(1);
	}

      for (i = timeout; i > 0; i--)
	{
	  if(*done & done_mask)
	    {
	      return count;
	    }
	  if(*cont & cont_mask)
	    break;
	  delay(1);
	}
      if(i == 0)
	panic("pdma: timeout");
      s = splhi();
      bcopy(from, to, temp_count);
      splx(s);
      count -= temp_count;
      from += temp_count;
      to += temp_count;
    }
  return count;
}
#elif PDMA_VERSION == 3

/* We achieve c) by copying with interrupts disabled. We copy in
 * chunks and check between chunks to see if an interrupt has
 * happened.  The chunks are presumably block_size or smaller.  The
 * board logic allows us to keep reading (garbage) if an interrupt is
 * pending. So long as we can tell how many bytes really got
 * transferred we are still OK.  That is scsi chip dependent, if it
 * can be done at all, and the code isn't in this file. If the chip
 * has no on board counter (eg dp8490) we can still work so long as
 * the scsi target doesn't want to interrupt within a block. If it
 * does, too bad.  Disable disconnects for that target.
 *
 * There is only one externally accessable entry points, pdma_xfer
 * which returns the number of bytes left (not transferred).
 */
int pdma_xfer(volatile char *from , volatile char * to, int count, int chunk_size,
	      volatile unsigned char *done, unsigned char done_mask, int delay_count)
{
  while (count)
    {
      int i, s;
      int temp_count = count > chunk_size? chunk_size: count;
      for (i = delay_count; i > 0; i--)
	{
	  if(*done & done_mask)
	    {
	      return count;
	    }
	  delay(1);
	}
      s = splhi();
      bcopy(from, to, temp_count);
      splx(s);
      count -= temp_count;
      from += temp_count;
      to += temp_count;
    }
  return count;
}

#elif PDMA_VERSION == 2

/* We achieve c) by copying in chunks with interrupts disabled. The
 * chunks are presumably block_size or smaller.  The board logic
 * allows us to keep reading (garbage) if an interrupt is pending. So
 * long as we can tell how many bytes really got transferred we are
 * still OK.  That is scsi chip dependent, if it can be done at all,
 * and the code isn't in this file. If the chip has no on board
 * counter (eg dp8490) we can still work so long as the scsi target
 * doesn't want to interrupt within a block. If it does, too bad.
 * Disable disconnects for that target.
 *
 * There are only two externally accessable entry points, pdma_xfer
 * and pdma_xfer_abort. The latter returns the number of bytes left
 * (not transferred).
 */

/* This could be simpler except we try and make it as much like real
 * DMA as possible. This makes it easier for the drivers code to be
 * ported from machine to machine.
 */
void pdma_xfer(volatile char *from, volatile char *to, int count,
	       int chunk_size, volatile unsigned *waitfor,
	       unsigned waitfor_mask, struct pdma_state *state)
{
  int s;
  volatile int *done = &(state->done);
  volatile int *mv = &(state->m);
  *done = 0;
  s = spldroptosoftclock();
  while (count)
    {
      int s;
      int temp_count = count > chunk_size? chunk_size: count;
      s = splhi();
      if (*done)
	{
	  splx(s);
	  break;
	}
      bcopy(from, to, temp_count);
      count -= temp_count;
      *mv = count;
      splx(s);
      from += temp_count;
      to += temp_count;
    }
  while(!(*waitfor & waitfor_mask))
    /* Wait for interrupt */;
  splx(s);
}

int pdma_xfer_abort(int count, struct pdma_state *state, struct ns532_saved_state *int_regs)
{
  state->done = 1;
  return state->m;
}

#elif PDMA_VERSION == 1
/* We use asm's so that only simple, non-interrruptable instructions
 * are used to access the dma buffer and that each such instruction is
 * labelled. When an interrupt occurs, we look through a table of
 * these instruction labels and compare them with the saved program
 * counter. This, combined with some extra state information kept in
 * the pdma_state structure, allows us to figure out just how much of
 * the transfer has been done. Yuck!
 *
 * All the really ugly bits are hidden within this file. There are
 * only two externally accessable entry points, pdma_xfer and
 * pdma_xfer_abort. The latter returns the number of bytes left (not
 * transferred).
 */
/* UNROLL_BY is the amount to unroll by. Values of 4, 8, 16 or 32 are
 * supported.
 */
#define UNROLL_BY 32		/* Guess */

/* The normal use of this file in in the MACH_KERNEL.
 * However, if you compile without MACH_KERNEL, you can
 * produce a test program for optimising the amount
 * of unrolling (say).
 */

int pdma_debug = 1;
#define DP(level, action) do { if ((level) <= pdma_debug) action; } while(0)

#define PDMA_RESET 0
#define PDMA_COPYING 1
#define PDMA_DONE 2

#ifndef __GNUC__
/* We use gcc extended asms. We also use gcc's &&label to get the address
 * corresponding to a C label. The latter is only in gcc2.
 */
#error This file uses gcc extensions. Get gcc.
#endif

#ifdef PC532
#define movb(from, to, label, offset) asm("l" #label ": movb %1, %0" : "=g" (*(to + offset)): "g" (*(from + offset)))
#define dec(m, label) asm("l" #label ": addqd -1, %0" : "=g" (m): "0" (m))
#if defined(MACH_KERNEL) || !defined(CHECK_RESID)
#define RESID
#else
#define RESID asm("addr 0(pc), %0": "=g" (pc)); resid(n, pc, state)
#endif
#else
#define movb(from, to, label, offset) asm ("l" #label ":"); *(to + offset) = *(from + offset)
#define dec(m, label) asm("l" #label ":"); (m)--
#ifndef CHECK_RESID
#define RESID
#else
#define RESID asm("pea pc@(0); movel sp@+,%0": "=g" (pc)); resid(n, pc, state)
#endif
#endif

/* switch_tbl isn't really external, it is defined in this file, but
 * inside of an asm (so the compiler doesn't know about it).
 */
extern void *switch_tbl[];

/* Change by Hand */
asm ("_switch_tbl:");
#if UNROLL_BY >= 32
asm ("
\t.long l32
\t.long l31
\t.long l30
\t.long l29
\t.long l28
\t.long l27
\t.long l26
\t.long l25
\t.long l24
\t.long l23
\t.long l22
\t.long l21
\t.long l20
\t.long l19
\t.long l18
\t.long l17");
#endif
#if UNROLL_BY >= 16
asm ("
\t.long l16
\t.long l15
\t.long l14
\t.long l13
\t.long l12
\t.long l11
\t.long l10
\t.long l9");
#endif
#if UNROLL_BY >= 8
asm ("
\t.long l8
\t.long l7
\t.long l6
\t.long l5");
#endif
asm ("
\t.long l4
\t.long l3
\t.long l2
\t.long l1
\t.long l0");

static resid(int n, void *pc, struct pdma_state *state)
{
  int i;
  int resid;
  int m = state->m;

  switch (state->phase)
    {
    case PDMA_RESET:
      resid = n;
      DP(1, printf("pdma reset: resid = %d\n", resid));
      break;
    case PDMA_COPYING:
      for(i = 0; i < UNROLL_BY; i++)
	{
	  if (pc > switch_tbl[i] && pc <= switch_tbl[i + 1])
	    break;
	}
      if (i == UNROLL_BY)
	if (m > 0)
	  resid = m * UNROLL_BY + (n & (UNROLL_BY - 1));
	else if ( m == 0)
	  resid = (n & (UNROLL_BY - 1));
	else
	  resid = 0;
      else
	resid = m > 0? m * UNROLL_BY - i - 1 + (n & (UNROLL_BY - 1)) : UNROLL_BY - i - 1;
      DP(1, printf("pdma copying: i = %d, m = %d, resid = %d\n", i, m, resid));
      break;
    case PDMA_DONE:
      resid = 0;
      DP(1, printf("pdma done: resid = %d\n", resid));
      break;
    }
  return resid;
}
      

static bcopy_bytes(volatile char *from, volatile char *to, int n, struct pdma_state *state)
{

  void *pc;
  int  r = n & (UNROLL_BY - 1);
  int *m = &(state->m);
  volatile int *mv = m;		/* Volatile version of m */
  volatile int *phase = &(state->phase);
  RESID;
  *mv = n/UNROLL_BY;
#ifdef MACH_KERNEL
  state->done_label = &&done;	/* The abort routine will jump this label */
#endif
  *phase = PDMA_COPYING;

  /* state->done is set by the abort routine if the abort happens
   * before PDMA_COPYING
   */
  if (state->done)
    goto done;
  RESID;
  while(*m >= 0)
    {
      if ( *m == 0)
	{
	  /* Yes this is not ansii compliant, but it is the least of the 
	   * sins we commit against the ansii standard!
	   */
	  to -= (UNROLL_BY - r);
	  from -= (UNROLL_BY - r);
	}

      RESID;
      switch (*m == 0? r: UNROLL_BY)
	{
	  for( ;*m > 0; to += UNROLL_BY, from += UNROLL_BY)
	    {
#define STEP(n) case n: movb(from, to, n, UNROLL_BY - n);
#define DEC(m,label) case label: dec(m, label)
	      /* Change by Hand */
#if UNROLL_BY >= 32
	      RESID;
	      STEP(32);
	      RESID;
	      STEP(31);
	      RESID;
	      STEP(30);
	      RESID;
	      STEP(29);
	      RESID;
	      STEP(28);
	      RESID;
	      STEP(27);
	      RESID;
	      STEP(26);
	      RESID;
	      STEP(25);
	      RESID;
	      STEP(24);
	      RESID;
	      STEP(23);
	      RESID;
	      STEP(22);
	      RESID;
	      STEP(21);
	      RESID;
	      STEP(20);
	      RESID;
	      STEP(19);
	      RESID;
	      STEP(18);
	      RESID;
	      STEP(17);
#endif
#if UNROLL_BY >= 16
	      RESID;
	      STEP(16);
	      RESID;
	      STEP(15);
	      RESID;
	      STEP(14);
	      RESID;
	      STEP(13);
	      RESID;
	      STEP(12);
	      RESID;
	      STEP(11);
	      RESID;
	      STEP(10);
	      RESID;
	      STEP(9);
#endif
#if UNROLL_BY >= 8
	      RESID;
	      STEP(8);
	      RESID;
	      STEP(7);
	      RESID;
	      STEP(6);
	      RESID;
	      STEP(5);
#endif
	      RESID;
	      STEP(4);
	      RESID;
	      STEP(3);
	      RESID;
	      STEP(2);
	      RESID;
	      STEP(1);
	      RESID;
	      DEC(*mv, 0);
	      RESID;
	    }
	}
    }
  RESID;
  *phase = PDMA_DONE;
  RESID;
 done:
  return;
}

#if MACH_KERNEL
void pdma_xfer(volatile char *from, volatile char *to, int count, volatile unsigned *waitfor, unsigned waitfor_mask, struct pdma_state *state)
{
  int s;
  state->phase = PDMA_RESET;
  state->done = 0;
  s = spl0();
  bcopy_bytes (from, to, count, state);
  while(!(*waitfor & waitfor_mask))
    /* Wait for interrupt */;
  splx(s);
}

int pdma_xfer_abort(int count, struct pdma_state *state, struct ns532_saved_state *int_regs)
{
  int r;
  r = resid(count, (void *) int_regs->pc, state);
  if (state->phase == PDMA_COPYING)
    int_regs->pc = (int) state->done_label; /* Mung interrupt return PC. Yuck! */
  else if(state->phase == PDMA_RESET)
    state->done = 1;		/* Not really started yet. Cause early exit */
  /* else must be already done */
  return r;
}

#if MACH_KDB
/* This can be called from kernel debugger */
pdma_xfer_test(int n)
{
  struct pdma_state state = { 0, PDMA_RESET};
  char s[] = "the quick brown fox";
  char b[] = "xxxxxxxxxxxxxxxxxxxxxxx";
  n = n > (sizeof(b) - 1)? (sizeof(b) - 1): n;
  bcopy_bytes(s, b, n, &state);
  printf("Copy %d bytes from \"%s\" to \"%s\"\n", n, s, b);
}
#endif

#else
#include <sys/time.h>
#include <sys/resource.h>
#define MBYTES (10)
main()
{
  int n;
  struct pdma_state state = { 0, PDMA_RESET};
  struct rusage rusage;
  char s[8192];
  char b[8192];
  int i;
  for (i = 0; i < 128 * MBYTES; i++)
    bcopy_bytes(s, b, 8192, &state);
  getrusage(RUSAGE_SELF, &rusage);
  printf("Through put = %f MB/s\n", MBYTES/((float) rusage.ru_utime.tv_sec + ((float) rusage.ru_utime.tv_usec)/ 1e6));
}

#endif
#endif
