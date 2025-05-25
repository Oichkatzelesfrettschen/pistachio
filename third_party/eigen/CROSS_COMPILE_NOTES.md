# Cross Compilation Notes

This repository now installs an extensive set of cross-compilation toolchains via `setup.sh`.
RISC-V (`riscv64-linux-gnu`) and LoongArch (`loongarch64-linux-gnu`) architectures are detected in
`Eigen/src/Core/util/Macros.h`. Vectorization stubs for LASX/LSX and RISC-V RVV were also added.

Missing pieces:

* QEMU for LoongArch is not available in the base environment, so tests for that
  architecture are skipped.
* Only a minimal arithmetic kernel is tested under cross-compilation. Wider
  coverage would require porting more of Eigen's tests.

