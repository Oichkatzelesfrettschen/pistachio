# Building with CMake

The repository now provides a small CMake build that mirrors the legacy
Makefiles for the kernel and userland libraries.  It wraps the existing
`make` based build system so the traditional workflow still works.

## Quick start

```bash
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
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

=======
# Building Pistachio

This guide summarises how to build the kernel and user land with modern
C23 and C++23 compilers.  GCC 14 and Clang 17 are known to work.  When
building from a repository checkout run `pre-commit install --install-hooks`
so that formatting and linting tools run automatically.

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

4. Build the kernel with `make`.  On PowerPC a minimal QEMU command line is
   `qemu-system-ppc -M g3beige -kernel kernel/powerpc-kernel`.

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

Further notes on PowerPC cross compilation can be found in
`doc/notes/ppc-build.txt`.

The `contrib/include` directory also provides `svr4_machdep.hpp`, a
header that translates the historic SVR4 machine dependencies into a
typed C++23 interface.  A short usage sample lives in
`docs/svr4_machdep_cpp23.cpp`.

See the top-level LICENSE file for the project's terms.
