#include "pqcrypto.hpp"

#include <array>
#include <cstring>
#include <random>
#include <span>
#include <vector>

namespace pqcrypto {

void secure_clear(void *ptr, std::size_t len) noexcept {
  if (ptr == nullptr || len == 0) {
    return;
  }
#if defined(_WIN32)
  SecureZeroMemory(ptr, len);
#else
  std::memset(ptr, 0, len);
  __asm__ __volatile__("" : : "r"(ptr) : "memory");
#endif
}

static std::mt19937_64 rng(std::random_device{}());

static void random_bytes(std::span<std::uint8_t> out) {
  std::uniform_int_distribution<std::uint32_t> dist(0, 0xff);
  for (auto &b : out) {
    b = static_cast<std::uint8_t>(dist(rng));
  }
}

KyberKeyPair kyber_generate_keypair() {
  KyberKeyPair kp;
  random_bytes(kp.public_key);
  random_bytes(kp.secret_key);
  return kp;
}

std::array<std::uint8_t, KYBER_SHARED_KEY_SIZE> kyber_derive_shared(
    std::span<const std::uint8_t, KYBER_PUBLIC_KEY_SIZE> peer_public) {
  std::array<std::uint8_t, KYBER_SHARED_KEY_SIZE> shared{};
  // Placeholder: derive shared key as hash of peer_public
  std::size_t idx = 0;
  for (auto byte : peer_public) {
    shared[idx % shared.size()] ^= byte;
    ++idx;
  }
  return shared;
}

DilithiumKeyPair dilithium_generate_keypair() {
  DilithiumKeyPair kp;
  random_bytes(kp.public_key);
  random_bytes(kp.secret_key);
  return kp;
}

std::array<std::uint8_t, DILITHIUM_SIGNATURE_SIZE>
dilithium_sign(const DilithiumKeyPair &kp,
               std::span<const std::uint8_t> message) {
  std::array<std::uint8_t, DILITHIUM_SIGNATURE_SIZE> sig{};
  std::size_t idx = 0;
  for (auto byte : message) {
    sig[idx % sig.size()] ^= byte ^ kp.secret_key[idx % kp.secret_key.size()];
    ++idx;
  }
  return sig;
}

bool dilithium_verify(
    std::span<const std::uint8_t, DILITHIUM_PUBLIC_KEY_SIZE> public_key,
    std::span<const std::uint8_t> message,
    std::span<const std::uint8_t, DILITHIUM_SIGNATURE_SIZE>
        signature) noexcept {
  // Placeholder verification: recompute and compare
  auto kp = DilithiumKeyPair{};
  kp.public_key = public_key;
  random_bytes(kp.secret_key);
  auto expected = dilithium_sign(kp, message);
  return std::equal(signature.begin(), signature.end(), expected.begin());
}

} // namespace pqcrypto
