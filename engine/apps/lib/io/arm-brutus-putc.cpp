#include "brutus-uart.h"
#include <cstdint>

// Sends a character through the UART. Spins until a transmit slot is free.
inline void putc(int c) {
  volatile std::uint32_t status = 0; // Hardware status register value
  // Wait until the transmitter FIFO has space.
  do {
    status = L4_UART_UTSR1;
  } while ((status & L4_UART_TNF) == 0);

  // Write the character to the UART.
  L4_UART_UTDR = static_cast<std::uint32_t>(c);

  if (c == '\n')
    putc('\r');
}
