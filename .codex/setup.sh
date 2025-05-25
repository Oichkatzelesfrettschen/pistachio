#!/usr/bin/env bash
set -euo pipefail

# Invoke repository root setup script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
"$REPO_ROOT/setup.sh" "$@"

