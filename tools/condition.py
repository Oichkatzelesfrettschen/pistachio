#!/usr/bin/env python3
"""Utility functions to translate XML build conditions to C expressions."""

from __future__ import annotations

import sys
from xml.etree import ElementTree as ET


def condition_to_cpp(elem: ET.Element) -> str:
    """Convert an XML condition element to a C preprocessor expression."""

    tag = elem.tag
    if tag == "config":
        var = elem.attrib["var"]
        return f"defined({var})"
    if tag == "and":
        parts = [condition_to_cpp(child) for child in elem]
        return "(" + " && ".join(parts) + ")"
    if tag == "or":
        parts = [condition_to_cpp(child) for child in elem]
        return "(" + " || ".join(parts) + ")"
    if tag == "not":
        child, = list(elem)
        return "!" + condition_to_cpp(child)
    raise ValueError(f"Unsupported condition tag: {tag}")

def main(argv: list[str] | None = None) -> int:
    """CLI for converting a single condition element."""

    argv = argv if argv is not None else sys.argv[1:]
    if len(argv) != 1:
        print("Usage: condition.py <condition.xml>")
        return 1
    tree = ET.parse(argv[0])
    root = tree.getroot()
    print(condition_to_cpp(root))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
