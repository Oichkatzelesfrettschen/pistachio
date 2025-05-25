# i16 Real Mode Port

The i16 architecture is a minimal real mode port used mainly for build
and toolchain tests.  It targets 16‑bit x86 using GCC's `-m16` option
or a cross compiler such as `m16c-elf-gcc`.

Only a few files are implemented.  `context_switch` performs a very
simple jump to the new stack and instruction pointer while
`interrupt_entry` immediately executes an `iret`.

To build the kernel use a toolchain that can generate 16‑bit code.  A
Debian package for `m16c-elf-gcc` works well:

```bash
$ sudo apt install m16c-elf-gcc
$ make -C kernel BUILDDIR=build-i16 TOOLPREFIX=m16c-elf- ARCH=i16 SUBARCH=x16
```

The resulting image boots in real mode.  It does not provide any of the
advanced features present on 32‑ or 64‑bit targets, but is useful for
educational purposes and for verifying the build system.
