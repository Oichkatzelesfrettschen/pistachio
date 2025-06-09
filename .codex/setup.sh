#!/usr/bin/env bash
set -euo pipefail
# Auto-heal: ensure strict mode and invoke repo setup

# Install core build tools and helpers
apt-get update -y
apt-get install -y --no-install-recommends \
    clang clang-tools clang-tidy clangd clang-format \
    llvm lld lldb llvm-dev libclang-dev \
    ccache bolt afl++ \
    build-essential cmake ninja-build pkg-config \
    python3 python3-pip python3-venv python3-dev python3-setuptools python3-wheel

# Ensure essential Python tooling is present
# Work around Debian's PEP 668 restrictions by allowing pip to alter
# the system installation when creating the development environment.
python3 -m pip install --upgrade --break-system-packages \
    pre-commit compiledb 
export PATH="$(python3 -m site --user-base)/bin:$PATH"
pre-commit --version >/dev/null
compiledb --help >/dev/null

# Invoke repository root setup script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
"$REPO_ROOT/setup.sh" "$@"

