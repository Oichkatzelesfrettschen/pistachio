/*
 * compose_sparc.h
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1993,1991,1990  Arizona Board of Regents
 *
 *
 * $Revision: 1.7 $
 * $Date: 1993/03/31 21:25:33 $
 */

#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/dir.h>
#define DIRENT direct
#include <sys/stat.h>

#ifdef __STDC__

int 	lstat( char *path, struct stat *buf );
char	toupper( char );
int	getopt( int, char **, char *);

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

