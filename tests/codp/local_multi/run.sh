#!/usr/bin/env bash
# local_multi/run.sh – complex multi-local dependency graph test.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# Determine toolchain directory based on COD_TEST_TOOLCHAIN environment variable
# Default to "build" for development workflow, can be set to "built" for pre-release testing
COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

if [[ "$COD_TEST_TOOLCHAIN" == "build" ]]; then
    TOOLCHAIN_DIR="$PROJECT_ROOT/build"
elif [[ "$COD_TEST_TOOLCHAIN" == "built" ]]; then
    TOOLCHAIN_DIR="$PROJECT_ROOT/built"
else
    echo "COD_TEST_TOOLCHAIN must be 'build' or 'built', got: $COD_TEST_TOOLCHAIN" >&2
    exit 1
fi

export PATH="$TOOLCHAIN_DIR/bin:$PATH"

if ! command -v codp >/dev/null; then
  echo "codp not found in PATH" >&2
  exit 1
fi

if ! command -v yaml-cmp >/dev/null; then
  YAMLCMP_BIN="$PROJECT_ROOT/tests/yaml-cmp/build/yaml-cmp"
else
  YAMLCMP_BIN="$(command -v yaml-cmp)"
fi
if [[ ! -x "$YAMLCMP_BIN" ]]; then
  echo "yaml-cmp not found – build tests/yaml-cmp first" >&2
  exit 1
fi

ROOT_DIR="$SCRIPT_DIR/data/root"

(cd "$ROOT_DIR" && codp solve)

MANIFEST="$ROOT_DIR/CodManifest.yaml"
if [[ ! -f "$MANIFEST" ]]; then
  echo "Manifest not generated" >&2
  exit 1
fi

# ---------------------------------------------------------------------------
# Compare YAML manifest content using yaml-cmp (subset comparison).
# Use static expected manifest file with relative paths.
# ---------------------------------------------------------------------------

EXPECTED_YAML="$SCRIPT_DIR/expected_manifest.yaml"

"$YAMLCMP_BIN" --subset "$EXPECTED_YAML" "$MANIFEST"

echo "local_multi test passed" 
