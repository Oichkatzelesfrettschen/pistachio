# Building with CMake

The repository now provides a small CMake build that mirrors the legacy
Makefiles for the kernel and userland libraries.  It wraps the existing
`make` based build system so the traditional workflow still works.

## Quick start

### Unified build helper

Use `scripts/unified_build.sh` to configure the build directory, compile all targets and run the tests in one step.

```bash
$ scripts/unified_build.sh
```


```bash
$ mkdir build && cd build
$ cmake -G Ninja ..
$ cmake --build .
$ ctest --output-on-failure
```

The default target compiles the example `string.o` object, builds the
 kernel using the Makefile under `kernel/` and builds the userland
 libraries from `user/lib`.

To build individual pieces you can specify the targets explicitly:

```bash
$ cmake --build . --target kernel_target
$ cmake --build . --target userlib_target
```

Cleaning can be performed with the matching clean targets:

```bash
$ cmake --build . --target kernel_clean
$ cmake --build . --target userlib_clean
```

These commands simply invoke the existing Makefiles so any custom
settings in `kernel/Makeconf.local` or environment variables are still
honoured.

### Cross compiling

`cmake` can forward build variables to these Makefiles.  The most
useful are `TOOLPREFIX`, `ARCH` and `SUBARCH`:

```bash
$ cmake -DTOOLPREFIX=/usr/bin/powerpc-linux-gnu- \
        -DARCH=powerpc -DSUBARCH=none ..
$ cmake --build .
```

Adjust the values for your toolchain and desired target.

# Building Pistachio

This guide summarises how to build the kernel and user land using the
LLVM toolchain.  A recent Clang (version 17 or newer) is required and
`ld.lld` is used for linking.  When building from a repository checkout
run `pre-commit install --install-hooks` so that formatting and linting
tools run automatically.

## Kernel

1. Create a build directory using the kernel Makefile:

   ```sh
   cd kernel
   make BUILDDIR=/path/to/build-dir
   cd /path/to/build-dir
   make menuconfig
   ```

2. Select `SUBARCH=x32` for 32-bit or `SUBARCH=x64` for 64‑bit targets in
   `Makeconf.local`.

3. For cross compilation set `TOOLPREFIX` to your compiler prefix, for
   example:

   ```make
   TOOLPREFIX=/usr/bin/powerpc-linux-gnu-
   ```

4. Build the kernel with `make`.

## User land

1. From the `user` directory run Autotools if the build system was
   regenerated:

   ```sh
   autoreconf -i
   ```

2. Create a separate build directory and configure:

   ```sh
   mkdir build && cd build
   ../configure --host=i686-pc-linux-gnu   # or another target triple
   make
   ```

   For Clang-based cross builds, set the compiler variables explicitly:

   ```sh
   export CC="clang --target=powerpc-linux-gnu"
   export CXX="clang++ --target=powerpc-linux-gnu"
   export AR=llvm-ar
   export LD=ld.lld
   ```

3. Install the built binaries with `make install` if desired.

## Cross-compilation

The kernel and user land can be built for other architectures using
standard cross compilers.  Modern GCC and Clang toolchains from Debian or
Ubuntu packages work well.

### PowerPC example

Install `gcc-powerpc-linux-gnu` and `g++-powerpc-linux-gnu` or use Clang
with the LLVM binutils:

```sh
$ sudo apt install gcc-powerpc-linux-gnu g++-powerpc-linux-gnu binutils-powerpc-linux-gnu

# Clang variant
export CC="clang --target=powerpc-linux-gnu"
export CXX="clang++ --target=powerpc-linux-gnu"
export AR=llvm-ar
export LD=ld.lld
```

Configure the kernel in a separate directory and set `TOOLPREFIX` in
`Makeconf.local`:

```make
TOOLPREFIX=/usr/bin/powerpc-linux-gnu-
```

Build the kernel with `make`.  To run under QEMU:

```sh
qemu-system-ppc -M g3beige -kernel kernel/powerpc-kernel
```

For 64‑bit targets use `qemu-system-ppc64`.

### x86 and ARM examples

Cross compiling to 32‑bit x86 uses the `i686` target triple:

```sh
../configure --host=i686-linux-gnu CC=i686-linux-gnu-gcc CXX=i686-linux-gnu-g++
```

ARM builds are similar.  For 64‑bit ARM:

```sh
../configure --host=aarch64-linux-gnu CC=aarch64-linux-gnu-gcc \
             CXX=aarch64-linux-gnu-g++
```

These examples mirror the toolchains installed by `setup.sh`.

### i16 example

The minimal real mode port builds with GCC's m16c cross compiler.  After
installing `m16c-elf-gcc` the kernel can be compiled with:

```bash
$ make -C kernel BUILDDIR=build-i16 TOOLPREFIX=m16c-elf- ARCH=i16 SUBARCH=x16
```

### CPU tuning flags

Both the CMake and Makefile builds use `-march=native` by default.  The
target CPU can be overridden with the `TUNE_CPU` option when invoking
CMake or by setting `CPU_CFLAGS` for the Makefile.

**x86 example**

```bash
$ cmake -DTUNE_CPU=skylake ..
$ make CPU_CFLAGS="-march=skylake"
```

**ARM example**

```bash
$ cmake -DTUNE_CPU=cortex-a53 ..
$ make CPU_CFLAGS="-march=armv8-a"
```

**PowerPC example**

```bash
$ cmake -DTUNE_CPU=power9 ..
$ make CPU_CFLAGS="-march=power9"
```

The `contrib/include` directory also provides `svr4_machdep.hpp`, a
header that translates the historic SVR4 machine dependencies into a
typed C++17 interface.  A short usage sample lives in
`docs/svr4_machdep_cpp23.cpp`.

### Calling conventions

Both the kernel and user land use explicit calling convention attributes.
The header `l4/compiler.h` defines macros such as `L4_CDECL` and
`L4_FASTCALL` which expand to the GNU style attributes `[[gnu::cdecl]]` and
`[[gnu::fastcall]]`.  All exported kernel entry points and public API
functions are annotated with these macros to make the required ABI
visible in the source code.

See the top-level LICENSE file for the project's terms.

## Running tests

The repository provides a small test suite consisting of Python unit tests,
a C stress test for the ticket-lock implementation and a pair of simple
POSIX programs under `tests/posix`.  The Makefile exposes a `check` target
which builds these programs and runs all tests:

```bash
$ make check
```

When using CMake, invoke the `tests` target:

```bash
$ cmake --build . --target tests
./spinlock_fairness
./posix_test_file
./posix_test_process
```

The `spinlock_fairness` binary spawns multiple threads, measures how many times
each thread acquires the lock for a short period and reports the counts.  It
exits with a non-zero status if the slowest thread obtained less than 80% of the
acquisitions of the fastest thread.
