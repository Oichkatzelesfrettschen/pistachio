// Translation of legacy Mach-based startup code to modern C++23
// This is a direct adaptation of the original K&R C implementation
// focusing on style and type safety. Hardware-specific behaviour has
// been retained as comments or minimal stubs.

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>

namespace machdep {

using mem_t = int;

// Example constants from the original environment. These would normally be
// provided by platform headers.
constexpr std::uint32_t USTART = 0x00000000; // placeholder
constexpr std::uint16_t LOW  = USTART & 0xFFFF;      // Low user start
constexpr std::uint16_t HIGH = (USTART >> 16) & 0xFFFF; // High user start
constexpr double MEMFACT = 0.8;                      // swap/free ratio

inline int keroff = 0;          // kernel memory offset
extern short segoff;            // MMU segment offset (from linker)

// ---------------------------------------------------------------------
// Bootstrap program used to exec init
// ---------------------------------------------------------------------
constexpr std::array<std::uint16_t, 26> icode{
    0x2E7C, HIGH, static_cast<std::uint16_t>(LOW + 0x100),
    0x227C, HIGH, static_cast<std::uint16_t>(LOW + 0x26),
    0x223C, HIGH, static_cast<std::uint16_t>(LOW + 0x22),
    0x207C, HIGH, static_cast<std::uint16_t>(LOW + 0x2A),
    0x42A7,
    0x303C, 0x003B,
    0x4E40,
    0x60FE,
    HIGH, static_cast<std::uint16_t>(LOW + 0x2A),
    0x0000, 0x0000,
    0x2F65, 0x7463, 0x2F69,
    0x6E69, 0x7400
};
constexpr std::size_t szicode = icode.size() * sizeof(std::uint16_t);

// ---------------------------------------------------------------------
// Machine dependent utility stubs
// ---------------------------------------------------------------------

[[maybe_unused]] bool iocheck(std::uintptr_t addr) {
    // Original code probed for an I/O device at the given address using
    // a fault handler. Here we simply check for a non-null pointer.
    return addr != 0;
}

enum class Access { RO, RW };

[[maybe_unused]] bool usraccess(std::uintptr_t virt, Access) {
    // Would consult the MMU for user permissions.
    (void)virt;
    return true;
}

[[maybe_unused]] std::uintptr_t vtop(std::uintptr_t virt) {
    // Convert virtual address to physical, accounting for segment offset.
    return virt - (static_cast<std::uintptr_t>(segoff) << 12);
}

[[maybe_unused]] int fuword(std::uintptr_t addr) {
    auto p = reinterpret_cast<const std::uint32_t*>(vtop(addr));
    return static_cast<int>(*p);
}

[[maybe_unused]] int fubyte(std::uintptr_t addr) {
    auto p = reinterpret_cast<const std::uint8_t*>(vtop(addr));
    return static_cast<int>(*p);
}

[[maybe_unused]] int suword(std::uintptr_t addr, std::uint32_t word) {
    auto p = reinterpret_cast<std::uint32_t*>(vtop(addr));
    *p = word;
    return 0;
}

[[maybe_unused]] int subyte(std::uintptr_t addr, std::uint8_t byte) {
    auto p = reinterpret_cast<std::uint8_t*>(vtop(addr));
    *p = byte;
    return 0;
}

[[maybe_unused]] int copyout(const void* from, void* to, std::size_t nbytes) {
    std::memcpy(to, from, nbytes);
    return 0;
}

[[maybe_unused]] int copyin(const void* from, void* to, std::size_t nbytes) {
    std::memcpy(to, from, nbytes);
    return 0;
}

[[maybe_unused]] void copyseg(int from, int to) {
    // In the original a `click` was copied using a block transfer.
    // Here we simply delegate to `std::memmove` with a fixed size.
    constexpr std::size_t click = 4096; // placeholder
    std::memmove(reinterpret_cast<void*>(to * click),
                 reinterpret_cast<const void*>(from * click),
                 click);
}

[[maybe_unused]] void clearseg(int where) {
    constexpr std::size_t click = 4096; // placeholder
    std::memset(reinterpret_cast<void*>(where * click), 0, click);
}

struct BusErrorFrame {
    std::array<long, 4> regs{};    // d0,d1,a0,a1
    int          baddr{};          // bsr return address
    short        fcode{};          // fault code
    int          aaddr{};          // fault address
    short        ireg{};           // instruction register
};

[[maybe_unused]] void busaddr(const BusErrorFrame& frame) {
    // Save information from a bus or address error.
    extern struct {
        short u_fcode;
        int   u_aaddr;
        short u_ireg;
    } u;
    u.u_fcode = frame.fcode;
    u.u_aaddr = frame.aaddr;
    u.u_ireg  = frame.ireg;
}

} // namespace machdep

