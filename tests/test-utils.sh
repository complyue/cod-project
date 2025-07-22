#!/usr/bin/env bash

# Function to set up COD_TEST_TOOLCHAIN with default if not set
setup_toolchain() {
  export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"
}

# Function to configure build directory if first run
setup_build_dir() {
  local build_dir="$1"
  local script_dir="$2"
  
  if [[ ! -f "$build_dir/Build.ninja" && ! -f "$build_dir/Makefile" ]]; then
    mkdir -p "$build_dir"
    cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$build_dir" "$script_dir"
  fi
}
