# Repository Guidelines

## Project Structure & Module Organization
- `include/`: public headers (`avl_tree.h`, `test_framework.h`) consumed by both library and CLI.
- `src/`: implementation files (`avl_tree.c` for AVL logic, `main.c` for interactive CLI).
- `tests/`: unit tests plus the custom runner (`avl_tree_tests.c`, `test_framework.c`).
- Support scripts (`run_tests.sh/.bat`, `run_cli.sh/.bat`) live at the repo root for quick local workflows.

## Build, Test, and Development Commands
- `./run_cli.sh` (or `run_cli.bat` on Windows): builds the CLI with `gcc` and launches the interactive tool.
- `./run_tests.sh` (or `run_tests.bat`): compiles the AVL library plus the in-repo test framework, then executes all tests.
- `cmake -S . -B build && cmake --build build`: generates build files for IDE use or alternative toolchains.
- `ctest --test-dir build`: runs the CTest suite after a CMake build.

## Coding Style & Naming Conventions
- C17/C11 codebase; stick to 4-space indentation, braces on the same line as control statements.
- Header guards use screaming snake case (e.g., `#ifndef AVL_TREE_H`).
- Files and symbols follow snake_case (`avl_tree_insert`, `test_framework_run_suites`).
- Keep comments concise and bilingual when user-facing; prefer English for code-level docs unless explicitly localized.

## Testing Guidelines
- Tests live in `tests/`; use the lightweight framework with `TEST_CASE`, `REQUIRE`, `REQUIRE_EQ`, and optional AAA logging macros (`TEST_ARRANGE`, etc.).
- Add new suites by extending the `suites` array in `tests/avl_tree_tests.c`.
- Always run `./run_tests.sh` (or the Windows equivalent) before submitting changes; ensure failures output colored logs for readability.

## Commit & Pull Request Guidelines
- Follow a “verb + subject” commit style (e.g., “Implement AVL structure traversal API”).
- Reference relevant scripts or files in the commit body when introducing new functionality (`run_cli.sh`, `src/main.c`).
- Pull requests should describe the motivation, summarize changes, note testing commands/results, and include screenshots of the CLI tree view when UX changes occur.

## Security & Configuration Tips
- CLI scripts assume `gcc` is available; Windows users should execute `run_cli.bat`/`run_tests.bat` in a UTF-8 (`chcp 65001`) console.
- The auto-demo mode seeds randomness at runtime; avoid committing predictable outputs or hard-coded secrets.***
