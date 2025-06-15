#pragma once

#include "pqcrypto.hpp"
#include <typed_channel.h>

/** Lattice-based secure IPC channel. */
struct lattice_channel_t {
  typed_channel_t base;           /**< Underlying typed channel. */
  pqcrypto::KyberKeyPair kx;      /**< Key-exchange key pair. */
  pqcrypto::DilithiumKeyPair sig; /**< Signature key pair. */
};

/**
 * \brief Initialize a lattice channel with PQ crypto key material.
 * \param ch       Channel to initialize.
 * \param partner  Remote thread identifier.
 */
inline void lattice_channel_setup(lattice_channel_t *ch,
                                  L4_ThreadId_t partner) {
  typed_channel_init(&ch->base, partner);
  ch->kx = pqcrypto::kyber_generate_keypair();
  ch->sig = pqcrypto::dilithium_generate_keypair();
}
