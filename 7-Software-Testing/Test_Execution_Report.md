# Test Report — Simple Shell

## 1. Status

**No validation run has been recorded yet.** This document specifies how to produce the evidence and
what a successful run must show. It deliberately contains no pass/fail claims until a real run is
performed and pasted below.

`docs/RTM.md` records implementation status only. Validation evidence becomes part of the submission
when Sections 3 and 4 are completed with actual output.

## 2. Target environment for validation

| Property | Required |
|---|---|
| OS | Linux, x86_64 (or the team's agreed target Linux VM) |
| Compiler | GCC, C11 mode |
| Build flags | `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes` |
| Sanitizers | AddressSanitizer + UndefinedBehaviorSanitizer |
| Shell for the harness | bash |

Record the exact `gcc --version` and `uname -a` output with the evidence.

## 3. How to produce the evidence

```bash
make clean
make
make test
make asan-test
```

Use `make asan-test`. Running `make asan && make test` does not validate under sanitizers, because
the second `make test` rebuilds without the sanitizer flags.

Paste the complete terminal output below.

## 4. Expected results

The suite is expected to report:

- **Parser unit tests** — 20 assertions, ending with `Parser tests: 0 failed`
  (basic argument parsing, double-quoted argument, pipeline parsing, redirection parsing, append
  redirection parsing, unterminated quote rejected, leading pipe rejected, trailing pipe rejected,
  non-terminal background operator rejected, single-quoted literal argument, single quotes do not
  process escapes, backslash escapes a space, backslash escapes an operator, double-quoted embedded
  quote, empty double-quoted argument, empty single-quoted argument, unterminated single quote
  rejected, trailing backslash rejected, 128 arguments accepted, 129th argument rejected)
- **Shell integration tests** — 18 scenarios, ending with `Total: 18 passed, 0 failed`
  (echo, quoted echo, escaped space, external command, help, input redirection, output redirection, append
  redirection, pipeline, invalid command recovery, `cd`, unterminated double-quote recovery,
  unterminated single-quote recovery, trailing-backslash recovery, dangling-pipe recovery,
  missing input file recovery, foreground process synchronization, background execution non-blocking)
- **ASan/UBSan** — no sanitizer diagnostics and no leaks

Total expected: **38 checks, 0 failures**.

If the real run differs from this, the difference must be explained here rather than the expectation
being edited to match.

## 5. Recorded run

Paste the evidence for each phase below.

### 5.1 `make test`

```text
mkdir -p build
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/builtins.c -o build/builtins.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/executor.c -o build/executor.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/main.c -o build/main.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/parser.c -o build/parser.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c src/shell.c -o build/shell.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude build/builtins.o build/executor.o build/main.o build/parser.o build/shell.o -o simple-shell 
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -c tests/test_parser.c -o build/test_parser.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude build/test_parser.o build/parser.o -o build/test_parser 
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
PASS: maximum of 128 arguments accepted
PASS: 129th argument rejected

Parser tests: 0 failed
bash tests/test_shell.sh ./simple-shell
PASS: echo builtin
PASS: quoted echo
PASS: escaped space
PASS: external command
PASS: help builtin
PASS: input redirection
PASS: output redirection
PASS: append redirection
PASS: pipeline
PASS: invalid command recovery
PASS: cd builtin
PASS: unterminated double quote recovery
PASS: unterminated single quote recovery
PASS: trailing backslash recovery
PASS: dangling pipe recovery
PASS: missing input file recovery
PASS: foreground process synchronization
PASS: background execution non-blocking

Total: 18 passed, 0 failed
```

Exit status: `0`. Total: **38 checks, 0 failures**.

### 5.2 `make asan-test`

```text
rm -rf build simple-shell
make CFLAGS='-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer' LDFLAGS='-fsanitize=address,undefined' test
make[1]: Entering directory '/tmp/simple-shell-run'
mkdir -p build
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -c src/builtins.c -o build/builtins.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -c src/executor.c -o build/executor.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -c src/main.c -o build/main.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -c src/parser.c -o build/parser.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -c src/shell.c -o build/shell.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer build/builtins.o build/executor.o build/main.o build/parser.o build/shell.o -o simple-shell -fsanitize=address,undefined
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer -c tests/test_parser.c -o build/test_parser.o
gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2 -Wstrict-prototypes -g -Iinclude -fsanitize=address,undefined -fno-omit-frame-pointer build/test_parser.o build/parser.o -o build/test_parser -fsanitize=address,undefined
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
PASS: maximum of 128 arguments accepted
PASS: 129th argument rejected

Parser tests: 0 failed
bash tests/test_shell.sh ./simple-shell
PASS: echo builtin
PASS: quoted echo
PASS: escaped space
PASS: external command
PASS: help builtin
PASS: input redirection
PASS: output redirection
PASS: append redirection
PASS: pipeline
PASS: invalid command recovery
PASS: cd builtin
PASS: unterminated double quote recovery
PASS: unterminated single quote recovery
PASS: trailing backslash recovery
PASS: dangling pipe recovery
PASS: missing input file recovery
PASS: foreground process synchronization
PASS: background execution non-blocking

Total: 18 passed, 0 failed
make[1]: Leaving directory '/tmp/simple-shell-run'
```

Exit status: `0`. No AddressSanitizer or UndefinedBehaviorSanitizer diagnostics were emitted.

### 5.3 Environment

```text
OS: Kali Linux under WSL 2 (Windows host)
Kernel: Linux 6.6.87.2-microsoft-standard-WSL2 x86_64
Compiler: gcc (Debian 16.2.0-1) 16.2.0
Build tool: GNU Make 4.4.1
Test shell: GNU bash 5.3.9(1)-release
Sanitizers: AddressSanitizer + UndefinedBehaviorSanitizer (make asan-test)
```

The suite was run from a temporary copy of the working tree with LF line endings, because the
Windows checkout uses CRLF. The source files were unchanged by that copy.

## 6. Manual verification still outstanding

These scenarios are documented but not automated, and must be exercised manually before acceptance.

| Test | What to confirm |
|---|---|
| TC-17 | Long pipelines (`seq 10000 \| wc -l`) and repeated runs produce no crashes |

## 7. Limitations

This report will record local development validation. Final acceptance should also be run on the
team's agreed target Linux VM and against any instructor-provided test specification.
