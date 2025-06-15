#include "lattice_ipc.h"
#include <array>
#include <l4/message.h>
#include <l4/thread.h>
#include <string>

using namespace std;

extern "C" {

/** Initialize and connect a lattice channel. */
bool lattice_connect(lattice_channel_t *ch, L4_ThreadId_t partner,
                     const char *label) {
  if (!ch)
    return false;
  ch->partner = partner;
  ch->seq.store(0, std::memory_order_relaxed);
  ch->label = label ? std::string(label) : std::string();
  ch->token.fill(0);
  return true;
}

/** Send an IPC message over the lattice channel. */
exo_ipc_status lattice_send(lattice_channel_t *ch, const L4_Word_t *words,
                            size_t count) {
  if (!ch)
    return exo_ipc_status::NoPartner;
  L4_Msg_t msg;
  L4_MsgPut(&msg, 0, count, words, 0, nullptr);
  L4_MsgLoad(&msg);
  ch->seq.fetch_add(1, std::memory_order_acq_rel);
  return exo_send(ch->partner);
}

/** Receive an IPC message on the lattice channel. */
exo_ipc_status lattice_recv(lattice_channel_t *ch) {
  if (!ch)
    return exo_ipc_status::NoPartner;
  L4_MsgTag_t tag = L4_Receive(ch->partner);
  ch->seq.fetch_add(1, std::memory_order_acq_rel);
  return exo_ipc_from_tag(tag);
}

/** Close the lattice channel. */
void lattice_close(lattice_channel_t *ch) {
  if (!ch)
    return;
  ch->label.clear();
  ch->seq.store(0, std::memory_order_relaxed);
  ch->token.fill(0);
  ch->partner = L4_nilthread;
}

/** Grant a capability token to the channel. */
void lattice_grant(lattice_channel_t *ch,
                   const uint64_t token[LATTICE_TOKEN_WORDS]) {
  if (!ch || !token)
    return;
  for (size_t i = 0; i < LATTICE_TOKEN_WORDS; ++i)
    ch->token[i] = token[i];
}

} // extern "C"
