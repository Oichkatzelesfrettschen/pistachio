#!/usr/bin/env bash
set -euo pipefail

# Simple wrapper to configure, build and test using CMake
BUILD_DIR=${BUILD_DIR:-build}

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -G Ninja ..
cmake --build .
ctest --output-on-failure
