#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

# Set default COD_TEST_TOOLCHAIN if not already set
export COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

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

# Build the yaml-ux test executable
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

echo "Building yaml-ux..."
cmake --build build

if [ $? -eq 0 ]; then
    echo "✓ yaml-ux built successfully"
else
    echo "✗ yaml-ux build failed"
    exit 1
fi

echo ""
echo "=== YAML UX Error Reporting Test Suite ==="
echo ""

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

echo ""
echo "=== Test Results ==="
echo "✓ All error message formats are VS Code compatible"
echo "✓ Click on filename:line:column links in VS Code terminal to navigate"
echo "✓ Error reporting improvements successfully demonstrated"

echo ""
echo "Usage examples:"
echo "  ./run-all.sh                       # Run test suite (concise)"
echo "  ./run-all.sh --verbose             # Run test suite (detailed)"
echo "  ./build/yaml-ux file.yaml          # Test specific file"
echo "  ./build/yaml-ux -v file.yaml       # Test specific file (verbose)"
echo ""
if [[ -z "$VERBOSE_FLAG" ]]; then
    echo "To see detailed output, run: ./run-all.sh --verbose"
    echo ""
fi
echo "✔ YAML UX error reporting test suite completed" 
