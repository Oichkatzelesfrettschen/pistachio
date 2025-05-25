/* 
 * msg_s.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.11 $
 * $Date: 1993/07/25 00:53:25 $
 */

/*
 * Independent definition of the message structure.  Protocols should
 * include msg.h which includes this file.
 */

#ifndef msg_s_h
#define msg_s_h
#define MSG_NEW_ALG

typedef struct MNode MNode, MNodeLeaf, MNodePair, MNodePage, MNodeBuf;

/*
 * the Msg structure
 */

#ifndef xtype_h
#include "xtype.h"
#endif

typedef struct {
#ifdef MSG_NEW_ALG
  char	     *headPtr;
  char	     *tailPtr;
  char	     *stackHeadPtr;
  char	     *stackTailPtr;
  struct {
    unsigned short numNodes;
    unsigned int   myStack;
  } state;
#else
  long       length;
  long       offset;
#endif MSG_NEW_ALG
  MNodeLeaf* stack;
  MNode*     tree;
  VOID       *attr;
#ifdef MSG_NEW_ALG
  MNodeLeaf  *lastStack;
  char	     *lastStackTailPtr;
  struct {
    unsigned int   myLastStack;
  } tailstate;
#endif MSG_NEW_ALG
  int	attrLen;
} Msg;

#endif msg_s_h
