#!/usr/bin/env bash
# run-all.sh – comprehensive YAML parser test suite
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
TEST_DATA_DIR="$SCRIPT_DIR/test-data"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Configure build directory if first run
if [[ ! -f "$BUILD_DIR/Build.ninja" && ! -f "$BUILD_DIR/Makefile" ]]; then
  mkdir -p "$BUILD_DIR"
  cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$BUILD_DIR" "$SCRIPT_DIR"
fi

# Build yaml-cmp
echo "Building yaml-cmp..."
cmake --build "$BUILD_DIR"
YAMLCMP_BIN="$BUILD_DIR/yaml-cmp"

if [[ ! -x "$YAMLCMP_BIN" ]]; then
  echo -e "${RED}✗ Failed to build yaml-cmp${NC}"
  exit 1
fi

echo -e "${GREEN}✓ yaml-cmp built successfully${NC}"
echo

# Test function
run_test() {
  local test_name="$1"
  local file1="$2"
  local file2="$3"
  local should_pass="$4"  # "pass" or "fail"
  local extra_args="${5:-}"

  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -n "Test $TESTS_RUN: $test_name... "
  
  if $YAMLCMP_BIN $extra_args "$file1" "$file2" >/dev/null 2>&1; then
    if [[ "$should_pass" == "pass" ]]; then
      echo -e "${GREEN}PASS${NC}"
      TESTS_PASSED=$((TESTS_PASSED + 1))
    else
      echo -e "${RED}FAIL${NC} (expected failure but passed)"
      TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
  else
    if [[ "$should_pass" == "fail" ]]; then
      echo -e "${GREEN}PASS${NC} (expected failure)"
      TESTS_PASSED=$((TESTS_PASSED + 1))
    else
      echo -e "${RED}FAIL${NC}"
      TESTS_FAILED=$((TESTS_FAILED + 1))
      echo "  Command: $YAMLCMP_BIN $extra_args $file1 $file2"
      echo "  Failed with exit code $?"
    fi
  fi
}

# Test parsing individual files (should not crash)
test_parsing() {
  local test_name="$1"
  local file="$2"
  
  TESTS_RUN=$((TESTS_RUN + 1))
  
  echo -n "Parse test $TESTS_RUN: $test_name... "
  
  # Try to parse by comparing file to itself
  if $YAMLCMP_BIN "$file" "$file" >/dev/null 2>&1; then
    echo -e "${GREEN}PASS${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
  else
    echo -e "${RED}FAIL${NC} (parse error)"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo "  File: $file"
    echo "  Parser failed with exit code $?"
  fi
}

echo "=== YAML Parser Comprehensive Test Suite ==="
echo

# Test 1: Basic identical file comparison
echo "--- Basic Tests ---"
TMP1="$(mktemp)"
cat > "$TMP1" <<EOF
{ "foo": 42 }
EOF
cp "$TMP1" "$TMP1.copy"
run_test "Identical JSON files" "$TMP1" "$TMP1.copy" "pass"
rm -f "$TMP1" "$TMP1.copy"

# Test 2: Basic mapping comparison (YAML vs JSON)
echo
echo "--- Format Equivalence Tests ---"
run_test "Basic mapping YAML vs JSON" "$TEST_DATA_DIR/basic_mapping.yaml" "$TEST_DATA_DIR/basic_mapping_json.yaml" "pass"

# Test 3: Sequence comparison (YAML vs JSON)
run_test "Sequence YAML vs JSON" "$TEST_DATA_DIR/sequence_yaml.yaml" "$TEST_DATA_DIR/sequence_json.yaml" "pass"

# Test 4: Nested structures (YAML vs JSON)
run_test "Nested structures YAML vs JSON" "$TEST_DATA_DIR/nested_yaml.yaml" "$TEST_DATA_DIR/nested_json.yaml" "pass"

# Test 5: Scalar types (YAML vs JSON)
run_test "Scalar types YAML vs JSON" "$TEST_DATA_DIR/scalars_yaml.yaml" "$TEST_DATA_DIR/scalars_json.yaml" "pass"

# Test 6: Comments should be ignored
run_test "Comments ignored" "$TEST_DATA_DIR/comments_yaml.yaml" "$TEST_DATA_DIR/comments_clean.yaml" "pass"

echo
echo "--- Parsing Tests ---"
# Test individual file parsing
test_parsing "Basic YAML mapping" "$TEST_DATA_DIR/basic_mapping.yaml"
test_parsing "Basic JSON mapping" "$TEST_DATA_DIR/basic_mapping_json.yaml"
test_parsing "YAML sequence" "$TEST_DATA_DIR/sequence_yaml.yaml"
test_parsing "JSON sequence" "$TEST_DATA_DIR/sequence_json.yaml"
test_parsing "Nested YAML" "$TEST_DATA_DIR/nested_yaml.yaml"
test_parsing "Nested JSON" "$TEST_DATA_DIR/nested_json.yaml"
test_parsing "Scalar types YAML" "$TEST_DATA_DIR/scalars_yaml.yaml"
test_parsing "Scalar types JSON" "$TEST_DATA_DIR/scalars_json.yaml"
test_parsing "Comments in YAML" "$TEST_DATA_DIR/comments_yaml.yaml"
test_parsing "Mixed formats" "$TEST_DATA_DIR/mixed_formats.yaml"
test_parsing "Edge cases" "$TEST_DATA_DIR/edge_cases.yaml"

echo
echo "--- Subset Tests ---"
# Test subset functionality
TMP_SUBSET="$(mktemp)"
TMP_FULL="$(mktemp)"

cat > "$TMP_SUBSET" <<EOF
{
  "name": "Alice",
  "age": 25
}
EOF

cat > "$TMP_FULL" <<EOF
{
  "name": "Alice",
  "age": 25,
  "city": "Boston",
  "active": true
}
EOF

run_test "Subset comparison (should pass)" "$TMP_SUBSET" "$TMP_FULL" "pass" "--subset"
run_test "Reverse subset (should fail)" "$TMP_FULL" "$TMP_SUBSET" "fail" "--subset"

rm -f "$TMP_SUBSET" "$TMP_FULL"

echo
echo "--- Difference Tests ---"
# Test files that should be different
TMP_DIFF1="$(mktemp)"
TMP_DIFF2="$(mktemp)"

cat > "$TMP_DIFF1" <<EOF
name: "Alice"
age: 25
EOF

cat > "$TMP_DIFF2" <<EOF
name: "Bob"
age: 30
EOF

run_test "Different content (should fail)" "$TMP_DIFF1" "$TMP_DIFF2" "fail"

rm -f "$TMP_DIFF1" "$TMP_DIFF2"

echo
echo "--- Self-consistency Tests ---"
# Each file should be equal to itself
run_test "Basic mapping self-consistency" "$TEST_DATA_DIR/basic_mapping.yaml" "$TEST_DATA_DIR/basic_mapping.yaml" "pass"
run_test "Sequence self-consistency" "$TEST_DATA_DIR/sequence_yaml.yaml" "$TEST_DATA_DIR/sequence_yaml.yaml" "pass"
run_test "Nested structure self-consistency" "$TEST_DATA_DIR/nested_yaml.yaml" "$TEST_DATA_DIR/nested_yaml.yaml" "pass"
run_test "Mixed formats self-consistency" "$TEST_DATA_DIR/mixed_formats.yaml" "$TEST_DATA_DIR/mixed_formats.yaml" "pass"

echo
echo "=== Test Results ==="
echo "Tests run: $TESTS_RUN"
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"

if [[ $TESTS_FAILED -eq 0 ]]; then
  echo -e "${GREEN}✓ All tests passed!${NC}"
  echo "✔ yaml-cmp comprehensive test suite completed successfully"
  exit 0
else
  echo -e "${RED}✗ Some tests failed!${NC}"
  exit 1
fi 
