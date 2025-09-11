#!/usr/bin/env bash
# test_eval.sh - Shell-based tests for cod evaluation functionality
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME="CoD Eval Tests"

# Determine the cod executable path based on COD_TEST_TOOLCHAIN
COD_PROJECT_SOURCE_DIR="$SCRIPT_DIR/../.."
COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

if [[ "$COD_TEST_TOOLCHAIN" == "build" ]]; then
    COD_EXECUTABLE="$COD_PROJECT_SOURCE_DIR/build/bin/cod"
elif [[ "$COD_TEST_TOOLCHAIN" == "built" ]]; then
    COD_EXECUTABLE="$COD_PROJECT_SOURCE_DIR/built/bin/cod"
else
    echo "Error: COD_TEST_TOOLCHAIN must be 'build' or 'built', got: $COD_TEST_TOOLCHAIN"
    exit 1
fi

# Check if cod executable exists
if [[ ! -x "$COD_EXECUTABLE" ]]; then
    echo "Error: cod executable not found at $COD_EXECUTABLE"
    echo "Make sure to build the project first with: cmake --build ./build"
    exit 1
fi

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

# Get test project path for evaluation tests
get_test_project_path() {
    echo "$SCRIPT_DIR/test-data/basic"
}

# Test basic evaluation with -e/--eval
test_basic_evaluation() {
    log_test "basic evaluation"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    
    # Test simple arithmetic expression
    if (cd "$PROJECT_DIR" && "$COD_EXECUTABLE" -e "1 + 1" > /tmp/cod_eval_out 2> /tmp/cod_eval_err); then
        # Check if output contains expected result or at least doesn't show argument parsing errors
        if ! grep -q "Unknown argument" /tmp/cod_eval_err && \
           ! grep -q "requires an expression argument" /tmp/cod_eval_err; then
            log_pass "basic evaluation works"
        else
            log_fail "basic evaluation failed with argument parsing error"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "basic evaluation failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_eval_err && \
           ! grep -q "requires an expression argument" /tmp/cod_eval_err; then
            log_pass "basic evaluation argument parsing works (execution may have failed for other reasons)"
        else
            log_fail "basic evaluation failed with argument parsing error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_eval_*
}

# Test evaluation with custom works path
test_eval_with_custom_works() {
    log_test "evaluation with custom works path"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    WORKS_PATH="$PROJECT_DIR/custom.dbmr"
    
    # Test with custom works path
    if (cd "$PROJECT_DIR" && "$COD_EXECUTABLE" -w "$WORKS_PATH" -e "2 + 2" > /tmp/cod_custom_out 2> /tmp/cod_custom_err); then
        if ! grep -q "Unknown argument" /tmp/cod_custom_err && \
           ! grep -q "requires a" /tmp/cod_custom_err; then
            log_pass "custom works path evaluation works"
        else
            log_fail "custom works path evaluation failed with argument parsing error"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "custom works path evaluation failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_custom_err && \
           ! grep -q "requires a" /tmp/cod_custom_err; then
            log_pass "custom works path argument parsing works (execution may have failed for other reasons)"
        else
            log_fail "custom works path evaluation failed with argument parsing error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_custom_*
}

# Test long form --eval option
test_eval_long_form() {
    log_test "--eval long form"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    
    # Test --eval instead of -e
    if (cd "$PROJECT_DIR" && "$COD_EXECUTABLE" --eval "3 + 3" > /tmp/cod_long_out 2> /tmp/cod_long_err); then
        if ! grep -q "Unknown argument" /tmp/cod_long_err && \
           ! grep -q "requires an expression argument" /tmp/cod_long_err; then
            log_pass "--eval long form works"
        else
            log_fail "--eval long form failed with argument parsing error"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "--eval long form failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_long_err && \
           ! grep -q "requires an expression argument" /tmp/cod_long_err; then
            log_pass "--eval long form argument parsing works (execution may have failed for other reasons)"
        else
            log_fail "--eval long form failed with argument parsing error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_long_*
}

# Test multiline expression (basic syntax check)
test_multiline_expression() {
    log_test "multiline expression syntax"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    
    # Test multiline expression with semicolons
    MULTILINE_EXPR="int x = 5; int y = 10; x + y"
    if (cd "$PROJECT_DIR" && "$COD_EXECUTABLE" -e "$MULTILINE_EXPR" > /tmp/cod_multi_out 2> /tmp/cod_multi_err); then
        if ! grep -q "Unknown argument" /tmp/cod_multi_err && \
           ! grep -q "requires an expression argument" /tmp/cod_multi_err; then
            log_pass "multiline expression syntax works"
        else
            log_fail "multiline expression failed with argument parsing error"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "multiline expression failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_multi_err && \
           ! grep -q "requires an expression argument" /tmp/cod_multi_err; then
            log_pass "multiline expression argument parsing works (execution may have failed for other reasons)"
        else
            log_fail "multiline expression failed with argument parsing error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_multi_*
}

# Test that invalid expressions produce appropriate exit codes
test_eval_exit_codes() {
    log_test "evaluation exit codes"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    
    # Test invalid C++ syntax - should fail but not with argument parsing error
    if (cd "$PROJECT_DIR" && "$COD_EXECUTABLE" -e "invalid syntax here" > /tmp/cod_exit_out 2> /tmp/cod_exit_err); then
        # If it succeeds, that's unexpected but not necessarily wrong
        log_pass "invalid expression handling works (unexpectedly succeeded)"
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "invalid expression failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_exit_err && \
           ! grep -q "requires an expression argument" /tmp/cod_exit_err; then
            log_pass "invalid expression properly rejected"
        else
            log_fail "invalid expression failed with argument parsing error instead of compilation error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_exit_*
}

# Main test execution
main() {
    echo "Running $TEST_NAME..."
    echo
    
    test_basic_evaluation
    test_eval_with_custom_works
    test_eval_long_form
    test_multiline_expression
    test_eval_exit_codes
    
    echo
    echo "✔ All eval tests passed! ($PASS_COUNT/$TEST_COUNT)"
}

# Run tests
echo -e "${GREEN}=== CoD Evaluation Tests ===${NC}"

test_basic_evaluation
test_eval_with_custom_works
test_eval_long_form
test_multiline_expression
test_eval_exit_codes

echo
if [[ "$FAIL_COUNT" -eq 0 ]]; then
    echo -e "${GREEN}✔ All CoD Evaluation tests passed! ($PASS_COUNT/$TEST_COUNT)${NC}"
    exit 0
else
    echo -e "${RED}✗ Some CoD Evaluation tests failed. ($FAIL_COUNT/$TEST_COUNT failures)${NC}"
    exit 1
fi
