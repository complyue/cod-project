#!/usr/bin/env bash
# local_single/run.sh – simple single local dependency test.

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

CODP_BIN="$(command -v codp)"
if [[ -z "$CODP_BIN" ]]; then
  echo "codp not found in PATH" >&2
  exit 1
fi

# Ensure yaml-cmp is available
YAMLCMP_BIN="$(command -v yaml-cmp || true)"
if [[ -z "$YAMLCMP_BIN" ]]; then
  # Fallback to build dir of yaml-cmp standalone test
  YAMLCMP_BIN="$PROJECT_ROOT/tests/yaml-cmp/build/yaml-cmp"
fi
if [[ ! -x "$YAMLCMP_BIN" ]]; then
  echo "yaml-cmp not found – build tests/yaml-cmp first" >&2
  exit 1
fi

ROOT_DIR="$SCRIPT_DIR/data/root"

# Run codp directly inside root project dir
(cd "$ROOT_DIR" && codp solve)

MANIFEST="$ROOT_DIR/CodManifest.yaml"
if [[ ! -f "$MANIFEST" ]]; then
  echo "CodManifest.yaml not generated" >&2
  exit 1
fi


# ---------------------------------------------------------------------------
# Compare YAML manifest content using yaml-cmp (subset comparison).
# Use static expected manifest file with relative paths.
# ---------------------------------------------------------------------------

EXPECTED_YAML="$SCRIPT_DIR/expected_manifest.yaml"

# Use yaml-cmp in subset mode – expected YAML must be contained in actual.
"$YAMLCMP_BIN" --subset "$EXPECTED_YAML" "$MANIFEST"

echo "local_single test passed" 
