#!/usr/bin/env bash
# Validate XML using xmllint if available, otherwise fall back to Python.

set -euo pipefail

schema=""
file=""
if [ "$#" -eq 2 ]; then
    schema="$1"
    file="$2"
elif [ "$#" -eq 1 ]; then
    file="$1"
else
    echo "Usage: xmllint.sh [schema.xsd] file.xml" >&2
    exit 1
fi

if command -v xmllint >/dev/null 2>&1; then
    if [ -n "$schema" ]; then
        xmllint --noout --schema "$schema" "$file"
    else
        xmllint --noout "$file"
    fi
else
    if [ -n "$schema" ]; then
        python3 - <<'EOF' "$schema" "$file"
import sys
from xml.etree import ElementTree as ET

schema = sys.argv[1] if len(sys.argv) == 3 else None
file = sys.argv[-1]
ET.parse(file)  # raises on invalid XML
if schema:
    print('Warning: schema validation skipped (xmllint not available)')
EOF
    else
        python3 - <<'EOF' "$file"
import sys
from xml.etree import ElementTree as ET

file = sys.argv[1]
ET.parse(file)
EOF
    fi
fi
