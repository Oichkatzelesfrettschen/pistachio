/*
 * $RCSfile: compose_hp700.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1991  Arizona Board of Regents
 *
 *
 * $Revision: 1.1 $
 * $Date: 1993/03/31 21:18:10 $
 */

#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/dir.h>
#define DIRENT dirent
#include <sys/stat.h>

int	unlink( char * );
int	access( char *, int );
