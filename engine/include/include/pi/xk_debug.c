/* 
 * debug.c
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.17 $
 * $Date: 1993/10/26 20:04:33 $
 */

#include "xk_debug.h"
#include "platform.h"
#ifndef XKMACHKERNEL
#include "x_stdio.h"
#endif XKMACHKERNEL

#ifdef XK_DEBUG

int
  tracebuserror,
  tracecustom,
  traceether,
  traceevent,
  tracefixme,
  traceidle,
  traceie,
  tracememoryinit,
  traceprocesscreation,
  traceprocessswitch,
  traceprotocol,
  traceprottest,
  tracetick,
  tracetrap,
  traceuser;

char assertMessage[] = "Assertion failed: file %s, line %d\n";

#endif XK_DEBUG


char	errBuf[250];

void
xError( msg )
    char	*msg;
{
    xTraceLock();
#if !defined(XKMACHKERNEL) && !defined(XKMACH4)
    fprintf(stderr, "%s\n", msg);
#else
    printf("%s\n", msg);
#endif XKMACHKERNEL
    xTraceUnlock();
}
