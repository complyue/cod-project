#!/usr/bin/env sh

# Setup COD_TEST_TOOLCHAIN with default if not set
export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

# Setup PATH for project utilities
# Use POSIX-compliant method to get script's directory
# Get absolute path to project root using a subshell to avoid directory changes
# Reliable method to get script's directory that works when sourced
# Uses BASH_SOURCE if available, otherwise falls back to $0
# Get absolute path to project root (parent of tests directory)
COD_PROJECT_SRC_ROOT="$(
  cd "$(dirname -- "${BASH_SOURCE[0]:-$0}")/.." && pwd -P
)"

case "$COD_TEST_TOOLCHAIN" in
  build|built)
    export PATH="${COD_PROJECT_SRC_ROOT}/${COD_TEST_TOOLCHAIN}/bin:${PATH}"
    ;;
  *)
    echo "COD_TEST_TOOLCHAIN must be 'build' or 'built', got: $COD_TEST_TOOLCHAIN" >&2
    return 1 2>/dev/null || exit 1
    ;;
esac

resolve_test_exe() {
  if [ -x "$1" ]; then
    printf '%s\n' "$1"
    return 0
  fi

  if [ -x "$1.exe" ]; then
    printf '%s\n' "$1.exe"
    return 0
  fi

  return 1
}

# Function to configure build directory for components to be built with CMake
setup_build_dir() {
  local build_dir="$1"
  local script_dir="$2"
  
  mkdir -p "$build_dir"
  cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$build_dir" "$script_dir"
}
