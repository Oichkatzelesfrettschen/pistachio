#pragma once

#include <array>
#include <atomic>
#include <exo_ipc.h>
#include <l4/message.h>
#include <l4/thread.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

/** Capability token size in 64-bit words. */
#define LATTICE_TOKEN_WORDS 8

/** Channel object carrying connection metadata. */
typedef struct lattice_channel {
  L4_ThreadId_t partner;                           /**< Partner thread ID. */
  std::array<uint64_t, LATTICE_TOKEN_WORDS> token; /**< Capability token. */
  std::atomic_uint64_t seq; /**< Atomic sequence number. */
  std::string label;        /**< Security label. */
} lattice_channel_t;

/** Initialize and connect a lattice channel. */
bool lattice_connect(lattice_channel_t *ch, L4_ThreadId_t partner,
                     const char *label);

/** Send an IPC message over the lattice channel. */
exo_ipc_status lattice_send(lattice_channel_t *ch, const L4_Word_t *words,
                            size_t count);

/** Receive an IPC message on the lattice channel. */
exo_ipc_status lattice_recv(lattice_channel_t *ch);

/** Close the lattice channel. */
void lattice_close(lattice_channel_t *ch);

/** Grant a capability token to the channel. */
void lattice_grant(lattice_channel_t *ch,
                   const uint64_t token[LATTICE_TOKEN_WORDS]);

#ifdef __cplusplus
}
#endif
