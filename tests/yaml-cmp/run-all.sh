#!/usr/bin/env bash
# run-all.sh – build yaml-cmp utility and run a quick self-check.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Configure build directory if first run
if [[ ! -f "$BUILD_DIR/Build.ninja" && ! -f "$BUILD_DIR/Makefile" ]]; then
  mkdir -p "$BUILD_DIR"
  cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B "$BUILD_DIR" "$SCRIPT_DIR"
fi

# Build yaml-cmp
cmake --build "$BUILD_DIR"
YAMLCMP_BIN="$BUILD_DIR/yaml-cmp"

# Basic sanity test: compare two identical YAML docs
TMP1="$(mktemp)"; TMP2="$(mktemp)"
cat > "$TMP1" <<EOF
{ "foo": 42 }
EOF
cp "$TMP1" "$TMP2"
"$YAMLCMP_BIN" "$TMP1" "$TMP2"
rm -f "$TMP1" "$TMP2"

echo "✔ yaml-cmp built and self-checked successfully at $YAMLCMP_BIN" 
