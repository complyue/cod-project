#!/usr/bin/env bash
# run-all.sh – YAML exact printing test suite
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
TEST_DATA_DIR="$SCRIPT_DIR/test-data"

# Set default COD_TEST_TOOLCHAIN if not already set
export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"

# Configure build directory if first run
if [[ ! -f "$BUILD_DIR/Build.ninja" && ! -f "$BUILD_DIR/Makefile" ]]; then
  mkdir -p "$BUILD_DIR"
  cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$BUILD_DIR" "$SCRIPT_DIR"
fi

# Build yaml-exact-test
echo "Building yaml-exact-test..."
cmake --build "$BUILD_DIR"
YAML_EXACT_BIN="$BUILD_DIR/yaml-exact-test"

if [[ ! -x "$YAML_EXACT_BIN" ]]; then
  echo -e "${RED}✗ Failed to build yaml-exact-test${NC}"
  exit 1
fi

echo -e "${GREEN}✓ yaml-exact-test built successfully${NC}"
echo

# Test function for exact printing
test_exact_printing() {
  local test_name="$1"
  local yaml_file="$2"
  
  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -n "Test $TESTS_RUN: $test_name... "
  
  # Run the test and capture output
  if $YAML_EXACT_BIN "$yaml_file" >/dev/null 2>&1; then
    echo -e "${GREEN}PASS${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
  else
    echo -e "${RED}FAIL${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo "  File: $yaml_file"
    echo "  Test failed with exit code $?"
  fi
}

# Test function for basic parsing (should not crash)
test_parsing() {
  local test_name="$1"
  local file="$2"
  
  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -n "Parse test $TESTS_RUN: $test_name... "
  
  # Try to parse the file (internally this will test exact formatting)
  if $YAML_EXACT_BIN "$file" >/dev/null 2>&1; then
    echo -e "${GREEN}PASS${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
  else
    echo -e "${RED}FAIL${NC} (parse error)"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo "  File: $file"
    echo "  Parser failed with exit code $?"
  fi
}

echo "=== YAML Exact Printing Test Suite ==="
echo

# Test 1: Basic functionality test
echo "--- Basic Functionality Tests ---"

# Test basic functionality with static test file
test_parsing "Simple YAML parsing" "$TEST_DATA_DIR/simple_test.yaml"

echo
echo "--- Exact Printing Tests ---"

# Test the prepared test data files
test_exact_printing "Basic YAML with comments" "$TEST_DATA_DIR/basic_with_comments.yaml"
test_exact_printing "Complex formatting" "$TEST_DATA_DIR/complex_formatting.yaml"
test_exact_printing "Whitespace edge cases" "$TEST_DATA_DIR/whitespace_edge_cases.yaml"

echo
echo "--- Individual File Parsing Tests ---"

# Test individual file parsing (should not crash)
test_parsing "Basic YAML with comments" "$TEST_DATA_DIR/basic_with_comments.yaml"
test_parsing "Complex formatting" "$TEST_DATA_DIR/complex_formatting.yaml"
test_parsing "Whitespace edge cases" "$TEST_DATA_DIR/whitespace_edge_cases.yaml"

echo
echo "--- Advanced Tests ---"

# Test with various YAML constructs using static test file
test_exact_printing "Advanced YAML constructs" "$TEST_DATA_DIR/advanced_constructs.yaml"

echo
echo "=== Test Results ==="
echo "Tests run: $TESTS_RUN"
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"

if [[ $TESTS_FAILED -eq 0 ]]; then
  echo -e "${GREEN}✓ All tests passed!${NC}"
  echo "✔ yaml-exact-test comprehensive test suite completed successfully"
  exit 0
else
  echo -e "${RED}✗ Some tests failed!${NC}"
  exit 1
fi
