#!/usr/bin/env bash
set -euo pipefail

SHELL_BIN=${1:-./simple-shell}
PASS=0
FAIL=0
TMP_DIR=$(mktemp -d /tmp/simple-shell-test-XXXXXX)
trap 'rm -rf "$TMP_DIR"' EXIT

run_test() {
    local name=$1
    local input=$2
    local expected=$3
    local actual
    actual=$(printf '%s' "$input" | "$SHELL_BIN" 2>/dev/null)
    if [[ "$actual" == "$expected" ]]; then
        printf 'PASS: %s\n' "$name"
        PASS=$((PASS + 1))
    else
        printf 'FAIL: %s\n' "$name"
        printf '  expected: %q\n' "$expected"
        printf '  actual:   %q\n' "$actual"
        FAIL=$((FAIL + 1))
    fi
}

# Verify that a bad command line produces a diagnostic on stderr and that the
# shell survives it and keeps executing following commands.
run_recovery_test() {
    local name=$1
    local input=$2
    local expected_stdout=$3
    local expected_stderr=$4
    local actual_stdout actual_stderr
    actual_stdout=$(printf '%s' "$input" | "$SHELL_BIN" 2>"$TMP_DIR/stderr.txt")
    actual_stderr=$(cat "$TMP_DIR/stderr.txt")
    if [[ "$actual_stdout" == "$expected_stdout" && "$actual_stderr" == *"$expected_stderr"* ]]; then
        printf 'PASS: %s\n' "$name"
        PASS=$((PASS + 1))
    else
        printf 'FAIL: %s\n' "$name"
        printf '  expected stdout: %q\n' "$expected_stdout"
        printf '  actual stdout:   %q\n' "$actual_stdout"
        printf '  expected stderr to contain: %q\n' "$expected_stderr"
        printf '  actual stderr:   %q\n' "$actual_stderr"
        FAIL=$((FAIL + 1))
    fi
}

# --- Functional tests ---

# TC-03: Built-in echo
run_test "echo builtin" $'echo hello\nexit\n' "hello"

# TC-05: Quoted argument
run_test "quoted echo" $'echo "hello world"\nexit\n' "hello world"

# TC-06: Escaped character (previously manual)
run_test "escaped space" $'echo hello\\ world\nexit\n' "hello world"

# TC-02: External command execution
run_test "external command" $'printf hi\nexit\n' "hi"

# TC-18: Built-in help
run_test "help builtin" $'help\nexit\n' $'Simple Shell - supported commands:\n  cd [DIR]       Change current directory\n  pwd            Print current directory\n  echo [TEXT...] Print text\n  help           Show this help message\n  exit           Exit the shell\n\nExternal commands, pipes (|), redirection (<, >, >>),\nand background execution (&) are supported.'

# TC-07: Input redirection
printf 'redirection-input\n' > "$TMP_DIR/in.txt"
input=$'cat < '"$TMP_DIR"$'/in.txt\nexit\n'
run_test "input redirection" "$input" "redirection-input"

# TC-08: Output overwrite
input=$'echo output > '"$TMP_DIR"$'/out.txt\ncat '"$TMP_DIR"$'/out.txt\nexit\n'
run_test "output redirection" "$input" "output"

# TC-09: Output append
input=$'echo second >> '"$TMP_DIR"$'/out.txt\ncat '"$TMP_DIR"$'/out.txt\nexit\n'
run_test "append redirection" "$input" $'output\nsecond'

# TC-12: Pipeline execution
run_test "pipeline" $'printf "a\\nb\\nc\\n" | wc -l\nexit\n' "3"

# TC-14: Invalid command handling (diagnostic + prompt recovery)
run_recovery_test "invalid command recovery" \
    $'definitely-not-a-command\necho alive\nexit\n' "alive" "definitely-not-a-command:"

# TC-04: Directory change (cd & pwd)
actual=$(printf 'cd /tmp\npwd\nexit\n' | "$SHELL_BIN" 2>/dev/null)
if [[ "$actual" == "/tmp" ]]; then
    printf 'PASS: cd builtin\n'; PASS=$((PASS + 1))
else
    printf 'FAIL: cd builtin (actual=%q)\n' "$actual"; FAIL=$((FAIL + 1))
fi

# TC-11: Syntax / malformed input error recovery
run_recovery_test "unterminated double quote recovery" \
    $'echo "unterminated\necho alive\nexit\n' "alive" "shell: syntax error:"
run_recovery_test "unterminated single quote recovery" \
    $'echo \'unterminated\necho alive\nexit\n' "alive" "shell: syntax error:"
run_recovery_test "trailing backslash recovery" \
    $'echo trailing\\\necho alive\nexit\n' "alive" "shell: syntax error:"
run_recovery_test "dangling pipe recovery" \
    $'echo hi |\necho alive\nexit\n' "alive" "shell: syntax error:"

# TC-15: Inaccessible / missing input file recovery (previously manual)
actual=$(printf 'cat < '"$TMP_DIR"'/nonexistent_missing.txt\necho alive\nexit\n' | "$SHELL_BIN" 2>/dev/null)
if [[ "$actual" == "alive" ]]; then
    printf 'PASS: missing input file recovery\n'; PASS=$((PASS + 1))
else
    printf 'FAIL: missing input file recovery (actual=%q)\n' "$actual"; FAIL=$((FAIL + 1))
fi

# TC-16: Foreground process synchronization (previously manual)
start_fg=$(date +%s)
printf 'sleep 1\nexit\n' | "$SHELL_BIN" >/dev/null 2>&1
dur_fg=$(( $(date +%s) - start_fg ))
if [[ $dur_fg -ge 1 ]]; then
    printf 'PASS: foreground process synchronization\n'; PASS=$((PASS + 1))
else
    printf 'FAIL: foreground process synchronization (waited %ds, expected >= 1s)\n' "$dur_fg"; FAIL=$((FAIL + 1))
fi

# TC-13: Background non-blocking execution (previously manual)
start_bg=$(date +%s)
printf 'sleep 2 &\nexit\n' | "$SHELL_BIN" >/dev/null 2>&1
dur_bg=$(( $(date +%s) - start_bg ))
if [[ $dur_bg -lt 2 ]]; then
    printf 'PASS: background execution non-blocking\n'; PASS=$((PASS + 1))
else
    printf 'FAIL: background execution non-blocking (waited %ds, expected < 2s)\n' "$dur_bg"; FAIL=$((FAIL + 1))
fi

printf '\nTotal: %d passed, %d failed\n' "$PASS" "$FAIL"
[[ $FAIL -eq 0 ]]
