#pragma once

#if defined(__cplusplus)
#define __BEGIN_DECLS   extern "C" {
#define __END_DECLS     }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#define __CONCAT(x,y)   x ## y
#define __STRING(x)     #x

/* Function attributes */
#if defined(__GNUC__)
#define __dead2         __attribute__((__noreturn__))
#define __pure2         __attribute__((__const__))
#else
#define __dead2
#define __pure2
#endif
#ifndef _Noreturn
# if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
/* _Noreturn is a keyword */
# elif defined(__GNUC__)
#  define _Noreturn __attribute__((__noreturn__))
# else
#  define _Noreturn
# endif
#endif
#define __dead
#define __unused       __attribute__((__unused__))

