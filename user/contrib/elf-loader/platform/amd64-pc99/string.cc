/*********************************************************************
 *                
 * Copyright (C) 2003,  Karlsruhe University
 *                
 * File path:     string.cc
 * Description:   stupid library routines.. 
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: string.cc,v 1.1 2004/03/01 19:04:32 stoess Exp $
 *                
 ********************************************************************/

/*
 * These are buggy as well..
 */
 
#include <l4/types.h>
#include "globals.h"
char * ___strtok = nullptr;

char * strcpy(char * dest,const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

char * strncpy(char * dest,const char *src,unsigned int count)
{
	char *tmp = dest;

	while (count-- && (*dest++ = *src++) != '\0')
		/* nothing */;

	return tmp;
}

char * strcat(char * dest, const char * src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;

	return tmp;
}

char * strncat(char *dest, const char *src, unsigned int count)
{
	char *tmp = dest;

	if (count) {
		while (*dest)
			dest++;
		while ((*dest++ = *src++)) {
			if (--count == 0)
				break;
		}
	}

	return tmp;
}

int strcmp(const char * cs,const char * ct)
{
        signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

int strncmp(const char * cs,const char * ct, unsigned int count)
{
        signed char __res = 0;

	while (count) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
		count--;
	}

	return __res;
}

char * strchr(const char * s,char c)
{
	for(; *s != c; ++s)
		if (*s == '\0')
			return nullptr;
	return (char *) s;
}

char * strrchr(const char * s,char c)
{
	const char * r = nullptr;
	while (*s++)
		if (*s == c) r = s;
	return (char *) r;
}

unsigned int strlen(const char * s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

unsigned int strnlen(const char * s, unsigned int count)
{
	const char *sc;

	for (sc = s; *sc != '\0' && count--; ++sc)
		/* nothing */;
	return sc - s;
}

unsigned int strspn(const char *s, const char *accept)
{
	const char *p;
	const char *a;
	unsigned int count = 0;

	for (p = s; *p != '\0'; ++p) {
		for (a = accept; *a != '\0'; ++a) {
			if (*p == *a)
				break;
		}
		if (*a == '\0')
			return count;
		++count;
	}

	return count;
}

char * strpbrk(const char * cs,const char * ct)
{
	const char *sc1,*sc2;

	for( sc1 = cs; *sc1 != '\0'; ++sc1) {
		for( sc2 = ct; *sc2 != '\0'; ++sc2) {
			if (*sc1 == *sc2)
				return (char *) sc1;
		}
	}
	return nullptr;
}

char * strtok(char * s,const char * ct)
{
        char *sbegin, *send;

        sbegin  = s ? s : ___strtok;
        if (!sbegin) {
                return nullptr;
        }
        sbegin += strspn(sbegin,ct);
        if (*sbegin == '\0') {
                ___strtok = nullptr;
                return( nullptr );
        }
        send = strpbrk( sbegin, ct);
        if (send && *send != '\0')
                *send++ = '\0';
        ___strtok = send;
        return (sbegin);
}
