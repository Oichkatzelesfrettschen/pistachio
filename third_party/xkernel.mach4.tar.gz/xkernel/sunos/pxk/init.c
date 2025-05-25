/* 
 * init.c
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.33.1.2 $
 * $Date: 1993/11/08 20:42:58 $
 */

#include <ctype.h>
#include "x_stdio.h"
#include "process.h"
#include "upi.h"
#include "xk_debug.h"
#include "event_i.h"
#include "event.h"
#include "machine.h"
#include "compose.h"
#include "platform.h"
#include "prottbl.h"
#include "x_libc.h"
#include "x_util.h"
#include "netmask.h"

int globalArgc;
char **globalArgv;
char *rom[ROM_MAX_LINES + 1][ROM_MAX_FIELDS + 1];  

int	traceinit;


static void	initRom( void );


static void
pgraphStub(ev, arg)
    Event	ev;
    VOID 	*arg;
{
    build_pgraph();
}


int
main(argc,argv)
int	argc;
char	*argv[];
{
    
    extern void clock_ih(), InitProtocols();
    
    setbuf(stdout, 0);
    globalArgc = argc;
    globalArgv = argv;
    
    LWP_Init();			/* cjt */
    
    xError("x-kernel initializes");
    xTraceInit();
    initRom();
    map_init();
    msgInit();
    evInit();
    init_clock(clock_ih, (long)(EVENT_INTERVAL/1000));
    netMaskInit();
    prottbl_init();
    upiInit();
    xMallocInit();
    build_pgraph_dev();
    evDetach( evSchedule( pgraphStub, 0, 0 ) );
    
    return 0;
}


/* 
 * Save all but the last character (the newline)
 */
static char *
savestr( char *s )
{
    char 	*r;
    unsigned	len;

    len = strlen(s);
    r = (char *) xMalloc(len);
    strncpy(r, s, len - 1);
    return r;
}


#define ROM_LEN	200

static void
initRom()
{
    char buf[ROM_LEN + 2];
    char *p;
    FILE *f;
    int	i, j;
    
    if ((f = fopen("rom", "r")) == NULL) {
	xTrace0(init, TR_MAJOR_EVENTS, "not loading ROM file");
	return;
    } else {
	xTrace0(init, TR_MAJOR_EVENTS, "loading ROM file");
    }
    i = 0;
    while ( fgets(buf, ROM_LEN + 2, f) ) {
	if ( i > ROM_MAX_LINES  ) {
	    sprintf(errBuf, "ROM file has too many lines (max %d)",
			   ROM_MAX_LINES);
	    Kabort(errBuf);
	}
	if ( strlen(buf) > ROM_LEN ) {
	    sprintf(errBuf, "ROM entry in line %d is too long (max %d chars)",
			   i, ROM_LEN);
	    Kabort(errBuf);
	}
	p = savestr(buf);
	/* 
	 * Clear out initial white space
	 */
	while ( *p && isspace(*p) ) {
	    p++;
	}
	if ( strlen(p) == 0 ) {
	    rom[i][0] = "";
	} else {
	    /* 
	     * Put a '\0' after each field and set the rom array to these
	     * fields 
	     */
	    for ( j=0; *p; j++ ) {
		if ( j >= ROM_MAX_FIELDS ) {
		    sprintf(errBuf,
			    "ROM entry on line %d has too many fields (max %d)",
			    i, ROM_MAX_FIELDS);
		    Kabort(errBuf);
		}
		if ( *p == '#' ) {
		    /* 
		     * Comments follow until end of line.  Make sure this
		     * doesn't look like the final line.
		     */
		    if ( j == 0 ) {
			rom[i][0] = p;	/* bogus, but non-null */
			rom[i][1] = 0;
		    } else {
			rom[i][j] = 0;
		    }
		    break;
		}
		rom[i][j] = p;
		/* 
		 * Find and mark the end of this field
		 */
		while ( *p && ! isspace(*p) ) {
		    p++;
		}
		if ( *p ) {
		    *p++ = 0;
		    /* 
		     * Find the start of the next field
		     */
		    while ( *p && isspace(*p) ) {
			p++;
		    }
		}
	    }
	}
	i++;
    }
    fclose(f);
}
