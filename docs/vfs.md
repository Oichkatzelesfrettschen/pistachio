# Minimal VFS Server

The minimal Virtual File System (VFS) server included with Pistachio
maintains a table of open file descriptors and their permissions.  Each
client request uses label `0x3456` and places an opcode in `MR1`.
Currently the server supports the following operations:

| Opcode | Meaning                              |
|-------:|--------------------------------------|
| `1`    | Allocate a new descriptor. `MR2` holds the `open(2)` flags
| `2`    | Close a descriptor and drop its permissions |

The server records read and write rights based on the flags supplied to
the open call.  Subsequent operations can query the table or rely on the
POSIX library to enforce the rights locally.
