import os
import re
import json
from collections import defaultdict


def parse_dependencies(path):
    deps = []
    try:
        with open(path, 'r', errors='ignore') as f:
            for line in f:
                line = line.strip()
                inc_match = re.match(r'#include\s*[<\"]([^>\"]+)[>\"]', line)
                if inc_match:
                    deps.append(inc_match.group(1))
                    continue
                py_match = re.match(r'(?:from|import)\s+([\w\.]+)', line)
                if py_match:
                    deps.append(py_match.group(1))
                    continue
                sh_match = re.match(r'(?:source|\.)\s+([\w./-]+)', line)
                if sh_match:
                    deps.append(sh_match.group(1))
    except Exception:
        pass
    return deps


def build_tree(root):
    tree = defaultdict(list)
    for dirpath, dirnames, filenames in os.walk(root):
        rel = os.path.relpath(dirpath, root)
        for d in dirnames:
            tree[rel].append({"directory": os.path.join(rel, d)})
        for f in filenames:
            tree[rel].append({"file": os.path.join(rel, f)})
    return tree


def inventory(root):
    result = []
    counts = defaultdict(int)
    deps = defaultdict(list)
    for dirpath, dirnames, filenames in os.walk(root):
        for name in filenames:
            full = os.path.join(dirpath, name)
            rel = os.path.relpath(full, root)
            st = os.lstat(full)
            if os.path.islink(full):
                ftype = 'symlink'
                target = os.readlink(full)
            else:
                ftype = 'file'
                target = None
            counts[ftype] += 1
            info = {
                'path': rel,
                'size': st.st_size,
                'type': ftype,
            }
            if target:
                info['target'] = target
            if ftype == 'file':
                deps_list = parse_dependencies(full)
                if deps_list:
                    deps[rel] = deps_list
            result.append(info)
        for d in dirnames:
            counts['directory'] += 1
    tree = build_tree(root)
    return {
        'counts': counts,
        'tree': tree,
        'dependencies': deps,
        'files': result,
    }


def main():
    import argparse

    parser = argparse.ArgumentParser(description='Inventory repository')
    parser.add_argument('root', nargs='?', default='.', help='root directory')
    parser.add_argument('-o', '--output', help='output file (json)')
    args = parser.parse_args()

    data = inventory(os.path.abspath(args.root))
    if args.output:
        with open(args.output, 'w') as f:
            json.dump(data, f, indent=2)
    else:
        print(json.dumps(data, indent=2))


if __name__ == '__main__':
    main()
