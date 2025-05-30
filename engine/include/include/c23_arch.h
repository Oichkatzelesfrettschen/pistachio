#pragma once

#if defined(__x86_64__) || defined(_M_X64)
#  define LITES_ARCH_X86_64 1
#elif defined(__i386__) || defined(_M_IX86)
#  define LITES_ARCH_I386 1
#else
#  error "Unsupported architecture"
#endif

static_assert(sizeof(void*) == 8 || sizeof(void*) == 4,
              "Pointer size must be 32 or 64 bits");

