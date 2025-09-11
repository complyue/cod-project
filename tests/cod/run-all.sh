#!/usr/bin/env bash
# run-all.sh – build and execute all cod C++ tests.
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Set default COD_TEST_TOOLCHAIN if not already set
export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"

# Configure (only once)
if [[ ! -f "$BUILD_DIR/Build.ninja" && ! -f "$BUILD_DIR/Makefile" ]]; then
  mkdir -p "$BUILD_DIR"
  cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$BUILD_DIR" "$SCRIPT_DIR"
fi

# Build (incremental)
cmake --build "$BUILD_DIR"

echo "🏃  Running shell-based tests..."
# Define colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Test counter
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

log_test() {
    echo -e "${YELLOW}Testing $1...${NC}"
}

log_pass() {
    echo -e "${GREEN}✓ $1${NC}"
    ((PASS_COUNT++))
}

log_fail() {
    echo -e "${RED}✗ $1${NC}"
    ((FAIL_COUNT++))
}

run_test() {
    ((TEST_COUNT++))
}

for shell_test in "$SCRIPT_DIR"/test_*.sh; do
  if [[ -x "$shell_test" ]]; then
    log_test "$(basename "$shell_test")"
    run_test
    "$shell_test"
    EXIT_CODE=$?
    if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
        log_fail "$(basename "$shell_test") failed with Segmentation Fault (exit code $EXIT_CODE)"
    elif [[ "$EXIT_CODE" -ne 0 ]]; then
        log_fail "$(basename "$shell_test") failed with exit code $EXIT_CODE"
    else
        log_pass "$(basename "$shell_test") passed"
    fi
  fi
done

echo "🏃  Running C++ test executables..."
# Execute remaining C++ test executables (cache and workspace tests)
for exe in "$BUILD_DIR"/*; do
  if [[ -x "$exe" && ! -d "$exe" ]]; then
    exe_name=$(basename "$exe")
    log_test "$exe_name"
    run_test
    "$exe"
    EXIT_CODE=$?
    if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
        log_fail "$exe_name failed with Segmentation Fault (exit code $EXIT_CODE)"
    elif [[ "$EXIT_CODE" -ne 0 ]]; then
        log_fail "$exe_name failed with exit code $EXIT_CODE"
    else
        log_pass "$exe_name passed"
    fi
  fi
done

echo
if [[ "$FAIL_COUNT" -eq 0 ]]; then
    echo -e "${GREEN}✔ All CoD tests passed! ($PASS_COUNT/$TEST_COUNT)${NC}"
    exit 0
else
    echo -e "${RED}✗ Some CoD tests failed. ($FAIL_COUNT/$TEST_COUNT failures)${NC}"
    exit 1
fi
