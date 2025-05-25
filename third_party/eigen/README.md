# Eigen Modernisation and Experimental C/Go Frontends

This fork modernises the Eigen codebase to **C++23** while also providing
experimental frontends for **C23** and **Go**. The traditional C++ headers are
kept in `Eigen/` and gradually updated to the new standard.  In parallel the
C23 interface ("EigenC") is generated under `eigenc/` and a Go module lives in
`go/`. Makefiles and CMake scripts are being updated to accommodate all three
variants.

The translation process lives in `porter/` and outputs headers under
`eigenc/include`.  These C headers implement basic matrix operations with
`static inline` functions and `_Generic` dispatch.

## Generating Headers

Run `porter/gen_headers.py` from the repository root after editing
`porter/mapping.yaml` with the template instantiations you need:

```bash
python3 porter/gen_headers.py
```

The script produces `eigenc/include/ec_generated.h` which is included from
`ec_core.h`.

## Porter Workflow

To regenerate the experimental headers and run the full test suite:

```bash
python3 porter/scan_templates.py
python3 porter/gen_headers.py
bash porter/build_all.sh
```

If Clang is installed in a non-standard location, set the `LIBCLANG_PATH`
environment variable so `scan_templates.py` can find `libclang.so`.

## Running Tests

The test suite can be executed via `tests/run_all.sh`.  It builds a small example
both as C and as C++ and compares the numerical results.  Cross-compilation for
PowerPC is exercised if the toolchain and QEMU are available.

```bash
bash tests/run_all.sh
```

## Using `ec_Matrixf32`

Below is a minimal example demonstrating how to use the generated C API:

```c
#include "ec_core.h"

int main(void) {
    float a_data[4] = {1,2,3,4};
    float b_data[4] = {5,6,7,8};
    float c_data[4];
    ec_Matrixf32 A = {2,2,a_data};
    ec_Matrixf32 B = {2,2,b_data};
    ec_Matrixf32 C = {2,2,c_data};
    ec_addf32(&A, &B, &C);
    return 0;
}
```

## Cross Compilation

The repository provides cross-compilation checks for PowerPC and RISC-V.
`tests/run_all.sh` builds the test program with the corresponding cross
compiler and runs it under QEMU. Specifically, it uses
`powerpc-linux-gnu-gcc` with `qemu-ppc-static` and
`riscv64-linux-gnu-gcc` with `qemu-riscv64-static`. The emulation step is
skipped if either the toolchain or QEMU binary is missing. If 32-bit
development libraries are available, the same script also attempts a 32-bit x86
build using `gcc -m32`.

## Go Bindings

Bindings for Go are generated in `go/ec`. The module targets **Go 1.23** and
mirrors the C API produced under `eigenc/include`. After running the porter
scripts you can execute the Go tests:

```bash
cd go && go test ./...
```

The convenience script `porter/build_all.sh` will regenerate headers and run the
entire suite including the Go tests.

## Development Environment

Run `./setup.sh` to install the toolchain including GCC, Clang/LLVM, Meson and
pre-commit.  After installation you can enable the git hooks with:

```bash
pre-commit install
pre-commit run --all-files
```

Clang-Tidy configurations for both C23 and C++23 live in `.clang-tidy-c23` and
`.clang-tidy`.  The project builds with either GCC or Clang; the test scripts
use whichever compiler is available.

### Running clang-tidy

Generate a build directory with a compile database:

```bash
mkdir build && cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Then execute the helper script which runs clang-tidy across the tree:

```bash
../scripts/run-clang-tidy.sh
```



