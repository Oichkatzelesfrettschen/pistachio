/*********************************************************************
 *                
 * Copyright (C) 2003-2004,  Karlsruhe University
 *                
 * File path:     grabmem/roottask.cc
 * Description:   A simple root task
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
 * $Id: grabmem.cc,v 1.2 2004/02/26 19:11:07 skoglund Exp $
 *                
 ********************************************************************/
#include <l4io.h>
#include <l4/kdebug.h>
#include <l4/memory.h>

#define KB(x) (x*1024)
#define MB(x) (x*1024*1024)
#define GB(x) (x*1024*1024*1024)

int main (void)
{
    printf ("Hello world, I will now allocate all available memory.\n\n");

    L4_Word_t tsize = 0;
    L4_ThreadId_t memsrv = memory_server();
    const L4_Word_t pagesz = 4096;
    while (true)
    {
        L4_Word_t addr = memory_alloc(memsrv, pagesz);
        if (!addr)
            break;
        tsize += pagesz;
    }

    // Avoid using floating point
    printf ("\nTotal memory: %ld.%ldGB | %ld.%ldMB | %ldKB\n",
	    tsize / GB(1), ((tsize * 100) / GB(1)) % 100,
	    tsize / MB(1), ((tsize * 100) / MB(1)) % 100,
	    tsize / KB(1));

    for (;;);
    return 0;
}
