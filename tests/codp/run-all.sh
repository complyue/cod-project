#!/usr/bin/env bash
# run-all.sh – CoD Package Manager shell integration tests
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Source test utilities and setup toolchain
source "$SCRIPT_DIR/../test-utils.sh"

# Colors for output
# Define colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Test counter
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"

# Ensure yaml-cmp is built using shared utility functions
YAML_CMP_DIR="$PROJECT_ROOT/tests/yaml-cmp"
YAML_CMP_BUILD_DIR="$YAML_CMP_DIR/build"

setup_build_dir "$YAML_CMP_BUILD_DIR" "$YAML_CMP_DIR"
cmake --build "$YAML_CMP_BUILD_DIR"

# Add yaml-cmp build dir to PATH
export PATH="$YAML_CMP_BUILD_DIR:$PATH"

echo -e "${GREEN}✓ yaml-cmp built successfully${NC}"
echo

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

# Run sub-tests
echo -e "${GREEN}=== CoD Package Manager Shell Integration Tests ===${NC}"
for td in "$SCRIPT_DIR"/*/; do
  if [[ -f "$td/run.sh" ]]; then
    log_test "$(basename "$td")"
    run_test
    (cd "$td" && ./run.sh)
    EXIT_CODE=$?
    if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
        log_fail "$(basename "$td") failed with Segmentation Fault (exit code $EXIT_CODE)"
    elif [[ "$EXIT_CODE" -ne 0 ]]; then
        log_fail "$(basename "$td") failed with exit code $EXIT_CODE"
    else
        log_pass "$(basename "$td") passed"
    fi
  fi
done

echo "Total tests: $TEST_COUNT, Passed: $PASS_COUNT, Failed: $FAIL_COUNT"

if [[ "$FAIL_COUNT" -eq 0 ]]; then
    echo -e "${GREEN}✔ All CoD Package Manager tests passed! ($PASS_COUNT/$TEST_COUNT)${NC}"
    exit 0
else
    echo -e "${RED}✗ Some CoD Package Manager tests failed. ($FAIL_COUNT/$TEST_COUNT failures)${NC}"
    exit 1
fi
