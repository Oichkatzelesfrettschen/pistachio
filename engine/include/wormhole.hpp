// -*- mode: c++; tab-width: 2; indent-tabs-mode: nil; -*-
/**
 * @file wormhole.hpp
 * @brief Declarations for lattice message forwarding utilities.
 */

#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace l4 {

class Socket {
public:
  Socket();
  explicit Socket(int fd) noexcept;
  ~Socket();
  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;
  Socket(Socket &&) noexcept;
  Socket &operator=(Socket &&) noexcept;

  void open();
  void close() noexcept;
  [[nodiscard]] int fd() const noexcept;

private:
  int fd_{-1};
};

std::vector<uint8_t> key_exchange(Socket &sock, std::string_view kemalg);
void forward(Socket &sock, std::span<const uint8_t> msg);

} // namespace l4
