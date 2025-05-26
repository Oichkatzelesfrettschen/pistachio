/* 
 * $RCSfile: xmed_util.c,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1991  Arizona Board of Regents
 *
 * Utility functions for x-kernel driver for the Medusa FDDI board.
 *
 */
/*
 * HISTORY
 * $Log: xmed_util.c,v $
 * Revision 1.6  1993/02/21  04:06:01  davidm
 * Corrected comment.
 *
 * Revision 1.5  1993/02/18  22:19:01  davidm
 * The copy-in routine now takes care of writing exactly as much bytes as
 * requests (before, it could have written up to 3 additional bytes).
 *
 * Revision 1.4  1992/12/01  23:30:20  davidm
 * Contains a test routine to check the operation of VENOM.
 *
 * Revision 1.3  1992/11/26  00:17:20  davidm
 * A version with Venom copy routines that handle non-aligned
 * (non-cache-line aligned) data transfer.  Unfortunately, they
 * don't seem to work.
 *
 * Revision 1.2  1992/11/18  04:21:19  davidm
 * This is the first **WORKING** version.  No elaborate handling of
 * rx and tx buffers, but it's working after all!
 *
 * Revision 1.1  1992/11/07  07:37:33  davidm
 * Initial revision
 *
 */

/* get number of configured interfaces: */
#include "xmed.h"

#if NXMED > 0

#include "xmed_i.h"
#include "xmed_util.h"
/*
 * Get system stuff:
 */
#include <sys/types.h>

/*
 * Get x-kernel stuff:
 */
#include "xkernel.h"
#include "eth.h"	/* pretend to be an ethernet device for now... */
#include "eth_i.h"
#include "msg.h"
#include "msg_internal.h"
#include "trace.h"

#define SGCSLOT_BASEADDR_MASK   (0x0c000000L)

extern int tracexmedp;

static volatile u_int *venom_user_control	= (u_int*) 0xfffbc080;
static volatile u_int *venom_span_count		= (u_int*) 0xfffbc084;
static volatile u_int *venom_graphics_address	= (u_int*) 0xfffbc088;
static volatile u_int *venom_sup_control	= (u_int*) 0xfffbd080;

/* cache size in # of bytes: */
#define CACHE_LINE_SIZE		(32)
#define CACHE_LINE_OFF(a)	((u_int)(a)&(CACHE_LINE_SIZE-1))

/*
 * Macros to produce Graphics Flush Read and Graphics Flush Write
 * instructions.  Unfortunately, the assembler doesn't recognize
 * these instructions, so we have to hand-craft their codes...
 */
#ifdef AS_WOULD_SUPPORT_GFR_AND_GFW_AND_GCC_SUPPORTED_PLUS_MODIFIERS

#define GFR(a,i) \
	asm volatile("gfr,m %0(%1)" : "+r"(a) : "r"(i));
#define GFW(a,i) \
	asm volatile("gfw,m %1(%0)" : "+r"(a) : "r"(i));

#else /* AS_WOULD_SUPPORT_GFR_AND_GFW_AND_GCC_SUPPORTED_PLUS_MODIFIERS */

#define GFR(a,i) \
	asm volatile(".word 0x04001aa0+%1*0x10000+%0*0x200000+0*%2" \
		     :"=r"(a):"r"(i),"0"(a));
#define GFW(a,i) \
	asm volatile(".word 0x040016a0+%1*0x10000+%0*0x200000+0*%2" \
		     :"=r"(a):"r"(i),"0"(a));

#endif /* AS_WOULD_SUPPORT_GFR_AND_GFW_AND_GCC_SUPPORTED_PLUS_MODIFIERS */

/*
 * Copy N bytes from SRC to DST.  N must be a multiple of 4,
 * SRC must be a word-aligned, valid graphics card address (i.e., in
 * the range 0xf4000000-0xfbffffff). DST must be word-aligned.
 */
static __inline__ void
venom_copy_out(u_int *src, u_int *dst, int n)
{
    int inc;

    xTrace3(xmedp, TR_FUNCTIONAL_TRACE, "venom_copy_out(src=%x,dst=%x,n=%d)",
	    src, dst, n);

    *venom_user_control = 0;		/* selects block move */
    /* set GA[3] and GA[4] according to slot address: */
    *venom_sup_control = (u_int) dst & SGCSLOT_BASEADDR_MASK;
    *venom_graphics_address = (u_int) dst;	/* set dst address */
    *venom_span_count = n/4 - 1;		/* # of words to move - 1 */

    if (CACHE_LINE_OFF(src)) {
	/* have to copy a partial cache line first: */
	inc = CACHE_LINE_SIZE - CACHE_LINE_OFF(src);
	GFR(src,inc);
	n -= inc;
    } /* if */

    /* copy cache-line sized blocks: */
    inc = CACHE_LINE_SIZE;
    while (n > 0) {
	/* flush it to IO space: */
	GFR(src,inc);
	n -= inc;
    } /* while */
} /* venom_copy_out */


/*
 * Copy the buffer of length LEN, starting at address SRC to the
 * address STATE->next_word (only the last STATE->remaining_space
 * bytes in the first word are free, the other bytes are taken from
 * STATE->partial_word).  Only locations in the range SRC..SRC+LEN-1
 * are accessed.  Furthermore, it is guaranteed that each word
 * in the destination range is written exactly one.
 */
bool
xmed_copy_out(src, len, state)
     u_char *src;
     long len;
     xmed_copy_state_t *state;
{
    register u_int t1, t2, t3;
    register u_int *pkt;
    register u_int *wp;
    int wlen;
    int rlen;

    xTrace2(xmedp, TR_FUNCTIONAL_TRACE,
	    "xmed_copy_out(src=0x%08x, len=%d)", src, len);

    if (!len) {
	return TRUE;
    } /* if */

    pkt = state->next_word;

    if (state->remaining_space) {

	/* previous segment didn't fill last word; fill it now: */

	t1 = state->partial_word;
	if (len < state->remaining_space) {

	    /* fill as much as we have: */

	    switch (state->remaining_space) {
	      case 3:
		t1 = t1 << 8 | *src++;
		--state->remaining_space;
		--len;
	      case 2:
		if (!len) break;
		t1 = t1 << 8 | *src++;
		--state->remaining_space;
		--len;
	      case 1:
		if (!len) break;
		t1 = t1 << 8 | *src++;
		--state->remaining_space;
	    } /* switch */
	    state->partial_word = t1;

	    /* no more to do: */

	    return TRUE;
	} else {
	    /* fill word and put it into packet: */
	    switch (state->remaining_space) {
	      case 3: t1 = t1 << 8 | *src++;
	      case 2: t1 = t1 << 8 | *src++;
	      case 1: t1 = t1 << 8 | *src++;
	    } /* switch */
	    *pkt++ = t1;
	    len -= state->remaining_space;
	} /* if */
    } /* if */

    /* invariant: pkt points to the first totally free word in the packet */

    /* # of full words we can copy and # of remaining bytes: */
    wlen = len / sizeof(int);
    rlen = len & (sizeof(int) - 1);

    /* handle alignment of src: */
    switch ((u_int)src & 3) {
      case 0:
#ifdef 0
	/*
	 * We cannot use Venom for copying data in *and* out, as we
	 * don't know whether there is a way to save Venom's state
	 * (and this routine is called from an interrupt handler).
	 * So for now, we just use Venom for copying data in, which
	 * is where we win big anyway.
	 */
	venom_copy_out((u_int*)src, pkt, wlen * sizeof(int));
	src += wlen * sizeof(int);
	pkt += wlen;
	wlen = 0;
#endif
	/*
	 * Unrolling the loop twice is optimal if going from memory
	 * to IO:
	 */
	wp = (u_int*) src;
	while (wlen >= 2) {
	    t1 = *wp++; t2 = *wp++;
	    *pkt++ = t1; wlen -= 2; *pkt++ = t2;
	} /* while */
	while (wlen-- > 0) {
	    *pkt++ = *wp++;
	} /* while */
	src = (u_char*) wp;
	break;

/*
 * Define a macro to combine the r least significant bytes of t1 with
 * the 4-r most significant bytes of t2 to form a single word.  If we
 * are using GNUC, we can use the "shift double" instruction.
 */
#if defined(__GNUC__)

# define COMB(t1,t2,s) \
	{asm("shd %1,%2,%3,%0":"=r"(*pkt++):"r"(t1),"r"(t2),"i"(8*(4-s)));}
#else
# define COMB(t1,t2,s) \
	{*pkt++ = t1<<8*s | t2>>8*(4-s);}
#endif /* __GNUC__ */

#define CASE(r) \
 wp = (u_int*) (src + 4 - r); \
 while (wlen >= 4) { \
   t2 = *wp++; t3 = *wp++; \
   COMB(t1,t2,r); COMB(t2,t3,r); \
   t2 = *wp++; t1 = *wp++; \
   COMB(t3,t2,r); wlen -= 4; COMB(t2,t1,r); \
 } /* while */ \
 while (wlen >= 2) { \
   t2 = *wp++; t3 = *wp++; \
   COMB(t1,t2,r); wlen -= 2; COMB(t2,t3,r); t1 = t3; \
 } /* while */ \
 while (wlen > 0) { \
   t2 = *wp++; COMB(t1,t2,r); --wlen; t1 = t2; \
 } /* while */ \
 src = (u_char*) wp - (4-r); \
 break;

      case 3:
	t1 = *src;
	CASE(3);
      case 2:
	t1 = *(u_short*)src;
	CASE(2);
      case 1:
	t1 = *src << 16 | *(u_short*)(src+1);
	CASE(1);

#undef CASE
    } /* switch */

    if (rlen) {
	/* some bytes are remaining, store them for later use: */
	t1 = 0;
	switch (rlen) {
	  case 3: t1 = *src++;
	  case 2: t1 = t1 << 8 | *src++;
	  case 1: t1 = t1 << 8 | *src++;
	} /* switch */
	state->remaining_space = 4 - rlen;
	state->partial_word = t1;
    } else {
	state->remaining_space = 0;
    } /* if */

    state->next_word = pkt;

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_copy_out: returning");

    return TRUE;
} /* xmed_copy_out */


/*
 * Copy N bytes from SRC to DST.  N must be a multiple of 4,
 * SRC must be a word-aligned, valid graphics card address
 * (i.e., in the range 0xf4000000-0xfbffffff), and DST must
 * be word-aligned.
 */
static __inline__ void
venom_copy_in(u_int *src, u_int *dst, int n)
{
    int inc;

    xTrace3(xmedp, TR_FUNCTIONAL_TRACE, "venom_copy_in(src=%x,dst=%x,n=%d)",
	    src, dst, n);

    *venom_user_control = 0;		/* selects block move */
    /* set GA[3] and GA[4] according to slot address: */
    *venom_sup_control = (u_int) src & SGCSLOT_BASEADDR_MASK;
    *venom_graphics_address = (u_int) src;	/* set src address */
    *venom_span_count = n/4 - 1;		/* # of words to move - 1 */

#if SIMPLE_VERSION
    if (CACHE_LINE_OFF(dst)) {
	/* have to copy a partial cache line first: */
	inc = CACHE_LINE_SIZE - CACHE_LINE_OFF(dst);
	GFW(dst,inc);
	n -= inc;
    } /* if */

    /* copy cache-line sized blocks: */
    inc = CACHE_LINE_SIZE;
    while (n > 0) {
	GFW(dst,inc);
	n -= inc;
    } /* while */
#else
    if (CACHE_LINE_OFF(dst)) {
	/* have to copy a partial cache line first: */
	inc = CACHE_LINE_SIZE - CACHE_LINE_OFF(dst);
	if (inc > n) {
	    inc = n;
	} /* if */
	GFW(dst,inc);
	n -= inc;
    } /* if */

    /* copy cache-line sized blocks: */
    inc = CACHE_LINE_SIZE;
    while (n > inc) {
	GFW(dst,inc);
	n -= inc;
    } /* while */

    /* copy last chunk: */
    if (n > 0) {
	inc = n;
	GFW(dst,inc);
    } /* if */
#endif /* SIMPLE_VERSION */
} /* venom_copy_in */


void
xmed_copy_in(u_int *src, long len, ETHhdr *hp, u_int *dst)
{
    u_int t1, t2, t3, t4, wlen;

    xTrace4(xmedp, TR_FUNCTIONAL_TRACE,
	    "xmed_copy_in(src=%x, len=%d, hp=%x, dst=%x)",
	    src, len, hp, dst);

    /* we are prepared to deal with aligned pointers only: */
    if (((u_int)hp & 3) || ((u_int)dst & 3)) {
	if ((u_int)hp & 3) {
	    xError("xmed_copy_in: hp not aligned!");
	} else {
	    xError("xmed_copy_in: dst not aligned!");
	} /* if */
	Kabort("bad alignment");
    } /* if */

    /* skip FDDI header: */
    src++;

    /* load MAC header and Ethernet type (aka, length): */
    t1 = *src++; t2 = *src++; t3 = *src++; t4 = *src++;
    hp->dst.high = t1 >> 16;
    hp->dst.mid  = t1 & 0xffff;
    hp->dst.low  = t2 >> 16;
    hp->src.high = t2 & 0xffff;
    hp->src.mid  = t3 >> 16;
    hp->src.low  = t3 & 0xffff;
    hp->type = t4;

#ifdef XMED_USE_VENOM
    wlen = len & ~0x3;
    if (wlen) {
	venom_copy_in(src, dst, wlen);
	len -= wlen;
	if (len) {
	    src += wlen / sizeof(u_int);
	    dst += wlen / sizeof(u_int);
	} /* if */
    } /* if */
#else /* XMED_USE_VENOM */
    /* unrolling the loop 4 times is optimal if going from IO to memory: */
    while (len >= 16) {
	t1 = *src++; t2 = *src++; t3 = *src++; t4 = *src++;
	*dst++ = t1; *dst++ = t2;
	len -= 16;
	*dst++ = t3; *dst++ = t4;
    } /* while */
    while (len >= 4) {
	*dst++ = *src++;
	len -= 4;
    } /* while */
#endif /* XMED_USE_VENOM */
    if (len) {
	/* copy the up to three last bytes: */
	t1 = *src++;
	switch (len) {
	  case 1:
	    *(u_char*)dst  = (t1 >> 24) & 0xff;
	    break;
	  case 2:
	    *(u_short*)dst = (t1 >> 16) & 0xffff;
	    break;
	  case 3:
	    *(u_short*)dst =    (t1 >> 16) & 0xffff;
	    *((u_char*)dst+3) = (t1 >> 8)  & 0xff;
	    break;
	} /* switch */
    } /* if */

    xTrace0(xmedp, TR_FUNCTIONAL_TRACE, "xmed_copy_in: returning");
} /* xmed_copy_in */


#include <machine/spl.h>

void
xmed_util_test(xmed_board_t *m)
{
#   define SZ	(8*1024)
    u_char *buf;
    u_char *p;
    u_int go1, go2, stop1, stop2, chk1, chk2;
    int i, j;
    xmed_copy_state_t state;
    u_int old_mask;
#   define INT_DISABLE {old_mask = splhigh();}
#   define INT_ENABLE {splx(old_mask);}

    buf = (u_char*) malloc(SZ+32);
    bzero((char*)buf, SZ+32);

    /* make buffer cacheline aligned: */
    while ((u_int) buf & 0x1f) {
	buf += 4;
    } /* buf */

    for (i = 0; i < SZ; i++) {
	buf[i] = i;
    } /* for */

    printf("\nTiming copying of %dKBytes of data(buf=%x,vram=%x):\n\n",
	   SZ/1024, buf, &m->vram_chk);

    for (i = 0; i < 5; i++) {
	INT_DISABLE;
	state.next_word = (u_int*) m->vram_chk;
	state.remaining_space = 0;
	fdcacheall();
	m->chk_sum = 0;
	go1 = read_itimer();
	xmed_copy_out(buf, SZ, &state);
	stop1 = read_itimer();
	chk1 = m->chk_sum;

	state.next_word = (u_int*) m->vram_chk;
	state.remaining_space = 0;
	m->chk_sum = 0;
	go2 = read_itimer();
	xmed_copy_out(buf, SZ, &state);
	stop2 = read_itimer();
	chk2 = m->chk_sum;
	INT_ENABLE;

	printf(
	"copy out, cold cache: %d cyc (chk=%x)\twarm cache: %d cyc (chk=%x)\n",
	       stop1-go1, chk1, stop2-go2, chk2);

	p = (u_char*) m->vram_chk;
	for (j=0; j<SZ; j++, p++) {
	    if (*p != (j & 0xff)) {
		printf("%x=%d (expected %d)\n", p, *p, j & 0xff);
	    } /* if */
	} /* for */

	fdcacheall();
	INT_DISABLE;
	m->chk_sum = 0;
	go1 = read_itimer();
	xmed_copy_in((u_int*) m->vram_chk, SZ-16, (ETHhdr*) &buf[4],
		     (u_int*)&buf[20]);
	stop1 = read_itimer();
	chk1 = m->chk_sum;

	m->chk_sum = 0;
	go2 = read_itimer();
	xmed_copy_in((u_int*) m->vram_chk, SZ-16, (ETHhdr*) &buf[4],
		     (u_int*)&buf[20]);
	stop2 = read_itimer();
	chk2 = m->chk_sum;
	INT_ENABLE;

	printf(
	 "copy in, cold cache: %d cyc (chk=%x)\twarm cache: %d cyc (chk=%x)\n",
	       stop1-go1, chk1, stop2-go2, chk2);

	p = buf;
	for (j=0; j<SZ; j++, p++) {
	    if (*p != (j & 0xff)) {
		printf("%x=%d (expected %d)\n", p, *p, j & 0xff);
	    } /* if */
	} /* for */
    } /* for */
} /* xmed_util_test */

#endif /* NXMED > 0 */

			/*** end of xmed_util.c ***/
