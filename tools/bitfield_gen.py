#!/usr/bin/env python3
"""Generate C macros for simple bitfield definitions.

The script reads a YAML file describing named fields and their bit ranges
and emits ``#define`` statements that encode masks and shift constants.

The YAML format is a mapping of field names to ``[start, end]`` bit
positions (inclusive).  The least significant bit is ``0``.

Example
-------

.. code-block:: yaml

   FIELD_A: [0, 3]
   FIELD_B: [4, 7]

Running ``bitfield_gen.py spec.yml`` will produce macros of the form::

   #define FIELD_A_SHIFT 0
   #define FIELD_A_MASK  0x0000000f

   #define FIELD_B_SHIFT 4
   #define FIELD_B_MASK  0x000000f0
"""

from __future__ import annotations

import sys
from pathlib import Path
from typing import Mapping, Sequence

try:
    import yaml  # type: ignore
except Exception:  # pragma: no cover - optional dependency
    yaml = None

def _load_spec(path: Path) -> Mapping[str, Sequence[int]]:
    """Load the YAML specification from ``path``."""

    with path.open("r", encoding="utf-8") as fh:
        text = fh.read()
    if yaml:
        data = yaml.safe_load(text)
    else:
        import re

        data = {}
        for line in text.splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            name, rest = line.split(":", 1)
            bits = [int(x) for x in re.findall(r"\d+", rest)]
            data[name.strip()] = bits
    if not isinstance(data, dict):
        raise ValueError("Specification must be a mapping")
    return data


def _emit_macros(spec: Mapping[str, Sequence[int]]) -> str:
    """Return C preprocessor definitions for ``spec``."""

    lines = []
    for name, (start, end) in spec.items():
        width = end - start + 1
        mask = ((1 << width) - 1) << start
        lines.append(f"#define {name}_SHIFT {start}")
        lines.append(f"#define {name}_MASK  0x{mask:08x}")
        lines.append("")
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    """Entry point for console use."""

    argv = argv if argv is not None else sys.argv[1:]
    if not argv or len(argv) != 1:
        print("Usage: bitfield_gen.py <spec.yml>")
        return 1
    spec_path = Path(argv[0])
    macros = _emit_macros(_load_spec(spec_path))
    print(macros)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
