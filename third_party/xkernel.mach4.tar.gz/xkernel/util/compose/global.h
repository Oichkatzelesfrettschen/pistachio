/*
 * global.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.15 $
 * $Date: 1993/10/14 22:30:45 $
 */

#define MAX_FILES 20
#define MAX_PROTOCOL 50

typedef struct {
    char *	name;
    char *	instance;
    int		index;		/* index into instance table */
} ProtName;

typedef struct {
    char *	name;
    char *	value;
    int		instantiate;
} TraceVar;

typedef enum {
    INIT_NOT_DONE,
    INIT_WORKING,
    INIT_DONE
} InitState;

typedef struct protocol {
    ProtName	n;
    char *	path;
    char *	trace;	  /* String rep. of initial value of trace variable */
    char *	traceVar; /* String rep. of name of trace variable */
    int		numfiles;
    char * 	files[MAX_FILES + 1];
    int 	numdown;
    ProtName	down[MAX_PROTOCOL + 1];
    InitState	initState;
    int		isDriver;
    TraceVar	*tv;
} PROTOCOL;

extern int	fileLine;
extern int	filePosition;

#ifdef __STDC__

void	addInstance( PROTOCOL * );
void	addProtTbl( char *name );
TraceVar	*addTraceVar( char *name, char * value, int instantiate );
void	errorCycle( void );
void	errorCycleName( char * );
void	errorFile( char *s );
void	errorLaterInstanceFiles( PROTOCOL * );
void	errorProtlUndefined( char *p1, char *p2 );
void	errorTooManyStates( void );
void	finishErrorCycle( void );
char *	join( char *, char * );
void	lastDriver( void );
void	parse( void );
void	syntaxErrorChar( char expected, char got );
void	syntaxErrorString( char *expected );
char *	xerox( char * );
void 	warnCouldNotAccess( char * );
void	warnDefaultPtblNotSupported( void );
void	warnProtNotFound( char * );
void	warnReassignedTraceValue( char * );

#endif __STDC__

#include ARCH_INCLUDE
