#!/usr/bin/env sh

# Setup COD_TEST_TOOLCHAIN with default if not set
export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

# Setup PATH for project utilities
# Use POSIX-compliant method to get script's directory
# Get absolute path to project root using a subshell to avoid directory changes
# Reliable method to get script's directory that works when sourced
# Uses BASH_SOURCE if available, otherwise falls back to $0
SCRIPT_PATH="${BASH_SOURCE[0]:-$0}"
SCRIPT_DIR="$(dirname -- "$SCRIPT_PATH")"

# Get absolute path to project root (parent of tests directory)
COD_PROJECT_SRC_ROOT="$(
  cd "$SCRIPT_DIR/.." && pwd -P
)"

# Prepend both build/ and built/ bin directories to PATH
export PATH="${COD_PROJECT_SRC_ROOT}/build/bin:${COD_PROJECT_SRC_ROOT}/built/bin:${PATH}"


# Function to configure build directory for components to be built with CMake
setup_build_dir() {
  local build_dir="$1"
  local script_dir="$2"
  
  if [ ! -f "$build_dir/Build.ninja" ]; then
    mkdir -p "$build_dir"
    cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$build_dir" "$script_dir"
  fi
}
