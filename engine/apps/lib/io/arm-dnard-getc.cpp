#include "arm-dnard-uart.h"
#include <cstdint>

// Reads a character from the DNARD UART.
inline std::uint8_t getc() {
  // Spin until a character is available.
  while ((STATUS(COM1) & 0x01) == 0) {
  }
  return static_cast<std::uint8_t>(DATA(COM1));
}
