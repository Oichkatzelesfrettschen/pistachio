/* include file to hack a standalone version of the ssr test */

/* from merge/include/prot/ip_host.h */
typedef struct iphost {
    unsigned char 	a,b,c,d;
} IPhost;
/* end of ip_host.h stuff */


/* from merge/include/xtime.h */
typedef struct {
  long sec;
  long usec;
} XTime;

#ifdef __STDC__

/* set *t to the current time of day */
void xGetTime(XTime *t);

#endif __STDC__
/* end of xtime.h stuff */

/* just hacked together */
typedef struct xobject
  {  void * state; char *instName; } *XObj;
typedef int Event;
#define xMalloc malloc


/* from merge/include/xtype.h */
typedef unsigned int	bool;
typedef enum xkret { XK_SUCCESS = 0, XK_FAILURE = -1 } xkern_return_t;
typedef void (*Pfv)();
/* end of xtype.h stuff */

/* from merge/include/upi.h */
#define	ERR_XOBJ	((XObj) XK_FAILURE)
enum { GETMYHOST = 0 };
/* end of stuff from upi.h */

/* from merge/include/trace.h */
#define TR_NEVER		100 /* we'll never use this */
#define TR_FULL_TRACE		25  /* every subroutine entry (sometimes exit, too */
#define TR_DETAILED		 9  /* all functions plus dumps of data structures at strategic points */
#define TR_FUNCTIONAL_TRACE	 7  /* all the functions of the module and their parameters */
#define TR_MORE_EVENTS		 6  /* probably should have used 7, instead */
#define TR_EVENTS		 5  /* more detail than major_events */
#define TR_SOFT_ERRORS		 4  /* mild warning */
#define TR_SOFT_ERROR TR_SOFT_ERRORS
#define TR_MAJOR_EVENTS		 3  /* open, close, etc. */
#define TR_GROSS_EVENTS		 2  /* probably should have used 3, instead */
#define TR_ERRORS		 1  /* serious non-fatal errors, low-level trace (init, closesessn, etc. */
#define TR_ALWAYS		 0  /* normally only used during protocol development */
/* end of stuff from trace.h */

/* from merge/include/xk_debug.h */
#ifdef __STDC__

void	xTraceLock( void );
void	xTraceUnlock( void );
void	xTraceInit( void );

#else

void	xTraceLock();
void	xTraceUnlock();
void	xTraceInit();

#endif


#if defined(XK_DEBUG) || defined(lint)

#define PRETRACE(l) { int __i=l; xTraceLock();  while(__i--) putchar(' '); }
#define POSTTRACE { putchar('\n'); xTraceUnlock(); }

#ifdef __STDC__
#define XPASTE(X,Y) X##Y
#define PASTE(X,Y) XPASTE(X,Y)


#   define xIfTrace(t, l) \
	if (PASTE(trace,t) >= l)
#   define xTrace0(t, l, f) 		\
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f); 		\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace1(t, l, f, arg1) 	\
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1);	\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace2(t, l, f, arg1, arg2) \
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1, arg2);	\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace3(t, l, f, arg1, arg2, arg3) \
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1, arg2, arg3);	\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace4(t, l, f, arg1, arg2, arg3, arg4) \
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1, arg2, arg3, arg4);	\
	    	POSTTRACE;		\
	    }				\
	} while (0)
#   define xTrace5(t, l, f, arg1, arg2, arg3, arg4, arg5) \
	do {				\
	    if (PASTE(trace,t) >= l) {	\
		PRETRACE(l);		\
	    	printf(f, arg1, arg2, arg3, arg4, arg5); \
	    	POSTTRACE;		\
	    }				\
	} while (0)
#else
#define D___I(X) X


#   define xIfTrace(t, l) \
	if (D___I(trace)t >= l)
#   define xTrace0(t, l, f) 			\
	do {					\
	    if (D___I(trace)t >= l) { 		\
	    	PRETRACE(l); 			\
	    	printf(f); 			\
	    	POSTTRACE; 			\
	    }					\
	} while (0)
#   define xTrace1(t, l, f, arg1) \
	do {					\
	    if (D___I(trace)t >= l) { 		\
	    	PRETRACE(l); 			\
		printf(f, arg1); 		\
	    	POSTTRACE; 			\
	    }					\
	} while (0)
#   define xTrace2(t, l, f, arg1, arg2) \
	do {					\
	    if (D___I(trace)t >= l) { 		\
	    	PRETRACE(l); 			\
	    	printf(f, arg1, arg2); 		\
	    	POSTTRACE; 			\
	    }					\
	} while (0)
#   define xTrace3(t, l, f, arg1, arg2, arg3) \
	do {					\
	    if (D___I(trace)t >= l) { 		\
	    	PRETRACE(l); 			\
	    	printf(f, arg1, arg2, arg3); 	\
	    	POSTTRACE; 			\
	    }					\
	} while (0)
#   define xTrace4(t, l, f, arg1, arg2, arg3, arg4) \
	do {					\
	    if (D___I(trace)t >= l) { 		\
	    	PRETRACE(l); 			\
	    	printf(f, arg1, arg2, arg3, arg4); \
	    	POSTTRACE; 			\
	    }					\
	} while (0)
#   define xTrace5(t, l, f, arg1, arg2, arg3, arg4, arg5) \
	do {					\
	    if (D___I(trace)t >= l) { 		\
	    	PRETRACE(l); 			\
	    	printf(f, arg1, arg2, arg3, arg4, arg5); \
	    	POSTTRACE; 			\
	    }					\
	} while (0)

#endif /* __STDC__ */
#else

#   define xIfTrace(t, l) if (0)
#   define xTrace0(t, l, f)
#   define xTrace1(t, l, f, arg1)
#   define xTrace2(t, l, f, arg1, arg2)
#   define xTrace3(t, l, f, arg1, arg2, arg3)
#   define xTrace4(t, l, f, arg1, arg2, arg3, arg4)
#   define xTrace5(t, l, f, arg1, arg2, arg3, arg4, arg5)

#endif	/* XK_DEBUG */

extern void	xError(
#ifdef __STDC__
		       char *
#endif
		       );
/* end of xk_debug.h stuff */

