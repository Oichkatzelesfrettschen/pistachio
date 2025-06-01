#!/usr/bin/env bash
set -euo pipefail

# Simple wrapper to configure, build and test using CMake.
BUILD_DIR=${BUILD_DIR:-build}

# Determine the compiler. Prefer Clang but gracefully fall back to GCC.
if command -v clang >/dev/null 2>&1 && command -v clang++ >/dev/null 2>&1; then
  # Use the Clang toolchain when available.
  export CC=clang
  export CXX=clang++
elif command -v gcc >/dev/null 2>&1 && command -v g++ >/dev/null 2>&1; then
  # Inform the user about the fallback to GCC.
  echo "clang not found, falling back to gcc" >&2
  export CC=gcc
  export CXX=g++
else
  # Abort if no suitable compiler exists.
  echo "error: clang/clang++ not found and gcc/g++ unavailable" >&2
  exit 1
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -G Ninja ..
cmake --build .
ctest --output-on-failure
