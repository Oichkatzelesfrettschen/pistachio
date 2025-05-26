/*
 * trace variable initialization
 *
 * Warning: this file is generated from graph.comp and is overwritten
 * every time 'make compose' is run
 */

#include "xk_debug.h"
#include "compose.h"
#include "protocols.h"


void
initTraceLevels()
{
#ifdef XK_DEBUG

    traceblastp = TR_SOFT_ERRORS;

#endif XK_DEBUG
}
