/* external data structure for NIST Secure Hash Algorithm */

typedef struct {
  unsigned long hi_bit_count, lo_bit_count;
  unsigned long h0,h1,h2,h3,h4;
  unsigned long data[80]; }  SHA_state;

void sha_setup(SHA_state *);
void sha_store(SHA_state *, unsigned char *, int count);
void sha_finish(SHA_state *);


