#!/usr/bin/env python3
"""Scan Eigen headers and build mapping of template instantiations.

This script walks the Eigen/ directory, parses each header with clang's
Python bindings, dumps the AST to porter/asts/<file>.yaml, and records
any ClassTemplateDecl or FunctionTemplateDecl names.

It outputs a YAML mapping file mapping template names to placeholder C
families which can be filled in later.
"""

import os
import sys
import re
import warnings

try:
    import yaml

    HAVE_YAML = True
except Exception:  # pragma: no cover - optional dependency
    import json

    HAVE_YAML = False

try:
    from clang.cindex import Index, Config, CursorKind
    HAVE_CLANG = True
except Exception:  # pragma: no cover - optional fallback
    Index = None
    Config = None
    CursorKind = None
    HAVE_CLANG = False

# Allow overriding libclang path via env var
LIBCLANG_PATH = os.environ.get("LIBCLANG_PATH")
if HAVE_CLANG and LIBCLANG_PATH:
    Config.set_library_file(LIBCLANG_PATH)

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EIGEN_DIR = os.path.join(REPO_ROOT, "Eigen")
AST_DIR = os.path.join(REPO_ROOT, "porter", "asts")
MAPPING_PATH = os.path.join(REPO_ROOT, "porter", "mapping.yaml")

os.makedirs(AST_DIR, exist_ok=True)

if HAVE_CLANG:
    index = Index.create()
else:
    index = None


MATRIX_RE = re.compile(r"\bMatrix<[^>]+>")


def _collect_matrix_instantiations(cursor, out):
    """Recursively collect `Eigen::Matrix<...>` specializations."""
    try:
        tspell = cursor.type.spelling
    except Exception:
        tspell = ""
    if tspell:
        m = MATRIX_RE.search(tspell)
        if m:
            out.add(m.group(0).replace("Eigen::", ""))
    for c in cursor.get_children():
        _collect_matrix_instantiations(c, out)


def _placeholder_name(spec: str) -> str:
    """Convert a Matrix<...> instantiation to a placeholder C family name."""
    m = re.match(r"Matrix<\s*([^,>]+)\s*,\s*([^,>]+)\s*,\s*([^,>]+)", spec)
    if not m:
        return "EC_Matrix"
    dtype, rows, _cols = m.groups()
    base = {
        "float": "f",
        "double": "d",
        "int": "i",
    }.get(dtype.split("::")[-1], "t")
    return f"EC_Matrix{rows}{base}"


_USED_NAMES = set()


def _sanitize_identifier(name: str) -> str:
    """Return a unique C identifier for ``name`` prefixed with ``EC_``."""
    base = re.sub(r"[^0-9a-zA-Z_]", "_", name)
    base = re.sub(r"_+", "_", base).strip("_") or "id"
    if base[0].isdigit():
        base = "_" + base
    prefix = f"EC_{base}"
    candidate = prefix
    idx = 1
    while candidate in _USED_NAMES:
        candidate = f"{prefix}_{idx}"
        idx += 1
    _USED_NAMES.add(candidate)
    return candidate


def cursor_to_dict(cursor):
    """Recursively convert a clang cursor to a serialisable dictionary."""
    return {
        "kind": str(cursor.kind),
        "spelling": cursor.spelling,
        "children": [cursor_to_dict(c) for c in cursor.get_children()],
    }


def process_header(path: str, tu=None, existing=None):
    """Extract template info from a header."""
    mapping = {}
    instantiations = set()
    if HAVE_CLANG and tu is not None:
        for cursor in tu.cursor.get_children():
            if cursor.kind in (
                CursorKind.CLASS_TEMPLATE,
                CursorKind.FUNCTION_TEMPLATE,
            ):
                if existing is None or cursor.spelling not in existing:
                    mapping[cursor.spelling] = _sanitize_identifier(cursor.spelling)
            _collect_matrix_instantiations(cursor, instantiations)
    else:  # text based fallback
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            text = f.read()
        for m in MATRIX_RE.finditer(text):
            instantiations.add(m.group(0).replace("Eigen::", ""))
    for spec in sorted(instantiations):
        mapping.setdefault(spec, _placeholder_name(spec))
    return mapping


def main():
    compile_args = ["-std=c++23", f"-I{EIGEN_DIR}"]
    if os.path.exists(MAPPING_PATH):
        with open(MAPPING_PATH, "r", encoding="utf-8") as f:
            if HAVE_YAML:
                mapping = yaml.safe_load(f) or {}
                mapping = mapping.get("mappings", mapping)
            else:
                mapping = {}
                for line in f:
                    line = line.strip()
                    if not line or line.startswith("#") or line == "mappings:":
                        continue
                    if ":" in line:
                        k, v = line.split(":", 1)
                        mapping[k.strip()] = v.strip()
    else:
        mapping = {}
    _USED_NAMES.update(mapping.values())
    for root, _, files in os.walk(EIGEN_DIR):
        for name in files:
            if not name.endswith((".h", ".hpp")):
                continue
            path = os.path.join(root, name)
            rel_path = os.path.relpath(path, EIGEN_DIR)
            print(f"Parsing {rel_path}...", file=sys.stderr)
            if HAVE_CLANG:
                tu = None
                try:
                    tu = index.parse(path, args=compile_args)
                except Exception as exc:
                    warnings.warn(
                        f"Failed to parse {rel_path} with clang: {exc}. "
                        "Set LIBCLANG_PATH to the directory containing libclang "
                        "or install the Clang package to enable AST generation."
                    )
                if tu is None or getattr(tu, "cursor", None) is None:
                    warnings.warn(
                        f"libclang produced no translation unit for {rel_path}. "
                        "Set LIBCLANG_PATH to the directory containing libclang "
                        "or install the Clang package to enable AST generation."
                    )
                    tu = None
            else:
                tu = None
            ext = ".yaml" if HAVE_YAML else ".json"
            ast_path = os.path.join(AST_DIR, rel_path.replace(os.sep, "_") + ext)
            with open(ast_path, "w", encoding="utf-8") as f:
                if HAVE_CLANG and tu is not None:
                    data = cursor_to_dict(tu.cursor)
                else:
                    data = {"parsed": False}
                if HAVE_YAML:
                    yaml.dump(data, f)
                else:
                    json.dump(data, f, indent=2)
            mapping.update(process_header(path, tu, mapping))
            # Persist after each file so the mapping grows incrementally
            with open(MAPPING_PATH, "w", encoding="utf-8") as mf:
                if HAVE_YAML:
                    yaml.dump({"mappings": mapping}, mf)
                else:
                    json.dump({"mappings": mapping}, mf, indent=2)

    with open(MAPPING_PATH, "w", encoding="utf-8") as f:
        if HAVE_YAML:
            yaml.dump({"mappings": mapping}, f)
        else:
            json.dump({"mappings": mapping}, f, indent=2)


if __name__ == "__main__":
    main()
