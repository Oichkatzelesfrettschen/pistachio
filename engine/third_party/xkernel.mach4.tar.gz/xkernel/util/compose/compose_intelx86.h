/*
 * $RCSfile: compose_intelx86.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.1 $
 * $Date: 1993/10/07 18:21:47 $
 */

#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/dir.h>
#define DIRENT direct
#include <sys/stat.h>

#ifdef __STDC__

int	unlink( char * );
int	access( char *, int );

/* 
 * System prototypes
 */
void	perror( char * );
int	printf();
int	bzero( char *, int );
int	_flsbuf();
int	_filbuf();
void	exit( int );
int	fclose( FILE * );
int	fprintf( FILE *, char *, ... );

#endif __STDC__
