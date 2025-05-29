#!/usr/bin/env bash
# Simple helper printing files changed since a given git commit.

set -euo pipefail

base="${1:-HEAD^}"
git diff --name-only "$base"
