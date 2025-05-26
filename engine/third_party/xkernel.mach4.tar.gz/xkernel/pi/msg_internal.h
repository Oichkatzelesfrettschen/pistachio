/*
 * msg_internal.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.22 $
 * $Date: 1993/07/03 00:49:41 $
 */

/* this should be a power of two; see PAGEROUND in msg.c */

#define MSG_STATISTICS

#ifndef XKMACHKERNEL
#define PageSize 4096
#define PAGEROUND(len)  ((len + MNodeLeafSize + (PageSize-1)) / PageSize)
#else
/* this hack is more efficient for funky inkernel malloc/free */
#define PageSize 2048
/* was 4096 -- rcs */
#define PAGEROUND(len)  ((len + MNodeLeafSize + (PageSize-1) + sizeof(int)) / PageSize)
#endif ! XKMACHKERNEL
/* 
 * Size of virtual space for tree nodes
 */
#ifdef MSGCACHEING
static int MNodePairVSpace = 1 * PageSize;
static int MNodeBufVSpace =  1 * PageSize;
static int MNodePageVSpace = 126 * PageSize;

static char *mWindow;
#endif MSGCACHEING


/* enum NodeType */

enum NodeType {
  t_MNodeJunk,
  t_MNodePair,
  t_MNodePage,
  t_MNodeUsrPage,
  t_MNodeBuf,
#ifdef MSG_NEW_ALG
  t_MNodeDummy
#endif MSG_NEW_ALG
};

/* These sizes are computed from the MNode structure definition */
#define MNodeSize (sizeof(enum NodeType) + sizeof(long))
#define MNodePairSize (MNodeSize + 2 * sizeof(struct nlink))
#define MNodeLeafSize (MNodeSize + sizeof(long) + sizeof(char *))
#define MNodePageSize (PageSize)
#define MNodePageDataSize (MNodePageSize - MNodeLeafSize)
#define MNodeUsrPageSize (MNodeLeafSize + sizeof(void (*)()))
#define MNodeBufSize  (MNodeLeafSize + sizeof(void (*)()))

#define mprsize MNodePairSize
#define mpgsize MNodePageSize
#define mupsize MNodeUsrPageSize
#define mbfsize MNodeBufSize

#define mprtype t_MNodePair
#define mpgtype t_MNodePage
#define muptype t_MNodeUsrPage
#define mbftype t_MNodeBuf

/*
 *     the left-right node pair constituent
 */

struct nlink {
  long offset;
  long length;
  MNode  *node;
};

/*
 * struct MNode
 * all tree nodes
 */
struct MNode {
  enum NodeType type;
  unsigned long refCnt;
  union {
    struct {
      struct nlink l, r;
    } pair;
    struct {
      long  size;     /* size of buffer */
      char *data;     /* the buffer */
    } leaf;
    struct {
      long  size;     /* size of buffer */
      char *data;     /* the buffer */
      char buffer[MNodePageDataSize];
    } page;
    struct {
      long  size;     /* size of buffer */
      char *data;     /* the buffer */
      void (*bFree)(/* void* */);  /* the buffer's deallocator function */
    } usrpage;
    struct {
      long  size;     /* size of buffer */
      char *data;     /* the buffer */
      void (*bFree)(/* void* */);  /* the buffer's deallocator function */
    } buf;
  } b;
};

/* non-recursive routines for traversing a message */

/* #define MSG_NON_REC */
#ifdef MSG_NON_REC

#define MSG_NR_STK_SZ (8*1024 - 4)

struct mstack {
  int size;
  MNodePage **top, **bottom, **limit;
};

#endif /* MSG_NON_REC */
