#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace pqcrypto {

/** Securely zero memory. */
void secure_clear(void *ptr, std::size_t len) noexcept;

/** Kyber public-key size in bytes. */
inline constexpr std::size_t KYBER_PUBLIC_KEY_SIZE = 800;
/** Kyber secret-key size in bytes. */
inline constexpr std::size_t KYBER_SECRET_KEY_SIZE = 2400;
/** Kyber shared-key size in bytes. */
inline constexpr std::size_t KYBER_SHARED_KEY_SIZE = 32;

/** Dilithium public-key size in bytes. */
inline constexpr std::size_t DILITHIUM_PUBLIC_KEY_SIZE = 1312;
/** Dilithium secret-key size in bytes. */
inline constexpr std::size_t DILITHIUM_SECRET_KEY_SIZE = 2528;
/** Dilithium signature size in bytes. */
inline constexpr std::size_t DILITHIUM_SIGNATURE_SIZE = 2420;

/** Kyber key pair container. Sensitive private key is cleared on destruction.
 */
struct KyberKeyPair {
  std::array<std::uint8_t, KYBER_PUBLIC_KEY_SIZE> public_key{};
  std::array<std::uint8_t, KYBER_SECRET_KEY_SIZE> secret_key{};
  ~KyberKeyPair() noexcept {
    secure_clear(secret_key.data(), secret_key.size());
  }
};

/** Dilithium key pair container. Clears private key on destruction. */
struct DilithiumKeyPair {
  std::array<std::uint8_t, DILITHIUM_PUBLIC_KEY_SIZE> public_key{};
  std::array<std::uint8_t, DILITHIUM_SECRET_KEY_SIZE> secret_key{};
  ~DilithiumKeyPair() noexcept {
    secure_clear(secret_key.data(), secret_key.size());
  }
};

KyberKeyPair kyber_generate_keypair();
std::array<std::uint8_t, KYBER_SHARED_KEY_SIZE> kyber_derive_shared(
    std::span<const std::uint8_t, KYBER_PUBLIC_KEY_SIZE> peer_public);

DilithiumKeyPair dilithium_generate_keypair();
std::array<std::uint8_t, DILITHIUM_SIGNATURE_SIZE>
dilithium_sign(const DilithiumKeyPair &kp,
               std::span<const std::uint8_t> message);
bool dilithium_verify(
    std::span<const std::uint8_t, DILITHIUM_PUBLIC_KEY_SIZE> public_key,
    std::span<const std::uint8_t> message,
    std::span<const std::uint8_t, DILITHIUM_SIGNATURE_SIZE> signature) noexcept;

} // namespace pqcrypto
