# Repository Codex Instructions

This repository uses Codex CLI to maintain the development environment. When the agent opens the repository it should:

1. Ensure `.codex/setup.sh` is executable and matches the latest template hosted by the project. If differences exist, patch the file in place and commit with message `auto-heal setup.sh`.
2. Run `shellcheck` on `.codex/setup.sh` and `setup.sh` and fix any reported issues.
3. Execute `./setup.sh` with available network access so dependencies are fetched before the environment goes offline.
4. When modifying any file, run `pre-commit` before committing.
5. Tests live under `tests/`; run `pytest -q` and `ctest --output-on-failure` after changes if applicable.
