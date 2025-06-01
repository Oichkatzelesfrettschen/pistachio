#include "arm-dnard-uart.h"
#include <cstdint>

// Writes a character to the DNARD UART.
inline void putc(int c) {
  // Spin until the transmitter is ready.
  while ((STATUS(COM1) & 0x60) == 0) {
  }
  DATA(COM1) = c;
  if (c == '\n')
    putc('\r');
}
