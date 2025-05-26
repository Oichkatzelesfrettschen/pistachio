/*
 *  gethost.h
 *
 *   prototype for xkernel version of gethostbyname
 *
 */


#ifdef __STDC__
        xkern_return_t xk_gethostbyname(char *, IPhost *);
#else
	xkern_return_t xk_gethostbyname();
#endif
