#!/usr/bin/env bash
set -euo pipefail

# Ensure essential Python tooling is present before running the full setup.
python3 -m pip install --user --upgrade pre-commit compiledb configuredb
pre-commit --version >/dev/null
compiledb --version >/dev/null
configuredb --help >/dev/null

# Invoke repository root setup script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
"$REPO_ROOT/setup.sh" "$@"

