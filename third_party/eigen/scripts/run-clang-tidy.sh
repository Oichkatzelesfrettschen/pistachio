#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=${1:-build}
if [ ! -f "${BUILD_DIR}/compile_commands.json" ]; then
  echo "compile_commands.json not found in ${BUILD_DIR}" >&2
  echo "Generate it with: cmake -B ${BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .." >&2
  exit 1
fi

FILES=$(git ls-files '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hpp')
for f in $FILES; do
  clang-tidy -p "${BUILD_DIR}" "$f"
done
