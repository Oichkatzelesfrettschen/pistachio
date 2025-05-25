#pragma once

#ifdef __GNUC__
# define L4_CDECL [[gnu::cdecl]]
# define L4_FASTCALL [[gnu::fastcall]]
# define L4_STDCALL [[gnu::stdcall]]
# define L4_REGPARM(n) [[gnu::regparm(n)]]
#else
# define L4_CDECL
# define L4_FASTCALL
# define L4_STDCALL
# define L4_REGPARM(n)
#endif

