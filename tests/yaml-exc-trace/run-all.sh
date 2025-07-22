#!/usr/bin/env bash
# run-all.sh – YAML exception trace test suite
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Source test utilities and setup toolchain
source "$SCRIPT_DIR/../test-utils.sh"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"

# Configure build directory using utility function
setup_build_dir "$BUILD_DIR" "$SCRIPT_DIR"

# Build the test
echo "Building yaml-exc-trace tests..."
cmake --build "$BUILD_DIR"

# Verify test executable exists
TEST_BIN="$BUILD_DIR/yaml-exc-trace-test"
if [[ ! -x "$TEST_BIN" ]]; then
  echo -e "${RED}✗ Failed to build yaml-exc-trace-test${NC}"
  exit 1
fi

echo -e "${GREEN}✓ yaml-exc-trace-test built successfully${NC}"
echo

# Run the test
echo "=== YAML Exception Trace Test Suite ==="
echo
echo "Running yaml-exc-trace tests..."
if "$TEST_BIN" "$@"; then
  echo
  echo -e "${GREEN}✓ All yaml-exc-trace tests passed!${NC}"
  exit 0
else
  echo
  echo -e "${RED}✗ Some yaml-exc-trace tests failed!${NC}"
  exit 1
fi
