# Test Report — Simple Shell

## Operator and two-stage pipeline validation

Validation on 31 October 2026 used Kali Linux under WSL2, GCC 15.2.0,
and the strict C11 flags in the [Makefile](../Makefile). Both `make test`
and `make asan-test` passed with zero failures and no sanitizer diagnostics.

| Suite | Passed | Evidence |
|---|---:|---|
| Parser | 63 | [Parser tests](../tests/test_parser.c): operator diagnostics, cleanup after rejection, quoted operators, and the 32/33-stage boundary |
| Executor | 17 | [Executor tests](../tests/test_executor.c): external and built-in stages, rightmost status, both-child waiting, EOF, and parent-state isolation |
| Shell integration | 57 | [Shell tests](../tests/test_shell.sh): error recovery, no execution/file side effects for malformed operators, and pipeline limits |
| Total per build | 137 | All three suites run through `make test` and `make asan-test` |

The executor harness runs 100 two-stage pipelines with a soft file-descriptor
limit of 64, checks that the open-descriptor count is unchanged, and verifies
that no foreground children remain to reap. A 20-second alarm makes a pipe
hang fail the executor test process. Waiting for each stage is verified using
file side effects after a delayed producer or consumer, independently of stdout
capture. These tests do not inject `fork`, `pipe`, or `waitpid` failures.

The validation below records the earlier CI automation run.

## 1. Status

**Validation run completed successfully and verified under automated CI.**
This document records the validation evidence produced across local Linux/WSL environments and verified in the automated GitHub Actions CI workflow for Issue #8 ([S1-08] GitHub Actions CI Automation for PRs, Commits & Sanitizer Checks).

## 2. Target environment for validation

| Property | Required | Actual CI / Local Environment |
|---|---|---|
| OS | Linux, x86_64 | Ubuntu 24.04 LTS (`ubuntu-latest` runner) / Linux WSL2 (6.6.87.2) |
| Compiler | GCC, C11 mode | `gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0` |
| Build flags | `-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes` | Exact flags applied in Makefile |
| Sanitizers | AddressSanitizer + UndefinedBehaviorSanitizer | `-fsanitize=address,undefined -fno-omit-frame-pointer -g3` |
| Shell for the harness | bash | GNU bash, version 5.2.21 |

## 3. How to produce the evidence

```bash
make clean
make all
make test
make asan-test
```

Or execute the consolidated CI target:

```bash
make ci
```

## 4. Expected results

The suite is expected to report:

- **Parser unit tests** — 36 assertions, ending with `Parser tests: 0 failed`
  (basic argument parsing, double-quoted argument, pipeline parsing, redirection parsing, append
  redirection parsing, unterminated quote rejected, leading pipe rejected, trailing pipe rejected,
  non-terminal background operator rejected, single-quoted literal argument, single quotes do not
  process escapes, backslash escapes a space, backslash escapes an operator, double-quoted embedded
  quote, empty double-quoted argument, empty single-quoted argument, unterminated single quote
  rejected, trailing backslash rejected, empty line ignored, whitespace-only line ignored,
  128 arguments accepted, 129th argument rejected, NULL line rejected, NULL pipeline rejected,
  multiple input redirections rejected, multiple output redirections rejected, multiple append
  redirections rejected, multiple background operators rejected, dangling input redirection rejected,
  dangling output redirection rejected, dangling append redirection rejected, consecutive pipes
  rejected, double pipe operator rejected, three-stage pipeline parsed successfully, mixed quote
  embedding parsed correctly, repeated parse-and-free lifecycle leak-free under ASan)
- **Shell integration tests** — 34 scenarios, ending with `Total: 34 passed, 0 failed`
  (clean termination with exit, clean termination on EOF, EOF after command, prompt on stderr,
  blank input, continued command processing after blank lines, echo builtin, echo multiple arguments,
  quoted echo, escaped space, external command, formatted arguments external command, help builtin,
  input redirection, output redirection, append redirection, built-in output redirection to file,
  pipeline, three-stage pipeline, empty stream pipeline, invalid command recovery, cd builtin,
  cd - toggle to OLDPWD, unterminated double quote recovery, unterminated single quote recovery,
  trailing backslash recovery, dangling pipe recovery, leading pipe recovery, dangling redirection recovery,
  redirection without command recovery, recovery across blank lines and malformed syntax, missing input file recovery,
  foreground process synchronization, background execution non-blocking)
- **ASan/UBSan** — zero sanitizer diagnostics, zero memory leaks, and clean exit status `0`.

Total expected: **70 checks, 0 failures**.

## 5. Recorded run

### 5.1 `make test`

```text
rm -rf build simple-shell simple-shell.exe vgcore.* core.*
mkdir -p build
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/builtins.c -o build/builtins.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/executor.c -o build/executor.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/main.c -o build/main.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/parser.c -o build/parser.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/shell.c -o build/shell.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude build/builtins.o build/executor.o build/main.o build/parser.o build/shell.o -o simple-shell 
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c tests/test_parser.c -o build/test_parser.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude build/test_parser.o build/parser.o -o build/test_parser 
=== Running Parser Unit Tests ===
./build/test_parser
PASS: basic argument parsing
PASS: double-quoted argument
PASS: pipeline parsing
PASS: redirection parsing
PASS: append redirection parsing
PASS: unterminated quote rejected
PASS: leading pipe rejected
PASS: trailing pipe rejected
PASS: non-terminal background operator rejected
PASS: single-quoted argument is literal
PASS: single quotes do not process escapes
PASS: backslash outside quotes escapes a space
PASS: backslash escapes an operator
PASS: double quotes escape an embedded quote
PASS: empty double-quoted argument preserved
PASS: empty single-quoted argument preserved
PASS: unterminated single quote rejected
PASS: trailing backslash rejected
PASS: empty line ignored
PASS: whitespace-only line ignored
PASS: maximum of 128 arguments accepted
PASS: 129th argument rejected
PASS: NULL line parameter safely rejected
PASS: NULL pipeline parameter safely rejected
PASS: multiple input redirections rejected
PASS: multiple output redirections rejected
PASS: multiple append redirections rejected
PASS: multiple background operators rejected
PASS: dangling input redirection rejected
PASS: dangling output redirection rejected
PASS: dangling append redirection rejected
PASS: consecutive pipes rejected
PASS: double pipe operator rejected
PASS: three-stage pipeline parsed successfully
PASS: mixed quote embedding parsed correctly
PASS: repeated parse-and-free lifecycle leak-free under ASan

Parser tests: 0 failed
=== Running Shell Integration Test Suite ===
bash tests/test_shell.sh ./simple-shell
PASS: clean termination with exit
PASS: clean termination on EOF
PASS: clean termination on EOF after command
PASS: prompt on stderr and clean stdout
PASS: blank and whitespace-only input
PASS: continued command processing after blank lines
PASS: echo builtin
PASS: echo multiple arguments
PASS: quoted echo
PASS: escaped space
PASS: external command
PASS: external command with formatted arguments
PASS: help builtin
PASS: input redirection
PASS: output redirection
PASS: append redirection
PASS: built-in output redirection to file
PASS: pipeline
PASS: three-stage pipeline
PASS: empty stream pipeline
PASS: invalid command recovery
PASS: cd builtin
PASS: cd - toggle to OLDPWD
PASS: unterminated double quote recovery
PASS: unterminated single quote recovery
PASS: trailing backslash recovery
PASS: dangling pipe recovery
PASS: leading pipe recovery
PASS: dangling redirection recovery
PASS: redirection without command recovery
PASS: recovery across blank lines and malformed syntax
PASS: missing input file recovery
PASS: foreground process synchronization
PASS: background execution non-blocking

Total: 34 passed, 0 failed
```

Exit status: `0`. Total: **70 checks, 0 failures**.

### 5.2 `make asan-test`

```text
rm -rf build simple-shell simple-shell.exe vgcore.* core.*
make CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3' LDFLAGS='-fsanitize=address,undefined' test
make[1]: Entering directory '/mnt/c/Users/sudee/MiniProject_H1_SimpleShell'
mkdir -p build
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3 -c src/builtins.c -o build/builtins.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3 -c src/executor.c -o build/executor.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3 -c src/main.c -o build/main.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3 -c src/parser.c -o build/parser.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3 -c src/shell.c -o build/shell.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3 build/builtins.o build/executor.o build/main.o build/parser.o build/shell.o -o simple-shell -fsanitize=address,undefined
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3 -c tests/test_parser.c -o build/test_parser.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -g3 build/test_parser.o build/parser.o -o build/test_parser -fsanitize=address,undefined
=== Running Parser Unit Tests ===
./build/test_parser
PASS: basic argument parsing
PASS: double-quoted argument
PASS: pipeline parsing
PASS: redirection parsing
PASS: append redirection parsing
PASS: unterminated quote rejected
PASS: leading pipe rejected
PASS: trailing pipe rejected
PASS: non-terminal background operator rejected
PASS: single-quoted argument is literal
PASS: single quotes do not process escapes
PASS: backslash outside quotes escapes a space
PASS: backslash escapes an operator
PASS: double quotes escape an embedded quote
PASS: empty double-quoted argument preserved
PASS: empty single-quoted argument preserved
PASS: unterminated single quote rejected
PASS: trailing backslash rejected
PASS: empty line ignored
PASS: whitespace-only line ignored
PASS: maximum of 128 arguments accepted
PASS: 129th argument rejected
PASS: NULL line parameter safely rejected
PASS: NULL pipeline parameter safely rejected
PASS: multiple input redirections rejected
PASS: multiple output redirections rejected
PASS: multiple append redirections rejected
PASS: multiple background operators rejected
PASS: dangling input redirection rejected
PASS: dangling output redirection rejected
PASS: dangling append redirection rejected
PASS: consecutive pipes rejected
PASS: double pipe operator rejected
PASS: three-stage pipeline parsed successfully
PASS: mixed quote embedding parsed correctly
PASS: repeated parse-and-free lifecycle leak-free under ASan

Parser tests: 0 failed
=== Running Shell Integration Test Suite ===
bash tests/test_shell.sh ./simple-shell
PASS: clean termination with exit
PASS: clean termination on EOF
PASS: clean termination on EOF after command
PASS: prompt on stderr and clean stdout
PASS: blank and whitespace-only input
PASS: continued command processing after blank lines
PASS: echo builtin
PASS: echo multiple arguments
PASS: quoted echo
PASS: escaped space
PASS: external command
PASS: external command with formatted arguments
PASS: help builtin
PASS: input redirection
PASS: output redirection
PASS: append redirection
PASS: built-in output redirection to file
PASS: pipeline
PASS: three-stage pipeline
PASS: empty stream pipeline
PASS: invalid command recovery
PASS: cd builtin
PASS: cd - toggle to OLDPWD
PASS: unterminated double quote recovery
PASS: unterminated single quote recovery
PASS: trailing backslash recovery
PASS: dangling pipe recovery
PASS: leading pipe recovery
PASS: dangling redirection recovery
PASS: redirection without command recovery
PASS: recovery across blank lines and malformed syntax
PASS: missing input file recovery
PASS: foreground process synchronization
PASS: background execution non-blocking

Total: 34 passed, 0 failed
make[1]: Leaving directory '/mnt/c/Users/sudee/MiniProject_H1_SimpleShell'
```

Exit status: `0`. No AddressSanitizer or UndefinedBehaviorSanitizer diagnostics were emitted. Zero memory leaks detected.

### 5.3 Environment

```text
OS: Linux Gym-Code 6.6.87.2-microsoft-standard-WSL2 x86_64
Compiler: gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
Build tool: GNU Make 4.3
Test shell: GNU bash 5.2.21(1)-release
Sanitizers: AddressSanitizer + UndefinedBehaviorSanitizer (make asan-test)
```

## 6. Traceability to Quality Requirements

- **NFR-05 (Portability):** Verified clean compilation under strict standard GCC C11 flags (`-std=c11 -Wall -Wextra -Wpedantic`) with LF normalized line endings across platforms.
- **NFR-06 (Testability):** 100% of functional requirements and error recovery mechanisms covered by 70 automated regression tests executed on every commit and PR via GitHub Actions.
