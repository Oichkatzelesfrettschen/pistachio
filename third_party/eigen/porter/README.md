# Porter Tools

The `porter/` directory contains helper scripts for generating the experimental
C23 and Go frontends from Eigen's modernised C++23 headers.

* `scan_templates.py` – walk the `Eigen/` headers and collect template
  instantiations. ASTs are written to `porter/asts/` and the discovered
  mappings are stored in `porter/mapping.yaml`.
* `gen_headers.py` – using `mapping.yaml`, generate the C23 header
  `eigenc/include/ec_generated.h` which defines specialised matrix
  structures and operations.
* `build_all.sh` – convenience driver that regenerates headers and runs the
  test suite for both C and Go.

Run the following from the repository root to refresh generated files and
validate everything:

```bash
bash porter/build_all.sh
```

## Troubleshooting AST Generation

`scan_templates.py` uses clang's Python bindings to generate AST dumps. If you
see warnings about failed parsing or missing translation units, ensure that
libclang is installed and visible. On Debian-based systems install the
`libclang-dev` package. If `libclang.so` lives in a non-standard location,
export the environment variable `LIBCLANG_PATH` pointing to the shared library
before running the script.

If the script reports that `index.parse` returned `None` or that libclang
produced no translation unit, verify that `LIBCLANG_PATH` is set correctly or
that the Clang development libraries are installed. Re-run `scan_templates.py`
after adjusting the environment.
