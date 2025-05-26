#include <stdio.h>
#include <sys/types.h>

static u_int ones = 0xffffffff;

typedef enum {FALSE, TRUE} bool;

typedef struct {
    u_int	*next_word;		/* next word to be written */
    int		remaining_space;	/* # of free bytes in partial word */
    u_int	partial_word;		/* partial word that's written next */
} xmed_copy_state_t;


void
write_word(u_int *addr, u_int val)
{
    printf("%08x <- %08x %s\n", addr, val,
	   (u_int)addr & 3 ? "[Segmentation Fault]" : "");
} /* write_word */


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
fprintf(stderr, "a\n");
		t1 = t1 << 8 | *src++;
		--state->remaining_space;
		--len;
	      case 2:
fprintf(stderr, "b\n");
		if (!len) break;
		t1 = t1 << 8 | *src++;
		--state->remaining_space;
		--len;
	      case 1:
fprintf(stderr, "c\n");
		if (!len) break;
		t1 = t1 << 8 | *src++;
		--state->remaining_space;
	    } /* switch */
	    state->partial_word = t1;

	    /* nothing more to do: */

	    return TRUE;
	} else {
	    /* fill word and put it into packet: */
	    switch (state->remaining_space) {
	      case 3: t1 = t1 << 8 | *src++;
fprintf(stderr, "d\n");
	      case 2: t1 = t1 << 8 | *src++;
fprintf(stderr, "e\n");
	      case 1: t1 = t1 << 8 | *src++;
fprintf(stderr, "f\n");
	    } /* switch */
	    write_word(pkt++, t1);
	    len -= state->remaining_space;
	} /* if */
    } /* if */

    /* invariant: pkt points to the first free word in the packet */

    /* # of full words we can copy and # of remaining bytes: */
    wlen = len >> 2;
    rlen = len & 3;

    /* handle alignment of src: */
    switch ((u_int)src & 3) {
      case 0:
	wp = (u_int*) src;
	while (wlen >= 4) {
fprintf(stderr, "g\n");
	    /* unrolled copy loop: */
	    t1 = *wp++; t2 = *wp++; write_word(pkt++,t1); write_word(pkt++,t2);
	    t1 = *wp++; t2 = *wp++; write_word(pkt++,t1); write_word(pkt++,t2);
	    wlen -= 4;
	} /* while */
	while (wlen-- > 0) {
fprintf(stderr, "h\n");
	    write_word(pkt++, *wp++);
	} /* while */
	src = (u_char*) wp;
	break;

#define CASE(r) \
 wp = (u_int*) (src + 4 - r); \
 while (wlen >= 4) { \
fprintf(stderr, "i"#r"\n"); \
   t2 = *wp++; t3 = *wp++; write_word(pkt++, t1<<8*r | t2>>8*(4-r)); \
   write_word(pkt++, t2<<8*r | t3>>8*(4-r)); \
   t2 = *wp++; t1 = *wp++; write_word(pkt++, t3<<8*r | t2>>8*(4-r)); \
   write_word(pkt++, t2<<8*r | t1>>8*(4-r)); \
   wlen -= 4; \
 } /* while */ \
 while (wlen-- > 0) { \
fprintf(stderr, "j"#r"\n"); \
   t2 = *wp++; \
   write_word(pkt++, t1 << 8*r | t2 >> 8*(4-r)); \
   t1 = t2; \
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
fprintf(stderr, "k\n");
	  case 2: t1 = t1 << 8 | *src++;
fprintf(stderr, "l\n");
	  case 1: t1 = t1 << 8 | *src++;
fprintf(stderr, "m\n");
	} /* switch */
	state->remaining_space = 4 - rlen;
	state->partial_word = t1;
    } else {
fprintf(stderr, "n\n");
	state->remaining_space = 0;
    } /* if */

    state->next_word = pkt;

    return TRUE;
} /* xmed_copy_out */


void
main()
{
    u_char buf[1024];
    int i;
    xmed_copy_state_t state;

    for (i = 0; i < 1024; i++) {
	buf[i] = i;
    } /* for */

    state.next_word = (u_int*) 0xf4000000;
    state.remaining_space = 0;

    printf("*** buf[0..31]:\n");
    xmed_copy_out(&buf[0], 32, &state);

    printf("*** buf[32]:\n");
    xmed_copy_out(&buf[32], 1, &state);

    printf("*** buf[33..34]:\n");
    xmed_copy_out(&buf[33], 2, &state);

    printf("*** buf[35..37]:\n");
    xmed_copy_out(&buf[35], 3, &state);

    printf("*** buf[38..41]:\n");
    xmed_copy_out(&buf[38], 4, &state);

    printf("*** buf[42..50]:\n");
    xmed_copy_out(&buf[42], 9, &state);

    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out(&ones, state.remaining_space, &state);

    printf("*** buf[1..11]:\n");
    xmed_copy_out(&buf[1], 11, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out(&ones, state.remaining_space, &state);

    printf("*** buf[2..11]:\n");
    xmed_copy_out(&buf[2], 10, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out(&ones, state.remaining_space, &state);

    printf("*** buf[3..11]:\n");
    xmed_copy_out(&buf[3], 9, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out(&ones, state.remaining_space, &state);

    printf("*** buf[3..31]:\n");
    xmed_copy_out(&buf[3], 29, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out(&ones, state.remaining_space, &state);

} /* main */
