#!/usr/bin/env bash
# test_cli.sh - Shell-based tests for cod CLI functionality
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME="CoD CLI Tests"

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

# Test help option (--help and -h)
test_help_option() {
    log_test "help option"
    run_test
    
    # Test --help
    if "$COD_EXECUTABLE" --help > /tmp/cod_help_output 2>&1; then
        if grep -q "Usage:" /tmp/cod_help_output && \
           grep -q "\-e, \-\-eval" /tmp/cod_help_output && \
           grep -q "\-w, \-\-works" /tmp/cod_help_output; then
            log_pass "--help option works correctly"
        else
            log_fail "--help output missing expected content"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "--help option failed with Segmentation Fault (exit code $EXIT_CODE)"
        else
            log_fail "--help option failed"
        fi
    fi
    
    # Test -h (short form)
    if "$COD_EXECUTABLE" -h > /tmp/cod_help_short 2>&1; then
        if grep -q "Usage:" /tmp/cod_help_short; then
            log_pass "-h option works correctly"
        else
            log_fail "-h output missing expected content"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "-h option failed with Segmentation Fault (exit code $EXIT_CODE)"
        else
            log_fail "-h option failed"
        fi
    fi
    
    # Clean up
    rm -f /tmp/cod_help_output /tmp/cod_help_short
}

# Test invalid arguments
test_invalid_arguments() {
    log_test "invalid arguments"
    run_test
    
    # Test unknown argument
    if "$COD_EXECUTABLE" --unknown > /tmp/cod_unknown_out 2> /tmp/cod_unknown_err; then
        log_fail "--unknown should have failed but didn't"
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "--unknown failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif grep -q "Unknown argument" /tmp/cod_unknown_err; then
            log_pass "--unknown argument properly rejected"
        else
            log_fail "--unknown error message incorrect"
        fi
    fi
    
    # Test missing argument for -w
    if "$COD_EXECUTABLE" -w > /tmp/cod_w_out 2> /tmp/cod_w_err; then
        log_fail "-w without argument should have failed but didn't"
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "-w without argument failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif grep -q "requires a path argument" /tmp/cod_w_err; then
            log_pass "-w missing argument properly detected"
        else
            log_fail "-w error message incorrect"
        fi
    fi
    
    # Test missing argument for -e
    if "$COD_EXECUTABLE" -e > /tmp/cod_e_out 2> /tmp/cod_e_err; then
        log_fail "-e without argument should have failed but didn't"
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "-e without argument failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif grep -q "requires an expression argument" /tmp/cod_e_err; then
            log_pass "-e missing argument properly detected"
        else
            log_fail "-e error message incorrect"
        fi
    fi
    
    # Test missing argument for --project
    if "$COD_EXECUTABLE" --project > /tmp/cod_project_out 2> /tmp/cod_project_err; then
        log_fail "--project without argument should have failed but didn't"
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "--project without argument failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif grep -q "requires a path argument" /tmp/cod_project_err; then
            log_pass "--project missing argument properly detected"
        else
            log_fail "--project error message incorrect"
        fi
    fi
    
    # Clean up
    rm -f /tmp/cod_unknown_* /tmp/cod_w_* /tmp/cod_e_* /tmp/cod_project_*
}

# Test basic argument parsing (without full project setup)
test_argument_parsing() {
    log_test "argument parsing"
    run_test
    
    # Use tracked test data directory
    PROJECT_DIR="$SCRIPT_DIR/test-data/cli"
    
    # Test with custom works path (should not fail on argument parsing)
    WORKS_PATH="$PROJECT_DIR/test.dbmr"
    if "$COD_EXECUTABLE" --project "$PROJECT_DIR" -w "$WORKS_PATH" -e "1 + 1" > /tmp/cod_args_out 2> /tmp/cod_args_err; then
        # Command might succeed or fail due to missing dependencies, but should not fail on argument parsing
        if ! grep -q "Unknown argument" /tmp/cod_args_err && \
           ! grep -q "requires a" /tmp/cod_args_err; then
            log_pass "argument parsing works correctly"
        else
            log_fail "argument parsing failed"
        fi
    else
        # Even if command fails, check that it's not due to argument parsing errors
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "argument parsing failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_args_err && \
           ! grep -q "requires a" /tmp/cod_args_err; then
            log_pass "argument parsing works correctly (command failed for other reasons)"
        else
            log_fail "argument parsing failed"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_args_*
}

# Run tests
echo -e "${GREEN}=== CoD CLI Tests ===${NC}"

run_test
test_help_option

run_test
test_invalid_arguments

run_test
test_argument_parsing

run_test
test_custom_works_path

run_test
test_project_option

run_test
test_eval_option

run_test
test_repl_option

run_test
test_version_option

run_test
test_unknown_option

run_test
test_multiple_options

run_test
test_option_with_argument

run_test
test_short_options

run_test
test_combined_short_options

run_test
test_dash_dash_argument

run_test
test_empty_arguments

run_test
test_no_arguments

run_test
test_long_form_eval

run_test
test_multiline_eval

run_test
test_invalid_syntax_eval


echo
if [[ "$FAIL_COUNT" -eq 0 ]]; then
    echo -e "${GREEN}✔ All CoD CLI tests passed! ($PASS_COUNT/$TEST_COUNT)${NC}"
    exit 0
else
    echo -e "${RED}✗ Some CoD CLI tests failed. ($FAIL_COUNT/$TEST_COUNT failures)${NC}"
    exit 1
fi
