/* 
 * sim_ether_i.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.8 $
 * $Date: 1993/02/09 07:56:24 $
 */

#define SIMETH_MAX_INSTANCES	5
#define MAX_ETH_SIZE (EMAXPAK+sizeof(ETHhdr))

#define MAX_ERROR_COUNT 1000

#define NOREF 0			/* buffer in use flag */
#define INUSE 1

#define CLEAR_REF(_bp) (_bp)->ref = NOREF

#define RCVBUFSIZE 	50*1024		
#define EMAXPAK 	(MAX_ETH_DATA_SZ + sizeof(ETHhdr))


/* block: basic structure used to pass packets from ethernet to process */
typedef struct {
    char id;
    char ref;
    int cur_len;
    char *data;
    Msg *msg;  /* the data ptr is the msg stack */
    Semaphore sem;
    XObj self;
} block;

/* top-level pool of buffers */
typedef struct {
    int total_blocks;
    int next_block;
    block *blocks;
} block_pool;

typedef struct {
    char	*buf;
    int		len;
    int		sock;
} EthMsg;

typedef struct {
    XObj	arp;
    int		sock;
    int		port;
    ETHhost	myHost;
    int		errorCount;
} PState;

