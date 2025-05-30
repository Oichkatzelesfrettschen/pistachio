/*     
 * $RCSfile: romopt.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.3 $
 * $Date: 1993/05/09 00:26:03 $
 */


#ifndef romopt_h
#define romopt_h

#ifndef upi_h
#include "upi.h"
#endif

typedef xkern_return_t	(* XObjRomOptFunc)(
				       XObj, char **, int numFields,
				        int line, void *
				       );

typedef struct {
    char		*name;
    int			minFields;
    XObjRomOptFunc	func;
} XObjRomOpt;


typedef xkern_return_t	(* RomOptFunc)(
				       char **, int numFields,
				       int line, void *
				       );

typedef struct {
    char		*name;
    int			minFields;
    RomOptFunc	func;
} RomOpt;


/* 
 * findXObjRomOpts -- Called by a protocol that has defined an array of
 * RomOpt structures.  If the first field of a romfile line matches
 * either the protocol name or the full instance name of the XObject,
 * the romOpt array is scanned.  If the second field of the line
 * matches the name field of one of the RomOpts, or if the name field
 * of one of the romOpts is the empty string, the RomOptFunc for that
 * option is called with the object, all rom fields on that line, the
 * number of fields on that line, the line number and the
 * user-supplied argument.
 *
 * If the first field of a rom line appears to match the XObject but
 * none of the supplied RomOpts matches the second field, an error
 * message will be printed and XK_FAILURE will be returned.  The rest
 * of the rom entries will not be scanned.  This same behaviour
 * results from the RomOptFunc returning XK_FAILURE.
 *
 * Note that default (empty-string) RomOpts must come last in the
 * RomOpt array.
 */
xkern_return_t	findXObjRomOpts(
			    XObj, XObjRomOpt *, int numOpts, VOID *arg
			    );



xkern_return_t	findRomOpts(
			    char *, RomOpt *, int numOpts, VOID *arg
			    );



#endif /* ! romopt_h */
