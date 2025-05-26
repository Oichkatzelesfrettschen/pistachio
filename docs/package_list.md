# Package List

This document lists packages installed by `setup.sh` and `.codex/setup.sh` along with their sources.

## apt packages

The following packages are installed via `apt-get`:

- build-essential, gcc, g++, clang, lld, llvm
- clang-format, clang-tidy, uncrustify, astyle, editorconfig, pre-commit
- make, bmake, ninja-build, cmake, meson
- autoconf, automake, libtool, m4, gawk, flex, bison, byacc
- pkg-config, file, ca-certificates, curl, git, unzip
- libopenblas-dev, liblapack-dev, libeigen3-dev
- strace, ltrace, linux-perf, systemtap, systemtap-sdt-dev, crash
- valgrind, kcachegrind, trace-cmd, kernelshark
- libasan6, libubsan1, likwid, hwloc
- shellcheck, graphviz, doxygen, python3-sphinx, llvm-17-tools, capnproto

### Python related packages from apt
- python3, python3-pip, python3-dev, python3-venv, python3-wheel
- python3-numpy, python3-scipy, python3-pandas
- python3-matplotlib, python3-scikit-learn
- python3-torch, python3-torchvision, python3-torchaudio
- python3-onnx, python3-onnxruntime

### QEMU and virtualization tools
- qemu-user-static
- qemu-system-x86, qemu-system-arm, qemu-system-aarch64
- qemu-system-riscv64, qemu-system-ppc, qemu-system-ppc64, qemu-utils

### Cross-compilers
- bcc, bin86, elks-libc
- gcc-ia64-linux-gnu, g++-ia64-linux-gnu
- gcc-i686-linux-gnu, g++-i686-linux-gnu
- gcc-aarch64-linux-gnu, g++-aarch64-linux-gnu
- gcc-arm-linux-gnueabi, g++-arm-linux-gnueabi
- gcc-arm-linux-gnueabihf, g++-arm-linux-gnueabihf
- gcc-riscv64-linux-gnu, g++-riscv64-linux-gnu
- gcc-powerpc-linux-gnu, g++-powerpc-linux-gnu
- gcc-powerpc64-linux-gnu, g++-powerpc64-linux-gnu
- gcc-powerpc64le-linux-gnu, g++-powerpc64le-linux-gnu
- gcc-m68k-linux-gnu, g++-m68k-linux-gnu
- gcc-hppa-linux-gnu, g++-hppa-linux-gnu
- gcc-loongarch64-linux-gnu, g++-loongarch64-linux-gnu
- gcc-mips-linux-gnu, g++-mips-linux-gnu
- gcc-mipsel-linux-gnu, g++-mipsel-linux-gnu
- gcc-mips64-linux-gnuabi64, g++-mips64-linux-gnuabi64
- gcc-mips64el-linux-gnuabi64, g++-mips64el-linux-gnuabi64

### High level language toolchains
- golang-go, nodejs, npm, typescript
- rustc, cargo, clippy, rustfmt
- lua5.4, liblua5.4-dev, luarocks
- ghc, cabal-install, hlint, stylish-haskell
- sbcl, ecl, clisp, cl-quicklisp, slime, cl-asdf
- ldc, gdc, dmd-compiler, dub, libphobos-dev
- chicken-bin, libchicken-dev, chicken-doc
- openjdk-17-jdk, maven, gradle, dotnet-sdk-8, mono-complete
- swift, swift-lldb, swiftpm, kotlin, gradle-plugin-kotlin
- ruby, ruby-dev, gem, bundler, php-cli, php-dev, composer, phpunit
- r-base, r-base-dev, dart, flutter, gnat, gprbuild, gfortran, gnucobol
- fpc, lazarus, zig, nim, nimble, crystal, shards, gforth

### GUI and desktop packages
- libqt5-dev, qtcreator, libqt6-dev
- libgtk1.2-dev, libgtk2.0-dev, libgtk-3-dev, libgtk-4-dev
- libfltk1.3-dev, xorg-dev, libx11-dev, libxext-dev
- libmotif-dev, openmotif, cde
- xfce4-dev-tools, libxfce4ui-2-dev, lxde-core, lxqt-dev-tools
- libefl-dev, libeina-dev
- libwxgtk3.0-dev, libwxgtk3.0-gtk3-dev
- libsdl2-dev, libsdl2-image-dev, libsdl2-ttf-dev
- libglfw3-dev, libglew-dev

### Containers and HPC
- docker.io, podman, buildah, virt-manager, libvirt-daemon-system, qemu-kvm
- gdb, lldb, perf, gcovr, lcov, bcc-tools, bpftrace
- openmpi-bin, libopenmpi-dev, mpich

### Theorem provers and modeling tools
- coq, coqide, coq-theories, libcoq-ocaml-dev
- ocaml, ocaml-findlib
- agda, agda-stdlib, agda-mode
- isabelle

## NPM global packages
- npm@latest
- prettier
- eslint

## Python packages installed via pip
- tensorflow-cpu, jax, jaxlib
- tensorflow-model-optimization, mlflow, onnxruntime-tools
- pre-commit, compiledb, configuredb, pytest, pyyaml, pylint, pyfuzz

## Tools downloaded directly (GitHub releases)
- TLA+ tools (tlaplus.zip)
- IA-16 cross-compiler (ia16-elf-gcc)
- protoc compiler

These packages are installed by the repository `setup.sh`. On failure of `apt-get` installs, the script attempts installation via `pip` or `npm`.
