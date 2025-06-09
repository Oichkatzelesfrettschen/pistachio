# Engine-specific Codex Instructions

All modifications within this directory must strictly adhere to modern C++23 paradigms and constructs.

- Every function, class, and global variable must have complete Doxygen documentation, including descriptions of purpose, parameters, return values, and any global state.
- When modifying existing code, refactor recursively as needed so the resulting implementation is fully idiomatic C++23.
- Decompose large functions into smaller units where possible and prefer constexpr, templates, and modern standard library features.
- After any code changes, run `clang-format` using the repository's `.clang-format` configuration.
- Ensure all code remains fully commented and formatted before committing.
