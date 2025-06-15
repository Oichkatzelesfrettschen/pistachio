// -*- mode: c++; tab-width: 2; indent-tabs-mode: nil; -*-
/**
 * @file wormhole.cpp
 * @brief Forward lattice messages over the network using post-quantum
 *        key exchange.
 */
#include "../include/wormhole.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

extern "C" {
#include <oqs/kem.h>
}

namespace l4 {

/**
 * @brief Simple RAII wrapper for a TCP socket.
 */
class Socket {
public:
  /// Construct an invalid socket.
  Socket() = default;

  explicit Socket(int fd) noexcept : fd_{fd} {}
  /// Destructor closes the socket if open.
  ~Socket() { close(); }

  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  Socket(Socket &&other) noexcept : fd_{other.fd_} { other.fd_ = -1; }
  Socket &operator=(Socket &&other) noexcept {
    if (this != &other) {
      close();
      fd_ = other.fd_;
      other.fd_ = -1;
    }
    return *this;
  }

  /// Create a TCP socket.
  void open() {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
      throw std::system_error{errno, std::generic_category(), "socket"};
    }
  }

  /// Close the socket if open.
  void close() noexcept {
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
  }

  /// Return the underlying file descriptor.
  [[nodiscard]] int fd() const noexcept { return fd_; }

private:
  int fd_{-1};
};

/**
 * @brief Establish a PQCrypto-secured channel and return the shared secret.
 *
 * The connection follows a simple KEM-based handshake using liboqs. The
 * initiator sends its public key and receives the encapsulated secret.
 * Both peers derive an identical shared secret which is then used to
 * authenticate lattice messages.
 *
 * @param sock   connected TCP socket
 * @param kemalg KEM algorithm name, e.g. "Kyber512".
 *
 * @return derived shared secret bytes
 */
std::vector<uint8_t> key_exchange(Socket &sock, std::string_view kemalg) {
  OQS_KEM *kem = OQS_KEM_new(kemalg.data());
  if (!kem) {
    throw std::runtime_error{"KEM not supported"};
  }
  std::vector<uint8_t> pub_key(kem->length_public_key);
  std::vector<uint8_t> sec_key(kem->length_secret_key);
  if (OQS_KEM_keypair(kem, pub_key.data(), sec_key.data()) != OQS_SUCCESS) {
    OQS_KEM_free(kem);
    throw std::runtime_error{"keypair failed"};
  }

  // send public key
  if (::send(sock.fd(), pub_key.data(), pub_key.size(), 0) !=
      static_cast<ssize_t>(pub_key.size())) {
    OQS_KEM_free(kem);
    throw std::system_error{errno, std::generic_category(), "send"};
  }

  std::vector<uint8_t> ciphertext(kem->length_ciphertext);
  std::vector<uint8_t> shared_secret(kem->length_shared_secret);
  if (::recv(sock.fd(), ciphertext.data(), ciphertext.size(), MSG_WAITALL) !=
      static_cast<ssize_t>(ciphertext.size())) {
    OQS_KEM_free(kem);
    throw std::system_error{errno, std::generic_category(), "recv"};
  }
  if (OQS_KEM_decaps(kem, shared_secret.data(), ciphertext.data(),
                     sec_key.data()) != OQS_SUCCESS) {
    OQS_KEM_free(kem);
    throw std::runtime_error{"decapsulation failed"};
  }
  OQS_KEM_free(kem);
  return shared_secret;
}

/**
 * @brief Forward the given message to the peer.
 *
 * The message is prefixed with its length encoded as a 32-bit integer in
 * network byte order. Callers must ensure the connection has been secured
 * via @ref key_exchange.
 *
 * @param sock TCP socket with an established shared secret
 * @param msg  opaque lattice message bytes
 */
void forward(Socket &sock, std::span<const uint8_t> msg) {
  uint32_t len = htonl(static_cast<uint32_t>(msg.size()));
  std::array<iovec, 2> vec{
      {{&len, sizeof(len)}, {const_cast<uint8_t *>(msg.data()), msg.size()}}};
  if (::writev(sock.fd(), vec.data(), vec.size()) < 0) {
    throw std::system_error{errno, std::generic_category(), "writev"};
  }
}

} // namespace l4
