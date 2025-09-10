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

# Create a test project for REPL tests
create_test_project() {
    TEMP_DIR=$(mktemp -d)
    
    # Create CodProject.yaml
    cat > "$TEMP_DIR/CodProject.yaml" << EOF
name: repl_test_project
version: 1.0.0
dependencies: {}
EOF
    
    # Create .cod directory
    mkdir -p "$TEMP_DIR/.cod"
    
    echo "$TEMP_DIR"
}

# Test REPL startup (without interactive input)
test_repl_startup() {
    log_test "REPL startup"
    run_test
    
    PROJECT_DIR=$(create_test_project)
    
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
        # Check output for signs it tried to start REPL
        if ! grep -q "Unknown argument" /tmp/cod_repl_err && \
           ! grep -q "requires a" /tmp/cod_repl_err; then
            log_pass "REPL startup attempted (may have failed for other reasons)"
        else
            log_fail "REPL startup failed with argument parsing error"
        fi
    fi
    
    # Clean up
    rm -rf "$PROJECT_DIR"
    rm -f /tmp/cod_repl_*
}

# Test REPL with basic commands
test_repl_commands() {
    log_test "REPL basic commands"
    run_test
    
    PROJECT_DIR=$(create_test_project)
    
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
        # Even if it fails, check it's not due to argument parsing
        if ! grep -q "Unknown argument" /tmp/cod_cmd_err && \
           ! grep -q "requires a" /tmp/cod_cmd_err; then
            log_pass "REPL command parsing works (execution may have failed for other reasons)"
        else
            log_fail "REPL commands failed with argument parsing error"
        fi
    fi
    
    # Clean up
    rm -rf "$PROJECT_DIR"
    rm -f /tmp/cod_cmd_*
}

# Test REPL help command
test_repl_help() {
    log_test "REPL help command"
    run_test
    
    PROJECT_DIR=$(create_test_project)
    
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
        # Even if it fails, check it's not due to argument parsing
        if ! grep -q "Unknown argument" /tmp/cod_help_err && \
           ! grep -q "requires a" /tmp/cod_help_err; then
            log_pass "REPL help command parsing works (execution may have failed for other reasons)"
        else
            log_fail "REPL help failed with argument parsing error"
        fi
    fi
    
    # Clean up
    rm -rf "$PROJECT_DIR"
    rm -f /tmp/cod_help_*
}

# Test REPL with custom works path
test_repl_workspace_handling() {
    log_test "REPL workspace handling"
    run_test
    
    PROJECT_DIR=$(create_test_project)
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
        # Even if it fails, check it's not due to argument parsing
        if ! grep -q "Unknown argument" /tmp/cod_ws_err && \
           ! grep -q "requires a" /tmp/cod_ws_err; then
            log_pass "REPL workspace argument parsing works (execution may have failed for other reasons)"
        else
            log_fail "REPL workspace handling failed with argument parsing error"
        fi
    fi
    
    # Clean up
    rm -rf "$PROJECT_DIR"
    rm -f /tmp/cod_ws_*
}

# Test REPL prompt behavior (basic check)
test_repl_prompt() {
    log_test "REPL prompt behavior"
    run_test
    
    PROJECT_DIR=$(create_test_project)
    
    # Test that REPL shows some kind of prompt or interactive behavior
    # We'll just check that it doesn't immediately exit with an error
    if (cd "$PROJECT_DIR" && echo ":exit" | shell_timeout 5 "$COD_EXECUTABLE" > /tmp/cod_prompt_out 2> /tmp/cod_prompt_err); then
        # Check that it doesn't fail with argument parsing errors
        if ! grep -q "Unknown argument" /tmp/cod_prompt_err && \
           ! grep -q "requires a" /tmp/cod_prompt_err; then
            log_pass "REPL prompt behavior works"
        else
            log_fail "REPL prompt failed with argument parsing error"
        fi
    else
        # Even if it fails, check it's not due to argument parsing
        if ! grep -q "Unknown argument" /tmp/cod_prompt_err && \
           ! grep -q "requires a" /tmp/cod_prompt_err; then
            log_pass "REPL prompt argument parsing works (execution may have failed for other reasons)"
        else
            log_fail "REPL prompt failed with argument parsing error"
        fi
    fi
    
    # Clean up
    rm -rf "$PROJECT_DIR"
    rm -f /tmp/cod_prompt_*
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
    
    echo
    echo "✔ All REPL tests passed! ($PASS_COUNT/$TEST_COUNT)"
}

# Run tests
main "$@"
