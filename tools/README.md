# Pistachio Helper Tools

This directory provides small utilities used when building or validating the
kernel sources.

## bitfield_gen.py

```
$ tools/bitfield_gen.py spec.yml > bitfields.h
```

Reads a YAML description of bitfields and prints C macros containing shift and
mask definitions.

## invocation_header_gen.py

```
$ tools/invocation_header_gen.py out.h
```

Parses `engine/api/syscall.xml` and writes a header declaring simple `static`
syscall stubs.

## syscall_header_gen.py

```
$ tools/syscall_header_gen.py out.h
```

Generates numeric syscall constants from the same XML specification.

## condition.py

```
$ tools/condition.py condition.xml
```

Converts an XML condition to a C preprocessor expression on stdout.

## changed.sh

```
$ tools/changed.sh [COMMIT]
```

Lists files changed since `COMMIT` (defaults to the previous commit).

## xmllint.sh

```
$ tools/xmllint.sh [schema.xsd] file.xml
```

Validates `file.xml` using `xmllint` if installed, otherwise falls back to a
basic Python parser.
