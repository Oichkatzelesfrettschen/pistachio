#!/usr/bin/env python3
"""Pre-commit hook to ensure only permitted source languages are used."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

# Allowed file extensions for C and assembly sources
ALLOWED_EXTS = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".h",
    ".hpp",
    ".hxx",
    ".hh",
    ".s",
    ".S",
    ".asm",
}

def main(argv: list[str] | None = None) -> int:
    argv = argv or []
    if argv:
        files = argv
    else:
        result = subprocess.run(
            ["git", "diff", "--cached", "--name-only"],
            capture_output=True,
            text=True,
            check=False,
        )
        files = result.stdout.splitlines()

    disallowed = [f for f in files if Path(f).suffix not in ALLOWED_EXTS]
    if disallowed:
        print("Disallowed file types detected. Only C/C++23 and assembly sources "
              "are permitted:")
        for f in disallowed:
            print(f"  {f}")
        return 1
    return 0

if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
