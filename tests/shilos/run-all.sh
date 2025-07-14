#!/usr/bin/env bash
# run-all.sh – build and execute all shilos C++ tests.
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

echo "🏃  Running shilos test executables..."
# Execute every built executable directly under build/ (ignore directories)
for exe in "$BUILD_DIR"/*; do
  if [[ -x "$exe" && ! -d "$exe" ]]; then
    echo "→ $(basename "$exe")"
    "$exe"
  fi
done

echo "✔ All shilos tests passed." 
