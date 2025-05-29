#!/usr/bin/env python3
"""Generate a minimal C header declaring syscall stubs.

This script parses ``engine/api/syscall.xml`` and emits a header with
``static inline`` prototypes for each syscall.  It is a very small subset
of the original tooling used by the kernel but suffices for unit tests and
examples.
"""

from __future__ import annotations

import sys
from pathlib import Path
from xml.etree import ElementTree as ET

sys.path.append(str(Path(__file__).resolve().parent))
from condition import condition_to_cpp

API_XML = Path("engine/api/syscall.xml")


def _iter_syscalls(xml: Path) -> list[tuple[str, str | None]]:
    """Return ``[(name, condition)]`` pairs from *xml*."""

    tree = ET.parse(xml)
    root = tree.getroot()
    pairs: list[tuple[str, str | None]] = []
    for config in root.findall(".//config"):
        cond_elem = config.find("condition")
        condition = None
        if cond_elem is not None and len(cond_elem):
            condition = condition_to_cpp(cond_elem[0])
        for sc in config.findall("syscall"):
            name = sc.attrib["name"]
            pairs.append((name, condition))
    return pairs


def _generate_header(syscalls: list[tuple[str, str | None]]) -> str:
    lines = [
        "/* Auto-generated syscall prototypes */",
        "#pragma once",
        "",
    ]
    for name, cond in syscalls:
        if cond:
            lines.append(f"#if {cond}")
        lines.append(f"static inline int seL4_{name}(void);")
        if cond:
            lines.append("#endif")
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    argv = argv if argv is not None else sys.argv[1:]
    out = Path(argv[0]) if argv else None
    syscalls = _iter_syscalls(API_XML)
    header = _generate_header(syscalls)
    if out:
        out.write_text(header, encoding="utf-8")
    else:
        print(header)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
