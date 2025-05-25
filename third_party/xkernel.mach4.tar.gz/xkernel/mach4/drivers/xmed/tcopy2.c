#include "xmed_util.h"

#define TICKS_PER_MSEC	50000

static u_int ones = 0xffffffff;

int tracexmedp = 100;
void xTraceLock() {};
void xTraceUnlock() {};


int
panic(char* msg)
{
    fprintf("panic: %s\n", msg);
    exit(1);
} /* panic */


static void
dump_buf(u_int *start, u_int *end)
{
    while (start < end) {
	printf("%08x = %08x\n", start, *start++);
    } /* while */
} /* dump_buf */


void
main()
{
#   define SZ	(128*1024)
    u_char buf[SZ+sizeof(int)];
    u_int dst[SZ/sizeof(int)];
    u_int *start;
    u_int go, stop, time;
    int i;
    xmed_copy_state_t state;

#if 1
    for (i = 0; i < 1024; i++) {
	buf[i] = i;
    } /* for */

    start = dst;
    state.next_word = (u_int*) dst;
    state.remaining_space = 0;

    printf("*** buf[0..31]:\n");
    xmed_copy_out(&buf[0], 32, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[32]:\n");
    xmed_copy_out(&buf[32], 1, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[33..34]:\n");
    xmed_copy_out(&buf[33], 2, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[35..37]:\n");
    xmed_copy_out(&buf[35], 3, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[38..41]:\n");
    xmed_copy_out(&buf[38], 4, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[42..50]:\n");
    xmed_copy_out(&buf[42], 9, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out((u_char*)&ones, state.remaining_space, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[1..11]:\n");
    xmed_copy_out(&buf[1], 11, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out((u_char*)&ones, state.remaining_space, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[2..11]:\n");
    xmed_copy_out(&buf[2], 10, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out((u_char*)&ones, state.remaining_space, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[3..11]:\n");
    xmed_copy_out(&buf[3], 9, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out((u_char*)&ones, state.remaining_space, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("\ntesting hard cases...\n\n");

    printf("*** buf[1..35]:\n");
    xmed_copy_out(&buf[1], 35, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out((u_char*)&ones, state.remaining_space, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[2..35]:\n");
    xmed_copy_out(&buf[2], 34, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out((u_char*)&ones, state.remaining_space, &state);
    dump_buf(start, state.next_word); start = state.next_word;

    printf("*** buf[3..35]:\n");
    xmed_copy_out(&buf[3], 33, &state);
    printf("*** remaining_space=%d\n", state.remaining_space);
    xmed_copy_out((u_char*)&ones, state.remaining_space, &state);
    dump_buf(start, state.next_word); start = state.next_word;

#else

    printf("\ntiming copying of %dKBytes of data:\n\n", SZ/1024);

    go = read_itimer();
    bcopy(buf, dst, SZ);
    stop = read_itimer();
    time = stop - go;
    printf("bcopy: %d cycles\n", time);

    state.next_word = dst;
    state.remaining_space = 0;
    go = read_itimer();
    xmed_copy_out(&buf[0], SZ, &state);
    stop = read_itimer();
    time = stop - go;
    printf("xmed_copy_out: %d cycles\n", time);

    go = read_itimer();
    bcopy(buf, dst, SZ);
    stop = read_itimer();
    time = stop - go;
    printf("bcopy: %d cycles\n", time);

    state.next_word = dst;
    state.remaining_space = 0;
    go = read_itimer();
    xmed_copy_out(&buf[0], SZ, &state);
    stop = read_itimer();
    time = stop - go;
    printf("xmed_copy_out: %d cycles\n", time);

    go = read_itimer();
    bcopy(&buf[1], dst, SZ);
    stop = read_itimer();
    time = stop - go;
    printf("bcopy[off=1]: %d cycles\n", time);

    state.next_word = dst;
    state.remaining_space = 0;
    go = read_itimer();
    xmed_copy_out(&buf[1], SZ, &state);
    stop = read_itimer();
    time = stop - go;
    printf("xmed_copy_out[off=1]: %d cycles\n", time);

    go = read_itimer();
    bcopy(&buf[2], dst, SZ);
    stop = read_itimer();
    time = stop - go;
    printf("bcopy[off=2]: %d cycles\n", time);

    state.next_word = dst;
    state.remaining_space = 0;
    go = read_itimer();
    xmed_copy_out(&buf[2], SZ, &state);
    stop = read_itimer();
    time = stop - go;
    printf("xmed_copy_out[off=2]: %d cycles\n", time);

    go = read_itimer();
    bcopy(&buf[3], dst, SZ);
    stop = read_itimer();
    time = stop - go;
    printf("bcopy[off=3]: %d cycles\n", time);

    state.next_word = dst;
    state.remaining_space = 0;
    go = read_itimer();
    xmed_copy_out(&buf[3], SZ, &state);
    stop = read_itimer();
    time = stop - go;
    printf("xmed_copy_out[off=3]: %d cycles\n", time);
#endif

    go = read_itimer();
    usleep(1000*1000);
    stop = read_itimer();
    time = stop - go;
    printf("nap(1000): %d cycles\n", time);
} /* main */
