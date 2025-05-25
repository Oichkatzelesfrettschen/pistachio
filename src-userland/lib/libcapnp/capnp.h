#ifndef LIBCAPNP_CAPNP_H
#define LIBCAPNP_CAPNP_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Minimal stub API for Cap'n Proto parsing used by tests and examples.
 * The real implementation would parse a Cap'n Proto message from 'buf'.
 */
int capnp_parse(const char *buf);

#ifdef __cplusplus
}
#endif

#endif /* LIBCAPNP_CAPNP_H */
