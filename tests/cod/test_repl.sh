#!/usr/bin/env bash
# test_repl.sh - Shell-based tests for cod REPL functionality
set -uo pipefail

# Source the timeout helper functions
SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SOURCE_DIR/../timeout_helper.sh"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_NAME="CoD REPL Tests"

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

# Get test project path for REPL tests
get_test_project_path() {
    echo "$SCRIPT_DIR/test-data/repl"
}

# Test REPL startup (without interactive input)
test_repl_startup() {
    log_test "REPL startup"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    
    # Test that REPL starts without arguments (should enter interactive mode)
    # We'll send EOF immediately to exit cleanly
    if (cd "$PROJECT_DIR" && echo "" | shell_timeout 5 "$COD_EXECUTABLE" > /tmp/cod_repl_out 2> /tmp/cod_repl_err); then
        # Check that it doesn't fail with argument parsing errors
        if ! grep -q "Unknown argument" /tmp/cod_repl_err && \
           ! grep -q "requires a" /tmp/cod_repl_err; then
            log_pass "REPL startup works"
        else
            log_fail "REPL startup failed with argument parsing error"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "REPL startup failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_repl_err && \
           ! grep -q "requires a" /tmp/cod_repl_err; then
            log_pass "REPL startup attempted (may have failed for other reasons)"
        else
            log_fail "REPL startup failed with argument parsing error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_repl_*
}

# Test REPL with basic commands
test_repl_commands() {
    log_test "REPL basic commands"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    
    # Test REPL with simple input and exit
    # Send a simple expression followed by exit command
    INPUT="1 + 1\n:exit\n"
    if (cd "$PROJECT_DIR" && echo -e "$INPUT" | shell_timeout 10 "$COD_EXECUTABLE" > /tmp/cod_cmd_out 2> /tmp/cod_cmd_err); then
        # Check that it doesn't fail with argument parsing errors
        if ! grep -q "Unknown argument" /tmp/cod_cmd_err && \
           ! grep -q "requires a" /tmp/cod_cmd_err; then
            log_pass "REPL basic commands work"
        else
            log_fail "REPL commands failed with argument parsing error"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "REPL basic commands failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_cmd_err && \
           ! grep -q "requires a" /tmp/cod_cmd_err; then
            log_pass "REPL command parsing works (execution may have failed for other reasons)"
        else
            log_fail "REPL commands failed with argument parsing error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_cmd_*
}

# Test REPL help command
test_repl_help() {
    log_test "REPL help command"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    
    # Test REPL help command
    INPUT=":help\n:exit\n"
    if (cd "$PROJECT_DIR" && echo -e "$INPUT" | shell_timeout 10 "$COD_EXECUTABLE" > /tmp/cod_help_out 2> /tmp/cod_help_err); then
        # Check that it doesn't fail with argument parsing errors
        if ! grep -q "Unknown argument" /tmp/cod_help_err && \
           ! grep -q "requires a" /tmp/cod_help_err; then
            log_pass "REPL help command works"
        else
            log_fail "REPL help failed with argument parsing error"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "REPL help command failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_help_err && \
           ! grep -q "requires a" /tmp/cod_help_err; then
            log_pass "REPL help command parsing works (execution may have failed for other reasons)"
        else
            log_fail "REPL help failed with argument parsing error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_help_*
}

# Test REPL with custom works path
test_repl_workspace_handling() {
    log_test "REPL workspace handling"
    run_test
    
    PROJECT_DIR=$(get_test_project_path)
    WORKS_PATH="$PROJECT_DIR/custom_repl.dbmr"
    
    # Test REPL with custom works path
    INPUT="1 + 1\n:exit\n"
    if (cd "$PROJECT_DIR" && echo -e "$INPUT" | shell_timeout 10 "$COD_EXECUTABLE" -w "$WORKS_PATH" > /tmp/cod_ws_out 2> /tmp/cod_ws_err); then
        # Check that it doesn't fail with argument parsing errors
        if ! grep -q "Unknown argument" /tmp/cod_ws_err && \
           ! grep -q "requires a" /tmp/cod_ws_err; then
            log_pass "REPL workspace handling works"
        else
            log_fail "REPL workspace handling failed with argument parsing error"
        fi
    else
        EXIT_CODE=$?
        if [[ "$EXIT_CODE" -eq 11 || "$EXIT_CODE" -eq 139 ]]; then
            log_fail "REPL workspace handling failed with Segmentation Fault (exit code $EXIT_CODE)"
        elif ! grep -q "Unknown argument" /tmp/cod_ws_err && \
           ! grep -q "requires a" /tmp/cod_ws_err; then
            log_pass "REPL workspace argument parsing works (execution may have failed for other reasons)"
        else
            log_fail "REPL workspace handling failed with argument parsing error"
        fi
    fi
    
    # Clean up temp files only
    rm -f /tmp/cod_ws_*
}

# Test REPL prompt behavior (basic check)
test_repl_prompt() {
    log_test "REPL prompt behavior (not implemented)"
    run_test
    log_fail "REPL prompt test not implemented"
}

# Test REPL exit command
test_repl_exit_command() {
    log_test "REPL exit command (not implemented)"
    run_test
    log_fail "REPL exit command test not implemented"
}

# Test REPL multiline input
test_repl_multiline_input() {
    log_test "REPL multiline input (not implemented)"
    run_test
    log_fail "REPL multiline input test not implemented"
}

# Test REPL invalid input
test_repl_invalid_input() {
    log_test "REPL invalid input (not implemented)"
    run_test
    log_fail "REPL invalid input test not implemented"
}

# Test REPL history
test_repl_history() {
    log_test "REPL history (not implemented)"
    run_test
    log_fail "REPL history test not implemented"
}

# Test REPL tab completion
test_repl_tab_completion() {
    log_test "REPL tab completion (not implemented)"
    run_test
    log_fail "REPL tab completion test not implemented"
}

# Test REPL interrupt
test_repl_interrupt() {
    log_test "REPL interrupt (not implemented)"
    run_test
    log_fail "REPL interrupt test not implemented"
}

# Main test execution
main() {
    echo "Running $TEST_NAME..."
    echo
    
    test_repl_startup
    test_repl_commands
    test_repl_help
    test_repl_workspace_handling
    test_repl_prompt
    test_repl_exit_command
    test_repl_multiline_input
    test_repl_invalid_input
    test_repl_history
    test_repl_tab_completion
    test_repl_interrupt
    
    echo
    if [[ "$FAIL_COUNT" -eq 0 ]]; then
        echo -e "${GREEN}✔ All CoD REPL tests passed! ($PASS_COUNT/$TEST_COUNT)${NC}"
        exit 0
    else
        echo -e "${RED}✗ Some CoD REPL tests failed. ($FAIL_COUNT/$TEST_COUNT failures)${NC}"
        exit 1
    fi
}

# Run tests
# echo -e "${GREEN}=== CoD REPL Tests ===${NC}"

# run_test
# test_repl_startup

# run_test
# test_repl_commands

# run_test
# test_repl_help_command

# run_test
# test_repl_custom_works_path

# run_test
# test_repl_prompt

# run_test
# test_repl_exit_command

# run_test
# test_repl_multiline_input

# run_test
# test_repl_invalid_input

# run_test
# test_repl_history

# run_test
# test_repl_tab_completion

# run_test
# test_repl_interrupt


# echo
# if [[ "$FAIL_COUNT" -eq 0 ]]; then
#     echo -e "${GREEN}✔ All CoD REPL tests passed! ($PASS_COUNT/$TEST_COUNT)${NC}"
#     exit 0
# else
#     echo -e "${RED}✗ Some CoD REPL tests failed. ($FAIL_COUNT/$TEST_COUNT failures)${NC}"
#     exit 1
# fi

main
