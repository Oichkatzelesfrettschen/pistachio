#include "lattice_channel.hpp"
#include <stdio.h>
#include <typed_channel.h>

int main() {
  lattice_channel_t ch;
  lattice_channel_setup(&ch, L4_Myself());
  L4_Word_t words[1] = {42};
  exo_ipc_status st = typed_channel_send(&ch.base, words, 1);
  printf("typed_channel_send returned %d\n", static_cast<int>(st));
  return 0;
}
