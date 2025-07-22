#!/usr/bin/env bash
# run-all.sh – YAML pretty print test suite
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
TEST_DATA_DIR="$SCRIPT_DIR/test-data"
UGLY_DIR="$TEST_DATA_DIR/ugly"
PRETTY_DIR="$TEST_DATA_DIR/pretty"

# Source test utilities and setup toolchain
source "$SCRIPT_DIR/../test-utils.sh"
setup_toolchain

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

# Diagnostic mode flag
DIAGNOSTIC_MODE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --diagnostic|-d)
      DIAGNOSTIC_MODE=true
      shift
      ;;
    --help|-h)
      cat << EOF
Usage: $0 [OPTIONS]

YAML Pretty Print Test Suite

This test suite verifies the YAML pretty printer by:
1. Converting ugly YAML files to pretty format and comparing with expected output
2. Testing idempotency by pretty-printing already pretty files

OPTIONS:
  --diagnostic, -d    Enable diagnostic output showing structural tree dumps
  --help, -h         Show this help message

EOF
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      echo "Use --help for usage information"
      exit 1
      ;;
  esac
done

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"
if [[ "$DIAGNOSTIC_MODE" == "true" ]]; then
  echo "🔍 Diagnostic mode enabled - will show structural tree dumps"
fi

# Configure build directory using utility function
setup_build_dir "$BUILD_DIR" "$SCRIPT_DIR"

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

# Test function for ugly->pretty transformation
test_ugly_to_pretty() {
  local test_name="$1"
  local ugly_file="$2"
  local pretty_file="$3"
  
  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -n "Test $TESTS_RUN: $test_name (ugly→pretty)... "
  
  # Generate pretty output from ugly file
  local temp_output=$(mktemp)
  if ! $YAML_PRETTY_BIN "$ugly_file" 2>/dev/null > "$temp_output"; then
    echo -e "${RED}FAIL${NC} (parse error)"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    rm -f "$temp_output"
    return 1
  fi
  
  # Compare with expected pretty file
  if cmp -s "$temp_output" "$pretty_file"; then
    echo -e "${GREEN}PASS${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    local result=0
  else
    echo -e "${RED}FAIL${NC} (output mismatch)"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo "  Expected: $pretty_file"
    echo "  Actual output differs"
    echo "  --- DIFF (expected vs actual) ---"
    diff -u "$pretty_file" "$temp_output" || true
    echo "  --- END DIFF ---"
    if [[ "$DIAGNOSTIC_MODE" == "true" ]]; then
      echo "  --- EXPECTED CONTENT ---"
      cat "$pretty_file"
      echo "  --- ACTUAL CONTENT ---"
      cat "$temp_output"
      echo "  --- END CONTENT ---"
    fi
    local result=1
  fi
  
  rm -f "$temp_output"
  return $result
}

# Test function for pretty->pretty idempotency
test_pretty_idempotency() {
  local test_name="$1"
  local pretty_file="$2"
  
  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -n "Test $TESTS_RUN: $test_name (pretty→pretty idempotency)... "
  
  # Generate pretty output from already pretty file
  local temp_output=$(mktemp)
  if ! $YAML_PRETTY_BIN "$pretty_file" 2>/dev/null > "$temp_output"; then
    echo -e "${RED}FAIL${NC} (parse error)"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    rm -f "$temp_output"
    return 1
  fi
  
  # Compare with original pretty file - should be identical
  if cmp -s "$temp_output" "$pretty_file"; then
    echo -e "${GREEN}PASS${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    local result=0
  else
    echo -e "${RED}FAIL${NC} (idempotency violation)"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo "  Input: $pretty_file"
    echo "  Output differs from input - not idempotent!"
    echo "  --- DIFF (original vs after pretty-print) ---"
    diff -u "$pretty_file" "$temp_output" || true
    echo "  --- END DIFF ---"
    if [[ "$DIAGNOSTIC_MODE" == "true" ]]; then
      echo "  --- ORIGINAL CONTENT ---"
      cat "$pretty_file"
      echo "  --- AFTER PRETTY-PRINT CONTENT ---"
      cat "$temp_output"
      echo "  --- END CONTENT ---"
    fi
    local result=1
  fi
  
  rm -f "$temp_output"
  return $result
}

# Show diagnostic tree dump if requested
show_diagnostic_tree() {
  local file="$1"
  local label="$2"
  
  if [[ "$DIAGNOSTIC_MODE" == "true" ]]; then
    echo -e "${BLUE}=== $label: Structural Tree Dump ===${NC}"
    $YAML_PRETTY_BIN --verbose "$file" 2>/dev/null | sed -n '/=== PARSED TREE ===/,/=== FORMAT_YAML OUTPUT ===/p' | head -n -1
    echo
  fi
}

echo "=== YAML Pretty Print Test Suite ==="
echo

# Find all test files in ugly/ directory
if [[ ! -d "$UGLY_DIR" ]]; then
  echo -e "${RED}✗ Test data directory not found: $UGLY_DIR${NC}"
  exit 1
fi

if [[ ! -d "$PRETTY_DIR" ]]; then
  echo -e "${RED}✗ Test data directory not found: $PRETTY_DIR${NC}"
  exit 1
fi

# Run tests for each file pair
echo "--- Pretty Print Transformation Tests ---"

for ugly_file in "$UGLY_DIR"/*.yaml; do
  if [[ -f "$ugly_file" ]]; then
    basename_file=$(basename "$ugly_file")
    pretty_file="$PRETTY_DIR/$basename_file"
    test_name=$(basename "$basename_file" .yaml)
    
    if [[ -f "$pretty_file" ]]; then
      # Show diagnostic output if requested
      show_diagnostic_tree "$ugly_file" "UGLY $test_name"
      show_diagnostic_tree "$pretty_file" "PRETTY $test_name"
      
      # Test ugly->pretty transformation
      test_ugly_to_pretty "$test_name" "$ugly_file" "$pretty_file"
      
      # Test pretty->pretty idempotency
      test_pretty_idempotency "$test_name" "$pretty_file"
      
    else
      echo -e "${YELLOW}⚠ Missing pretty file for $basename_file (skipping)${NC}"
    fi
  fi
done

echo
echo "=== Test Results ==="
echo "Tests run: $TESTS_RUN"
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"

if [[ $TESTS_FAILED -eq 0 ]]; then
  echo -e "${GREEN}✓ All tests passed!${NC}"
  echo "✔ YAML pretty print functionality is working correctly"
  exit 0
else
  echo -e "${RED}✗ Some tests failed${NC}"
  if [[ "$DIAGNOSTIC_MODE" != "true" ]]; then
    echo "💡 Run with --diagnostic flag to see detailed output for debugging"
  fi
  exit 1
fi 
