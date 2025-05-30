/*********************************************************************
 *                
 * Copyright (C) 2003, 2009-2010,  Karlsruhe University
 *                
 * File path:     amd64.cc
 * Description:   AMD64 syscall pointers.
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
 * $Id: amd64-syscalls.c,v 1.2 2003/09/24 19:06:28 skoglund Exp $
 *                
 ********************************************************************/

#include <l4/kip.h>


__L4_Ipc_t __L4_Ipc = nullptr;
__L4_Lipc_t __L4_Lipc = nullptr;
__L4_Unmap_t __L4_Unmap = nullptr;
__L4_Schedule_t __L4_Schedule = nullptr;
__L4_ThreadSwitch_t __L4_ThreadSwitch = nullptr;
__L4_SystemClock_t __L4_SystemClock = nullptr;
__L4_ExchangeRegisters_t __L4_ExchangeRegisters = nullptr;

__L4_ThreadControl_t __L4_ThreadControl = nullptr;
__L4_SpaceControl_t __L4_SpaceControl = nullptr;
__L4_ProcessorControl_t __L4_ProcessorControl = nullptr;
__L4_MemoryControl_t __L4_MemoryControl = nullptr;

extern "C" void __L4_Init( void )
{
    L4_KernelInterfacePage_t *kip;
    L4_Word_t dummy;
    
    kip = (L4_KernelInterfacePage_t *) L4_KernelInterface( &dummy, &dummy, &dummy );


    __L4_Ipc = (__L4_Ipc_t) (kip->Ipc);
    __L4_Lipc = (__L4_Lipc_t) (kip->Lipc);
    __L4_Unmap = (__L4_Unmap_t) (kip->Unmap);
    __L4_Schedule = (__L4_Schedule_t) (kip->Schedule);
    __L4_ThreadSwitch = (__L4_ThreadSwitch_t) (kip->ThreadSwitch);
    __L4_SystemClock = (__L4_SystemClock_t) (kip->SystemClock);
    __L4_ExchangeRegisters = (__L4_ExchangeRegisters_t) (kip->ExchangeRegisters);

    __L4_ThreadControl = (__L4_ThreadControl_t) (kip->ThreadControl);
    __L4_SpaceControl = (__L4_SpaceControl_t) (kip->SpaceControl);
    __L4_ProcessorControl = (__L4_ProcessorControl_t) (kip->ProcessorControl );
    __L4_MemoryControl = (__L4_MemoryControl_t) (kip->MemoryControl);

}

