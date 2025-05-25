// Example use of the C++23 SVR4 compatibility header.
// This demonstrates how the modernised definitions can be
// consumed from regular C++ code.

#include <iostream>
#include <svr4_machdep.hpp>

int main() {
    svr4::mcontext_t ctx{};
    ctx.greg[static_cast<std::size_t>(svr4::Reg::EIP)] = 0xdeadbeef;
    std::cout << "EIP: 0x" << std::hex
              << ctx.greg[static_cast<std::size_t>(svr4::Reg::EIP)]
              << std::dec << '\n';
}
