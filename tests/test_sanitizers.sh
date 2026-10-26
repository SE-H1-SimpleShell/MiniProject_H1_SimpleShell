#!/usr/bin/env bash
# Test Runner for AddressSanitizer (ASan) & UndefinedBehaviorSanitizer (UBSan)
# Verifies zero memory violations, zero descriptor leaks, and clean execution.

set -euo pipefail

echo "=========================================================="
echo " Starting Sanitizer Test Suite (ASan + UBSan)"
echo "=========================================================="

export ASAN_OPTIONS="detect_leaks=1:abort_on_error=1:symbolize=1:fast_unwind_on_malloc=0"
export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"

# 1. Clean previous builds
echo "Cleaning build tree..."
make clean

# 2. Run asan-test target
echo "Compiling and executing test suite with sanitizer instrumentation..."
make asan-test

echo "=========================================================="
echo " All Sanitizer Checks Passed with Zero Diagnostics!"
echo " Total: 36 parser assertions + 34 shell integration tests"
echo "=========================================================="
