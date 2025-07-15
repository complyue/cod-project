#!/usr/bin/env bash
# run-all.sh – YAML pretty print debug test suite
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
BLUE='\033[0;34m'
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

# Build yaml-pretty
echo "Building yaml-pretty..."
cmake --build "$BUILD_DIR"
YAML_PRETTY_BIN="$BUILD_DIR/yaml-pretty"

if [[ ! -x "$YAML_PRETTY_BIN" ]]; then
  echo -e "${RED}✗ Failed to build yaml-pretty${NC}"
  exit 1
fi

echo -e "${GREEN}✓ yaml-pretty built successfully${NC}"
echo

# Test function for parsing and debug output
test_debug_parsing() {
  local test_name="$1"
  local yaml_file="$2"
  
  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -e "${BLUE}=== Test $TESTS_RUN: $test_name ===${NC}"
  echo "File: $yaml_file"
  
  # Always show the debug output, even if it fails
  if $YAML_PRETTY_BIN "$yaml_file" 2>&1; then
    echo -e "${GREEN}✓ $test_name completed successfully${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
  else
    local exit_code=$?
    echo -e "${RED}✗ $test_name failed with exit code $exit_code${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
  fi
  echo
}

# Test function for basic parsing (should not crash)
test_parsing_only() {
  local test_name="$1"
  local file="$2"
  
  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -n "Parse test $TESTS_RUN: $test_name... "
  
  # Try to parse the file silently
  if $YAML_PRETTY_BIN "$file" >/dev/null 2>&1; then
    echo -e "${GREEN}PASS${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
  else
    local exit_code=$?
    echo -e "${RED}FAIL${NC} (parse error)"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo "  File: $file"
    echo "  Parser failed with exit code $exit_code"
  fi
}

# Test function for comparing parsed output against expected output
test_comparison() {
  local test_name="$1"
  local yaml_file="$2"
  local expected_file="$3"
  
  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -n "Comparison test $TESTS_RUN: $test_name... "
  
  # Run comparison test
  if $YAML_PRETTY_BIN --compare "$yaml_file" "$expected_file" >/dev/null 2>&1; then
    echo -e "${GREEN}PASS${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
  else
    local exit_code=$?
    echo -e "${RED}FAIL${NC} (comparison mismatch)"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo "  YAML file: $yaml_file"
    echo "  Expected file: $expected_file"
    echo "  Comparison failed with exit code $exit_code"
    echo "  Run manually: $YAML_PRETTY_BIN --compare \"$yaml_file\" \"$expected_file\""
  fi
}

echo "=== YAML Pretty Print Debug Suite ==="
echo

# Test 1: Built-in test suite
echo "--- Built-in Test Suite ---"
echo "Running yaml-pretty --test-all..."
if $YAML_PRETTY_BIN --test-all; then
  echo -e "${GREEN}✓ Built-in tests completed${NC}"
else
  echo -e "${YELLOW}⚠ Built-in tests had issues (continuing with file tests)${NC}"
fi
echo

# Test 2: Debug parsing with detailed output
echo "--- Debug Parsing Tests (with full output) ---"

# Test our created test files
test_debug_parsing "Simple YAML structure" "$TEST_DATA_DIR/simple.yaml"
test_debug_parsing "Nested YAML structure" "$TEST_DATA_DIR/nested.yaml"
test_debug_parsing "Mixed types YAML" "$TEST_DATA_DIR/mixed.yaml"

# Test the problematic file from yaml-cmp
YAML_CMP_TEST_DIR="$SCRIPT_DIR/../yaml-cmp/test-data"
if [[ -f "$YAML_CMP_TEST_DIR/nested_yaml.yaml" ]]; then
  test_debug_parsing "Problematic nested YAML (from yaml-cmp)" "$YAML_CMP_TEST_DIR/nested_yaml.yaml"
fi

if [[ -f "$YAML_CMP_TEST_DIR/nested_json.yaml" ]]; then
  test_debug_parsing "JSON-style nested YAML (from yaml-cmp)" "$YAML_CMP_TEST_DIR/nested_json.yaml"
fi

echo "--- Comparison Tests (ugly vs expected) ---"

# Test our ugly files against expected outputs
test_comparison "Ugly simple YAML format" "$TEST_DATA_DIR/ugly_simple.yaml" "$TEST_DATA_DIR/expected_ugly_simple.txt"
test_comparison "Ugly nested YAML format" "$TEST_DATA_DIR/ugly_nested.yaml" "$TEST_DATA_DIR/expected_ugly_nested.txt"
test_comparison "Ugly mixed types YAML format" "$TEST_DATA_DIR/ugly_mixed.yaml" "$TEST_DATA_DIR/expected_ugly_mixed.txt"

echo "--- Quick Parse Tests (silent) ---"

# Quick parsing tests for any other test files we can find
for test_file in "$TEST_DATA_DIR"/*.yaml; do
  if [[ -f "$test_file" ]]; then
    basename_file=$(basename "$test_file")
    test_parsing_only "Parse check: $basename_file" "$test_file"
  fi
done

echo
echo "=== Test Results ==="
echo "Tests run: $TESTS_RUN"
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"

if [[ $TESTS_FAILED -eq 0 ]]; then
  echo -e "${GREEN}✓ All tests completed successfully!${NC}"
  echo "✔ yaml-pretty debug test suite finished"
  exit 0
else
  echo -e "${YELLOW}⚠ Some tests failed - check debug output above${NC}"
  echo "The failed tests may help identify YAML parsing issues."
  exit 1
fi 
