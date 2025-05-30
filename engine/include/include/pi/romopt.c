/*     
 * $RCSfile: romopt.c,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.5 $
 * $Date: 1993/10/26 20:14:36 $
 */

/* 
 * Library support for processing romfile options
 */


#include "xkernel.h"
#include "romopt.h"


typedef struct {
    char		*name;
    int			minFields;
    xkern_return_t	(* func)();
} GenericRomOpt;


#ifdef __STDC__
static xkern_return_t	findOptions( char *, char *, GenericRomOpt *,
				     int, void *, XObj );
#endif __STDC__


static xkern_return_t
findOptions( name, fullName, opts, numOpts, arg, obj )
    char		*name, *fullName;
    XObj		obj;
    GenericRomOpt	*opts;
    int			numOpts;
    VOID		*arg;
{
    int			line, j, nFields;
    xkern_return_t	xkr;
    GenericRomOpt	*opt = 0;

    for ( line=0; rom[line][0]; line++ ) {
	if ( ! strcmp(rom[line][0], name) || ! strcmp(rom[line][0], fullName) ) {
	    for ( j=0; j < numOpts; j++ ) {
		opt = &opts[j];
		if ( strlen(opt->name) == 0 ) {
		    /*
		     * a default option -- issue warning if it's not
		     * the last option 
		     */
		    if ( j+1 != numOpts ) {
			sprintf(errBuf,
	"xkernel findRomOpts WARNING: %s options after default are ignored",
				fullName);
			xError(errBuf);
		    }
		    break;
		}
		if ( ! strcmp(opt->name, rom[line][1]) ) {
		    break;
		}
	    }
	    if ( j < numOpts ) {
		xAssert(opt);
		for ( nFields=1; rom[line][nFields] != 0; nFields++ )
		  ;
		if ( nFields < opt->minFields ) {
		    sprintf(errBuf,
		"romfile error(%s): not enough fields on line %d",
			    fullName, line+1);
		    xError(errBuf);
		    return XK_FAILURE;
		}
		if ( obj ) {
		    xkr = opt->func(obj, rom[line], nFields, line+1, arg);
		} else {
		    xkr = opt->func(rom[line], nFields, line+1, arg);
		}
		if ( xkr == XK_FAILURE ) {
		    sprintf(errBuf,
		"romfile error(%s): bad format on line %d",
			    fullName, line+1);
		    xError(errBuf);
		    return XK_FAILURE;
		}
	    } else {
		sprintf(errBuf,
		"romfile error(%s): unhandled option \"%s\" on line %d",
			fullName, rom[line][1], line+1);
		    xError(errBuf);
		    return XK_FAILURE;
	    }
	}
    }
    return XK_SUCCESS;
}


xkern_return_t
findXObjRomOpts( obj, opt, numOpts, arg )
    XObj	obj;
    XObjRomOpt 	*opt;
    int 	numOpts;
    VOID 	*arg;
{    
    return findOptions( obj->name, obj->fullName, (GenericRomOpt *)opt,
		        numOpts, arg, obj );
}



xkern_return_t
findRomOpts( str, opt, numOpts, arg )
    char	*str;
    RomOpt 	*opt;
    int 	numOpts;
    VOID 	*arg;
{    
    return findOptions( str, str, (GenericRomOpt *)opt, numOpts, arg, 0 );
}



