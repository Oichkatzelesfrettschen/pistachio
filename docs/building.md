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
libraries from `src-userland/lib`.

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

