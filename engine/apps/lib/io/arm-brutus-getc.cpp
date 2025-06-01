#include "brutus-uart.h"
#include <cstdint>

// Returns a single character read from the UART. Spins until data is available.
inline std::uint8_t getc() {
  volatile std::uint32_t status = 0; // Hardware status register value
  // Wait until the receiver FIFO is not empty.
  do {
    status = L4_UART_UTSR1;
  } while ((status & L4_UART_RNE) == 0);

  // Fetch the character from the UART data register.
  return static_cast<std::uint8_t>(L4_UART_UTDR);
}
