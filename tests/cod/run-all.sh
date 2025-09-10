#!/usr/bin/env bash
# run-all.sh – build and execute all cod C++ tests.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Set default COD_TEST_TOOLCHAIN if not already set
export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"

# Configure (only once)
if [[ ! -f "$BUILD_DIR/Build.ninja" && ! -f "$BUILD_DIR/Makefile" ]]; then
  mkdir -p "$BUILD_DIR"
  cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$BUILD_DIR" "$SCRIPT_DIR"
fi

# Build (incremental)
cmake --build "$BUILD_DIR"

echo "🏃  Running shell-based tests..."
# Execute shell-based tests first
for shell_test in "$SCRIPT_DIR"/test_*.sh; do
  if [[ -x "$shell_test" ]]; then
    echo "→ $(basename "$shell_test")"
    "$shell_test"
  fi
done

echo "🏃  Running C++ test executables..."
# Execute remaining C++ test executables (cache and workspace tests)
for exe in "$BUILD_DIR"/*; do
  if [[ -x "$exe" && ! -d "$exe" ]]; then
    exe_name=$(basename "$exe")
    # Skip tests that have been converted to shell scripts
    case "$exe_name" in
      test_cod_cli|test_cod_eval|test_cod_repl)
        echo "→ $exe_name (skipped - using shell version)"
        ;;
      *)
        echo "→ $exe_name"
        "$exe"
        ;;
    esac
  fi
done

echo "✔ All cod tests passed."