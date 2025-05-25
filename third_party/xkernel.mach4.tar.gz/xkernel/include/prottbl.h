/*
 * prottbl.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.7 $
 * $Date: 1993/05/09 00:23:29 $
 */

/* 
 * Interface to the protocol table utility
 */

#ifndef prottbl_h
#define prottbl_h

#ifndef upi_h
#include "upi.h"
#endif

/* 
 * return the protocol id number of the protocol by looking in the table.
 * If no entry for this protocol exists, -1 is returned.
 */
extern long	protTblGetId(
#ifdef __STDC__
			 char *protocolName
#endif
			 );

/* 
 * relProtNum -- return the number of hlp relative to llp.  If this
 * number can not be determined from the table, -1 is returned.  This
 * should be considered an error.
 */
extern long	relProtNum(
#ifdef __STDC__
			   XObj hlp, XObj llp
#endif
			   );


extern void	prottbl_init( void );


#  ifdef XK_DEBUG

/* 
 * Display the contents of the protocol table map
 */
void
protTblDisplayMap(
#    ifdef __STDC__
		  void
#    endif
		  );
#  endif  XK_DEBUG

#endif !prottbl_h
