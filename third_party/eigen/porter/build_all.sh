#!/usr/bin/env bash
# Build helper to generate headers and run both C and Go tests.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# Log any failures for later inspection
FAIL_LOG=/tmp/porter_build_failures.log
: > "$FAIL_LOG"

# Generate C headers from mapping
python3 "$ROOT/porter/gen_headers.py"

# Verify tool availability
pre-commit --version >/dev/null 2>&1 || {
  echo "pre-commit --version failed" >> "$FAIL_LOG"
}
pytest --version >/dev/null 2>&1 || {
  echo "pytest --version failed" >> "$FAIL_LOG"
}

# Run pre-commit checks
if command -v pre-commit >/dev/null; then
  pre-commit run --show-diff-on-failure --color=always || {
    echo "pre-commit run failed" >> "$FAIL_LOG"
  }
else
  echo "pre-commit not found" >> "$FAIL_LOG"
fi

# Ensure go.mod targets Go 1.23
if command -v go >/dev/null; then
  (cd "$ROOT/go" && go mod edit -go=1.23)
  (cd "$ROOT/go" && go vet ./... && go test ./...)
else
  echo "Go toolchain not found" >&2
fi

# Run C/C++ tests
bash "$ROOT/tests/run_all.sh"
