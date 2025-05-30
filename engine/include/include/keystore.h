#pragma once

#include <stddef.h>

#if defined(__has_include)
#  if __has_include(<openssl/aes.h>)
#    define KS_HAVE_OPENSSL 1
#  else
#    define KS_HAVE_OPENSSL 0
#  endif
#else
#  define KS_HAVE_OPENSSL 0
#endif

int ks_generate_key(const char *path, size_t len);
int ks_encrypt(const char *key_path, const unsigned char *in, size_t in_len, unsigned char *out,
               size_t *out_len);
int ks_decrypt(const char *key_path, const unsigned char *in, size_t in_len, unsigned char *out,
               size_t *out_len);

