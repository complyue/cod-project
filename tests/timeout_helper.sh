#!/bin/bash

# Portable timeout function for shell scripts
# Works on macOS and other Unix systems without requiring GNU timeout

# Usage: shell_timeout <seconds> <command> [args...]
# Returns: exit code of the command, or 124 if timeout occurred
shell_timeout() {
    local timeout_duration="$1"
    shift
    
    # Start the command in background
    "$@" &
    local cmd_pid=$!
    
    # Start a timeout process in background
    (
        sleep "$timeout_duration"
        # Check if the process is still running
        if kill -0 "$cmd_pid" 2>/dev/null; then
            # Kill the process group to ensure all child processes are terminated
            kill -TERM "-$cmd_pid" 2>/dev/null || kill -TERM "$cmd_pid" 2>/dev/null
            sleep 1
            # Force kill if still running
            kill -KILL "-$cmd_pid" 2>/dev/null || kill -KILL "$cmd_pid" 2>/dev/null
        fi
    ) &
    local timeout_pid=$!
    
    # Wait for the command to complete
    local exit_code=0
    if wait "$cmd_pid" 2>/dev/null; then
        # Command completed successfully, kill the timeout process
        kill "$timeout_pid" 2>/dev/null
        wait "$timeout_pid" 2>/dev/null
        exit_code=$?
    else
        # Command was killed or failed
        exit_code=$?
        # Kill the timeout process
        kill "$timeout_pid" 2>/dev/null
        wait "$timeout_pid" 2>/dev/null
        
        # If exit code indicates the process was terminated, return timeout exit code
        if [ $exit_code -eq 143 ] || [ $exit_code -eq 137 ]; then
            exit_code=124  # Standard timeout exit code
        fi
    fi
    
    return $exit_code
}

# Alternative simpler timeout function using a different approach
# Usage: simple_timeout <seconds> <command> [args...]
simple_timeout() {
    local timeout_duration="$1"
    shift
    
    # Use a subshell with timeout logic
    (
        "$@" &
        local cmd_pid=$!
        
        # Sleep for timeout duration in background
        sleep "$timeout_duration" &
        local sleep_pid=$!
        
        # Wait for either command or sleep to finish
        wait -n "$cmd_pid" "$sleep_pid" 2>/dev/null
        local first_exit=$?
        
        # Check which one finished first
        if kill -0 "$cmd_pid" 2>/dev/null; then
            # Command is still running, timeout occurred
            kill "$cmd_pid" 2>/dev/null
            wait "$cmd_pid" 2>/dev/null
            exit 124  # Timeout exit code
        else
            # Command finished first
            kill "$sleep_pid" 2>/dev/null
            wait "$sleep_pid" 2>/dev/null
            exit $first_exit
        fi
    )
}
