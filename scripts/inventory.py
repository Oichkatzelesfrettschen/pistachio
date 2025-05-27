#!/usr/bin/env python3
import os


def list_tree(base="."):
    for root, dirs, files in os.walk(base):
        level = root.replace(base, "").count(os.sep)
        indent = " " * 2 * level
        print(f"{indent}{os.path.basename(root)}/")
        subindent = " " * 2 * (level + 1)
        for d in sorted(dirs):
            print(f"{subindent}{d}/")
        for f in sorted(files):
            print(f"{subindent}{f}")
        if level >= 1:
            dirs[:] = []  # do not recurse deep


if __name__ == "__main__":
    list_tree()
