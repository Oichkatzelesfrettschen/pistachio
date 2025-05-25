#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/.."; pwd)"

# Native build

gcc -std=c23 -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.c" -o /tmp/ec_test \
  || gcc -std=c2x -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.c" -o /tmp/ec_test

g++ -std=c++23 -I"${ROOT_DIR}" -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.cpp" -o /tmp/eigen_test23

/tmp/ec_test
/tmp/eigen_test23

# SpinLock test (C++23 only)
g++ -std=c++23 -pthread -I"${ROOT_DIR}" "${ROOT_DIR}/tests/spinlock/test_spinlock.cpp" -o /tmp/spinlock_test
/tmp/spinlock_test

python3 "${ROOT_DIR}/tests/compare_eigen_cpp_vs_c.py"
cp /tmp/c_out.txt /tmp/c_out_host.txt

echo "\xE2\x9C\x93  native EigenC vs Eigen checks passed"

# 32-bit x86 build using -m32 if available
if echo 'int main(){}' | gcc -m32 -xc -o /tmp/ccheck32 - >/dev/null 2>&1; then
  rm /tmp/ccheck32
  gcc -m32 -std=c23 -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.c" -o /tmp/ec_test_x86 \
    || gcc -m32 -std=c2x -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.c" -o /tmp/ec_test_x86
  /tmp/ec_test_x86
  mv /tmp/c_out.txt /tmp/c_out_x86.txt
  diff -u /tmp/c_out_host.txt /tmp/c_out_x86.txt
  echo "\xE2\x9C\x93  cross-compiled EigenC checks passed (x86-32)"
else
  echo "Skipping x86 32-bit cross-compile test: toolchain not found" >&2
fi

# Cross-compile for RISC-V and run via qemu if toolchain present
if command -v riscv64-linux-gnu-gcc >/dev/null && command -v qemu-riscv64-static >/dev/null; then
  riscv64-linux-gnu-gcc -static -std=c23 -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.c" -o /tmp/ec_test_riscv \
    || riscv64-linux-gnu-gcc -static -std=c2x -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.c" -o /tmp/ec_test_riscv

  qemu-riscv64-static /tmp/ec_test_riscv
  mv /tmp/c_out.txt /tmp/c_out_riscv.txt

  diff -u /tmp/c_out_host.txt /tmp/c_out_riscv.txt

  echo "\xE2\x9C\x93  cross-compiled EigenC checks passed (riscv64)"
else
  echo "Skipping RISC-V cross-compile test: toolchain or QEMU not found" >&2
fi

# Cross-compile for PowerPC and run via qemu if toolchain present
if command -v powerpc-linux-gnu-gcc >/dev/null && command -v qemu-ppc-static >/dev/null; then
  powerpc-linux-gnu-gcc -static -std=c23 -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.c" -o /tmp/ec_test_ppc \
    || powerpc-linux-gnu-gcc -static -std=c2x -I"${ROOT_DIR}/eigenc/include" "${ROOT_DIR}/tests/core/test_core.c" -o /tmp/ec_test_ppc

  qemu-ppc-static /tmp/ec_test_ppc
  mv /tmp/c_out.txt /tmp/c_out_ppc.txt

  diff -u /tmp/c_out_host.txt /tmp/c_out_ppc.txt

  echo "\xE2\x9C\x93  cross-compiled EigenC checks passed (powerpc)"
else
  echo "Skipping PowerPC cross-compile test: toolchain or QEMU not found" >&2
fi


# Run Go tests if Go is available
if command -v go >/dev/null; then
  (cd "${ROOT_DIR}/go" && go test ./...)
  echo "\xE2\x9C\x93  Go bindings tests passed"
else
  echo "Skipping Go tests: go not found" >&2
fi
