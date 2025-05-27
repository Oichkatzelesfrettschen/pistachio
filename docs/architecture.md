# Repository Inventory

This repository includes a helper script under `scripts/inventory.py`.
The script traverses the tree, records file metadata, collects basic
source dependencies, and emits a JSON summary.

## Usage

```bash
python3 scripts/inventory.py -o repo_inventory.json
```

The output JSON contains:

- `counts` – totals of files, directories and symlinks.
- `tree` – a simple directory hierarchy listing.
- `dependencies` – detected edges from each source file.
- `files` – per-file records including size and symlink targets.

You can inspect the report directly or process it with other tools.

