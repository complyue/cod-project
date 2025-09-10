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

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TEST_COUNT=0
PASS_COUNT=0

log_test() {
    echo -e "${YELLOW}Testing $1...${NC}"
}

log_pass() {
    echo -e "${GREEN}✓ $1${NC}"
    ((PASS_COUNT++))
}

log_fail() {
    echo -e "${RED}✗ $1${NC}"
    exit 1
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
        log_fail "--help option failed"
    fi
    
    # Test -h (short form)
    if "$COD_EXECUTABLE" -h > /tmp/cod_help_short 2>&1; then
        if grep -q "Usage:" /tmp/cod_help_short; then
            log_pass "-h option works correctly"
        else
            log_fail "-h output missing expected content"
        fi
    else
        log_fail "-h option failed"
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
        if grep -q "Unknown argument" /tmp/cod_unknown_err; then
            log_pass "--unknown argument properly rejected"
        else
            log_fail "--unknown error message incorrect"
        fi
    fi
    
    # Test missing argument for -w
    if "$COD_EXECUTABLE" -w > /tmp/cod_w_out 2> /tmp/cod_w_err; then
        log_fail "-w without argument should have failed but didn't"
    else
        if grep -q "requires a path argument" /tmp/cod_w_err; then
            log_pass "-w missing argument properly detected"
        else
            log_fail "-w error message incorrect"
        fi
    fi
    
    # Test missing argument for -e
    if "$COD_EXECUTABLE" -e > /tmp/cod_e_out 2> /tmp/cod_e_err; then
        log_fail "-e without argument should have failed but didn't"
    else
        if grep -q "requires an expression argument" /tmp/cod_e_err; then
            log_pass "-e missing argument properly detected"
        else
            log_fail "-e error message incorrect"
        fi
    fi
    
    # Test missing argument for --project
    if "$COD_EXECUTABLE" --project > /tmp/cod_project_out 2> /tmp/cod_project_err; then
        log_fail "--project without argument should have failed but didn't"
    else
        if grep -q "requires a path argument" /tmp/cod_project_err; then
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
    
    # Create a temporary directory for testing
    TEMP_DIR=$(mktemp -d)
    
    # Create a minimal CodProject.yaml
    cat > "$TEMP_DIR/CodProject.yaml" << EOF
name: test_project
version: 1.0.0
EOF
    
    # Test with custom works path (should not fail on argument parsing)
    WORKS_PATH="$TEMP_DIR/test.dbmr"
    if "$COD_EXECUTABLE" --project "$TEMP_DIR" -w "$WORKS_PATH" -e "1 + 1" > /tmp/cod_args_out 2> /tmp/cod_args_err; then
        # Command might succeed or fail due to missing dependencies, but should not fail on argument parsing
        if ! grep -q "Unknown argument" /tmp/cod_args_err && \
           ! grep -q "requires a" /tmp/cod_args_err; then
            log_pass "argument parsing works correctly"
        else
            log_fail "argument parsing failed"
        fi
    else
        # Even if command fails, check that it's not due to argument parsing errors
        if ! grep -q "Unknown argument" /tmp/cod_args_err && \
           ! grep -q "requires a" /tmp/cod_args_err; then
            log_pass "argument parsing works correctly (command failed for other reasons)"
        else
            log_fail "argument parsing failed"
        fi
    fi
    
    # Clean up
    rm -rf "$TEMP_DIR"
    rm -f /tmp/cod_args_*
}

# Main test execution
main() {
    echo "Running $TEST_NAME..."
    echo
    
    test_help_option
    test_invalid_arguments
    test_argument_parsing
    
    echo
    echo "✔ All CLI tests passed! ($PASS_COUNT/$TEST_COUNT)"
}

# Run tests
main "$@"
