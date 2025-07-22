#!/usr/bin/env bash

# Setup COD_TEST_TOOLCHAIN with default if not set
export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

# Setup PATH for project utilities
COD_PROJECT_SRC_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Prepend both build/ and built/ bin directories to PATH
export PATH="${COD_PROJECT_SRC_ROOT}/build/bin:${COD_PROJECT_SRC_ROOT}/built/bin:${PATH}"


# Function to configure build directory for components to be built with CMake
setup_build_dir() {
  local build_dir="$1"
  local script_dir="$2"
  
  if [[ ! -f "$build_dir/Build.ninja" ]]; then
    mkdir -p "$build_dir"
    cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$build_dir" "$script_dir"
  fi
}
