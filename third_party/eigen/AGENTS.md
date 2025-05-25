# Eigen Modernisation Plan

This repository is being refactored to **C++23** while simultaneously providing
experimental C23 and Go frontends. The notes below outline how Codex can assist
with generating the C23 interface ("EigenC") and the Go bindings. All build
scripts and makefiles are gradually updated to support these variants.

Codex is brilliant at static reasoning over the tree of files it can see. Install every external tool it might invoke during the setup window; afterwards it is hermetically sealed. Eigen itself is already header-only, but its C++ machinery (expression templates, meta-programming) must be reified into C23's `_Generic`, `static inline`, and macro metaprogramming.

---

## 0  Repository Layout & Commit Checklist
```
eigen-c23-port/
├── eigen/                 # your fork   (C++)
├── eigenc/                # output goes here (C23)
├── tests/                 # re-used Eigen unit tests, plus C bridge
├── AGENTS.md              # high-level workplan (see \u00a71)
├── setup.sh               # network-phase installer  (see \u00a72)
└── .codex.yaml            # optional overrides (parallel jobs, etc.)
```
Make sure `setup.sh` is executable (`chmod +x`). Commit nothing that you expect Codex to fetch during task-phase\u2014vendor wheels, tarballs, etc., ahead of time if needed.

---

## 1  AGENTS.md \u2014 tell Codex what to do

### Goal
Modernise Eigen to C++23 and provide experimental `eigenc/` and `eigengo/`
frontends:
* Keep the C++ API compatible while adopting modern language features.
* Generate a header-only C23 layer mirroring Eigen's API where C allows.
* Offer Go bindings that call into the generated C layer.

### Road-map
1. Build a **template-to-C23 translation pipeline** using Clang libTooling
2. Stage-wise port:
   a. Core matrix / vector types  
   b. Basic ops (+, \u2212, *, transpose, block)  
   c. Decompositions (LU, QR, SVD)
3. Emit shim headers so C++ code can `#include <eigenc/core.h>` under `extern "C"`
4. Generate differential tests: compare `eigen::MatrixXd` vs `ec_Matrix64`
5. Optimise with `static inline`, `_Generic`, and `restrict`.

### Deliverables
* `eigenc/include/\u2026` headers
* `tools/porter/` \u2013 the Clang-AST\u2013driven translator
* GitHub CI workflow that builds both C++ and C versions and runs tests

---

## 2  setup.sh \u2014 give Codex its toolbox

```bash
#!/usr/bin/env bash
set -euo pipefail

apt-get update -qq
# \u2460 Build & parse tooling
apt-get install -y clang-17 clang-tools-17 libclang-17-dev \
                   llvm-17-dev make cmake ninja-build \
                   python3 python3-pip
pip3 install --no-cache-dir clang==17.* pycparser numpy pytest

# \u2461 Optional: BLAS for performance tests (offline deb already in repo)
dpkg -i ./vendor/libopenblas*.deb || true

exit 0
```
Everything above downloads while the network is open, then lives on the disk layer visible in task-phase.

---

## 3  Prompt Codex in the Run field
Codex understands multi-step instructions.  Paste something like:

```
Phase 1.  Scan the eigen/ tree and emit a table (porter/mapping.yaml) that maps every Eigen template instantiation to a concrete C type or function family.  Use libclang to dump full ASTs.
```

When it finishes, refine:

```
Phase 2.  Using mapping.yaml, generate eigenc/include/ec_core.h that defines ec_Matrixf32, ec_Matrixf64, etc., plus _Generic overloads for the arithmetic operators.  Provide unit tests in tests/core/ that compile as C23.
```

Iterate with further phases to implement GEMM, decompositions, CI workflow, and so on.

---

## 4  Key translation tactics you can explicitly demand

| Eigen idiom | Ask Codex to\u2026 | C23 construct |
|-------------|----------------|---------------|
| Expression templates (MatrixBase, Derived) | Freeze at compile-time into structs with function-pointer vtables when size is dynamic, else `_Static_assert` on dimensions | `struct ec_expr { ... }`, `_Generic` |
| `template<int Rows,int Cols, \u2026>` | Generate a family `ec_M##R##x##C##_f32` when constexpr dims \u2264 4; fall back to dynamic case above | type-id macros |
| Operator overloading | Emit helper macros `EC_ADD(a,b)` so that user code can be mechanically rewritten, or gate on `_Generic` to pick the right inline | `_Generic` |
| SIMD packets | Translate Eigen's `packet_traits` to compiler intrinsics guarded by `#ifdef __AVX2__`, etc. | `#pragma clang attribute` or builtin |
| constexpr maths | Convert to static inline + `enum { ... }` constants | compile-time evaluable |

---

## 5  Automated self-diagnosis inside the same task
Because internet is off, your test harness must be self-contained:

```bash
# tests/run_all.sh
#!/usr/bin/env bash
set -euo pipefail
gcc -std=c23 -I../eigenc/include tests/core/test_core.c -o /tmp/test_core
/tmp/test_core
python3 -m pytest tests/compare_eigen_cpp_vs_c.py   # uses numpy only
```

Tell Codex to add a final CI step that runs `tests/run_all.sh` and returns non-zero on any mismatch; that way the Codex job will mark failure and show logs you can inspect.

---

## 6  Iterating & refining
Use chat-style feedback: "test_core fails on the transpose case \u2013 show diff and patch." Codex will open the file, edit, and commit the fix inside the container.

If translation logic itself becomes hairy, ask for helper tools, e.g.:

```
Write tools/porter/visit_templates.cpp \u2013 a clang-tool that visits every TemplateDecl and prints location + full instantiation path; emit JSON so Python can drive code-gen.
```

Codex can compile and run that helper inside the task container because Clang dev libs were installed in setup.sh.

---

## 7  Final polish & export
Once tests pass:
1. Ask Codex: "Generate README.md describing EigenC, usage examples, and build badges."
2. Add semantic-version tag (v0.1.0) by writing CHANGELOG.md.
3. For release artefacts, have Codex tar `eigenc/` and output the path:
   "Create /workspace/dist/eigenc-0.1.0.tar.gz containing eigenc/include."
   Download from the Codex UI when the run completes.

---

### References
* Codex network-lifecycle documentation
* Community posts clarifying "everything goes in setup.sh"
* Eigen's header-only architecture and getting-started guide
* StackOverflow insights on Eigen internals (expression templates, data layout)
