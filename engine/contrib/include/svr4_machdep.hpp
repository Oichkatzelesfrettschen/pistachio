#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace svr4 {

// Register indices for general purpose registers on x86 when
// communicating with SVR4 user space components. Matches the
// traditional SVR4 layout but expressed using an enum class
// for type safety.
enum class Reg : std::size_t {
  GS,
  FS,
  ES,
  DS,
  EDI,
  ESI,
  EBP,
  ESP,
  EBX,
  EDX,
  ECX,
  EAX,
  TRAPNO,
  ERR,
  EIP,
  CS,
  EFL,
  UESP,
  SS,
  MAXREG
};

using greg_t = int;
using gregset_t = std::array<greg_t, static_cast<std::size_t>(Reg::MAXREG)>;

struct fregset_t {
  std::array<int, 62> x87{};     // x87 registers
  std::array<long, 33> weitek{}; // weitek registers
};

struct mcontext_t {
  gregset_t greg{};
  fregset_t freg{};
};

inline constexpr int UC_MACHINE_PAD = 5;

// Numbers used with the sysarch() system call
enum class Sysarch : int { FPHW = 40, DSCR = 75 };

struct ssd {
  unsigned int selector;
  unsigned int base;
  unsigned int limit;
  unsigned int access1;
  unsigned int access2;
};

// Processor trap identifiers
enum class Trap : int {
  DIVIDE = 0,
  TRCTRAP = 1,
  NMI = 2,
  BPTFLT = 3,
  OFLOW = 4,
  BOUND = 5,
  PRIVINFLT = 6,
  DNA = 7,
  DOUBLEFLT = 8,
  FPOPFLT = 9,
  TSSFLT = 10,
  SEGNPFLT = 11,
  STKFLT = 12,
  PROTFLT = 13,
  PAGEFLT = 14,
  ALIGNFLT = 17
};

// Fast system call gate identifiers
enum class FastTrap : int {
  FNULL = 0,      // Null trap, for testing
  FGETFP = 1,     // Get emulated FP context
  FSETFP = 2,     // Set emulated FP context
  GETHRTIME = 3,  // implements gethrtime(2)
  GETHRVTIME = 4, // implements gethrvtime(2)
  GETHRESTIME = 5 // clock_gettime(CLOCK_REALTIME, tp)
};

struct proc; // forward declaration from the kernel

void syscall_intern(proc *);

} // namespace svr4
