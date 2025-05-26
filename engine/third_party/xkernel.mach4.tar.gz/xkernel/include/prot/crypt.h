/*
 * crypt.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993  Arizona Board of Regents
 *
 *
 * $Revision: 1.2 $
 * $Date: 1993/06/24 20:41:31 $
 *
 */

#ifndef crypt_h
#define crypt_h



#define CRYPT_IN_RESOLVE      (CRYPT_CTL*MAXOPS + 0)
#define CRYPT_IN_SET          (CRYPT_CTL*MAXOPS + 1)
#define CRYPT_IN_INVALIDATE   (CRYPT_CTL*MAXOPS + 2)
#define CRYPT_IN_ISEMPTY      (CRYPT_CTL*MAXOPS + 3)
#define CRYPT_IN_ISVALID      (CRYPT_CTL*MAXOPS + 4)
#define CRYPT_IN_ISINVALID    (CRYPT_CTL*MAXOPS + 5)
#define CRYPT_IN_KEYSIZE      (CRYPT_CTL*MAXOPS + 6)
#define CRYPT_IN_HOSTSIZE     (CRYPT_CTL*MAXOPS + 7)
#define CRYPT_OUT_RESOLVE     (CRYPT_CTL*MAXOPS + 8)
#define CRYPT_OUT_SET         (CRYPT_CTL*MAXOPS + 9)
#define CRYPT_OUT_INVALIDATE  (CRYPT_CTL*MAXOPS + 10)
#define CRYPT_OUT_ISEMPTY     (CRYPT_CTL*MAXOPS + 11)
#define CRYPT_OUT_ISVALID     (CRYPT_CTL*MAXOPS + 12)
#define CRYPT_OUT_ISINVALID   (CRYPT_CTL*MAXOPS + 13)
#define CRYPT_OUT_KEYSIZE     (CRYPT_CTL*MAXOPS + 14)
#define CRYPT_OUT_HOSTSIZE    (CRYPT_CTL*MAXOPS + 15)



#  ifdef __STDC__

void	crypt_init( XObj );

#  endif


#endif
