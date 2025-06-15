// -*- mode: c++; tab-width: 2; indent-tabs-mode: nil; -*-
/**
 * @file main.cc
 * @brief User-level daemon that forwards lattice messages over TCP.
 */

#include "../../../include/wormhole.hpp"

#include <cstring>
#include <iostream>
#include <vector>

using namespace l4;

/**
 * @brief Entry point for the wormhole daemon.
 *
 * Usage: wormhole_daemon <port>
 */
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: wormhole_daemon <port>\n";
    return 1;
  }

  int port = std::stoi(argv[1]);
  Socket listener;
  listener.open();

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(listener.fd(), reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) <
      0) {
    perror("bind");
    return 1;
  }
  if (listen(listener.fd(), 1) < 0) {
    perror("listen");
    return 1;
  }

  std::cout << "Waiting for connection on port " << port << "..." << std::endl;
  int cfd = accept(listener.fd(), nullptr, nullptr);
  if (cfd < 0) {
    perror("accept");
    return 1;
  }

  Socket client{cfd};
  std::vector<uint8_t> secret;
  try {
    secret = key_exchange(client, "Kyber512");
  } catch (const std::exception &e) {
    std::cerr << "key exchange failed: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "Established shared secret of " << secret.size() << " bytes\n";

  while (true) {
    uint32_t len = 0;
    if (recv(client.fd(), &len, sizeof(len), MSG_WAITALL) != sizeof(len))
      break;
    len = ntohl(len);
    std::vector<uint8_t> msg(len);
    if (recv(client.fd(), msg.data(), len, MSG_WAITALL) !=
        static_cast<int>(len))
      break;
    forward(client, msg);
  }
  return 0;
}
