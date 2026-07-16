#!/usr/bin/env bash
# run-all.sh – build and execute all shilos C++ tests.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Test counter
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

# Define colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

log_test() {
    echo -e "${YELLOW}Testing $1...${NC}"
}

log_pass() {
    echo -e "${GREEN}✓ $1${NC}"
    ((PASS_COUNT += 1))
}

log_fail() {
    echo -e "${RED}✗ $1${NC}"
    ((FAIL_COUNT += 1))
}

run_test() {
    ((TEST_COUNT += 1))
}

# Set default COD_TEST_TOOLCHAIN if not already set
export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"

# Configure
mkdir -p "$BUILD_DIR"
cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$BUILD_DIR" "$SCRIPT_DIR"

# Build (incremental)
cmake --build "$BUILD_DIR"

echo "🏃  Running shilos test executables..."
# Execute every built executable directly under build/ (ignore directories)
for exe in "$BUILD_DIR"/*; do
  if [[ -x "$exe" && ! -d "$exe" ]]; then
    log_test "$(basename "$exe")"
    run_test
    if "$exe"; then
      EXIT_CODE=0
    else
      EXIT_CODE=$?
    fi
    if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
        log_fail "$(basename "$exe") failed with Segmentation Fault (exit code $EXIT_CODE)"
    elif [[ "$EXIT_CODE" -ne 0 ]]; then
        log_fail "$(basename "$exe") failed with exit code $EXIT_CODE"
    else
        log_pass "$(basename "$exe") passed"
    fi
  fi
done

# Run tests
echo -e "${GREEN}=== Shilos Tests ===${NC}"
for test_exec in "$BUILD_DIR"/tests/*; do
    if [[ -x "$test_exec" ]]; then
        log_test "$(basename "$test_exec")"
        run_test
        if "$test_exec"; then
          EXIT_CODE=0
        else
          EXIT_CODE=$?
        fi
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "$(basename "$test_exec") failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif [[ "$EXIT_CODE" -ne 0 ]]; then
            log_fail "$(basename "$test_exec") failed with exit code $EXIT_CODE"
        else
            log_pass "$(basename "$test_exec") passed"
        fi
    fi
done

echo
if [[ "$FAIL_COUNT" -eq 0 ]]; then
    echo -e "${GREEN}✔ All Shilos tests passed! ($PASS_COUNT/$TEST_COUNT)${NC}"
    exit 0
else
    echo -e "${RED}✗ Some Shilos tests failed. ($FAIL_COUNT/$TEST_COUNT failures)${NC}"
    exit 1
fi
