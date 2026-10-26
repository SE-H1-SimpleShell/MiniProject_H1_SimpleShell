# Testing Tools Used and Planned

| Tool | Purpose | Evidence state |
|---|---|---|
| GCC and Makefile warning flags | Compile C source and surface diagnostics (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -std=c11`) | Build configuration active; verified warning-free across standard and sanitizer builds |
| GNU Make | Reproducible build/test/sanitizer targets (`all`, `test`, `asan-test`, `ci`, `clean`, `info`) | Enhanced Makefile with explicit timeout, inspection, and clean targets |
| Bash test harness | End-to-end shell scenarios (34 tests including pipelines, built-ins, redirections, error recovery) | `tests/test_shell.sh` active; 34 passing scenarios with zero failures |
| Parser unit test | Parsing, quote handling, operator syntax, and memory lifecycle (36 tests) | `tests/test_parser.c` active; 36 assertions passing with zero failures |
| GitHub Actions | Automated CI pipeline triggered on PRs and commits targeting `main` with step timeouts | `.github/workflows/ci.yml` configured and active on `ubuntu-latest` (Refs Issue #8) |
| AddressSanitizer / UndefinedBehaviorSanitizer | Memory violation, leak detection, and undefined-behaviour checks via `make asan-test` | Automated in CI and local harness (`tests/test_sanitizers.sh`); zero diagnostics emitted |
| Valgrind (optional) | Secondary memory check | Target `make valgrind` configured in Makefile |

## Bug-fix & CI Validation Process
Reproduce → isolate → add regression test → patch on feature branch → run focused tests → run regression/sanitizer checks (`make asan-test`) → open PR targeting `main` → GitHub Actions CI green → review and merge → record retest evidence.
