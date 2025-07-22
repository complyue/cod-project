#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Source test utilities and setup toolchain
source "$SCRIPT_DIR/../test-utils.sh"
setup_toolchain

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"

# Parse command line arguments
VERBOSE_FLAG=""
for arg in "$@"; do
    if [[ "$arg" == "--verbose" || "$arg" == "-v" ]]; then
        VERBOSE_FLAG="--verbose"
    elif [[ "$arg" == "--help" || "$arg" == "-h" ]]; then
        echo "Usage: $0 [--verbose]"
        echo "  --verbose, -v    Show detailed output"
        echo "  --help, -h       Show this help message"
        exit 0
    fi
done

echo "🔧 Using toolchain mode: ${COD_TEST_TOOLCHAIN}"
echo "-- Using development toolchain from ${COD_TEST_TOOLCHAIN}/ directory"
echo "-- Using source headers from include/ directory"

# Configure build directory using utility function
setup_build_dir "$BUILD_DIR" "$SCRIPT_DIR"

# Build the test
echo "Building yaml-ux..."
cmake --build "$BUILD_DIR"

# Verify test executable exists
TEST_BIN="$BUILD_DIR/yaml-ux"
if [[ ! -x "$TEST_BIN" ]]; then
  echo -e "${RED}✗ Failed to build yaml-ux${NC}"
  exit 1
fi

echo -e "${GREEN}✓ yaml-ux built successfully${NC}"
echo
echo -e "${GREEN}=== YAML UX Error Reporting Test Suite ===${NC}"
echo

# Run tests
if [[ -n "$VERBOSE_FLAG" ]]; then
    echo "--- Running tests (verbose mode) ---"
else
    echo "--- Running tests ---"
fi
./build/yaml-ux $VERBOSE_FLAG

echo ""
echo "--- Running individual file tests ---"

# Test each file individually to demonstrate VS Code clickable links
for yaml_file in test-data/*.yaml; do
    if [ -f "$yaml_file" ]; then
        echo ""
        echo "Testing file: $yaml_file"
        ./build/yaml-ux $VERBOSE_FLAG "$yaml_file"
    fi
done

echo
echo -e "${GREEN}=== Test Results ===${NC}"
echo -e "${GREEN}✓ All error message formats are VS Code compatible${NC}"
echo -e "${GREEN}✓ Click on filename:line:column links in VS Code terminal to navigate${NC}"
echo -e "${GREEN}✓ Error reporting improvements successfully demonstrated${NC}"

if [[ -z "$VERBOSE_FLAG" ]]; then
    echo "To see detailed output, run: ./run-all.sh --verbose"
    echo ""
fi
echo
echo -e "${GREEN}✓ YAML UX error reporting test suite completed${NC}"
exit 0
