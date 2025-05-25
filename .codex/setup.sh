#!/usr/bin/env bash
set -euo pipefail

# Install core build tools and helpers
apt-get update -y
apt-get install -y --no-install-recommends \
    clang clang-tools clang-tidy clangd clang-format \
    llvm lld lldb llvm-dev libclang-dev \
    ccache bolt lightning afl++ \
    build-essential cmake ninja-build pkg-config \
    python3 python3-pip python3-venv python3-dev python3-setuptools python3-wheel

# Ensure essential Python tooling is present
python3 -m pip install --upgrade pre-commit compiledb configuredb
pre-commit --version >/dev/null
compiledb --version >/dev/null
configuredb --help >/dev/null

# Invoke repository root setup script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
"$REPO_ROOT/setup.sh" "$@"

