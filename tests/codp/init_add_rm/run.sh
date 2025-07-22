#!/bin/bash
set -euo pipefail

# Test the new init/add/rm subcommands using yaml-cmp

# Setup
TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$TEST_DIR/../../.." && pwd)"

# Determine toolchain directory based on COD_TEST_TOOLCHAIN environment variable
COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"

if [[ "$COD_TEST_TOOLCHAIN" == "build" ]]; then
    TOOLCHAIN_DIR="$PROJECT_ROOT/build"
elif [[ "$COD_TEST_TOOLCHAIN" == "built" ]]; then
    TOOLCHAIN_DIR="$PROJECT_ROOT/built"
else
    echo "COD_TEST_TOOLCHAIN must be 'build' or 'built', got: $COD_TEST_TOOLCHAIN" >&2
    exit 1
fi

# Ensure codp and yaml-cmp are in PATH
export PATH="$TOOLCHAIN_DIR/bin:$PATH"
export PATH="$PROJECT_ROOT/tests/yaml-cmp/build:$PATH"

# Create local temporary test directory
TMP_DIR="$TEST_DIR/tmp"
cleanup() {
    if [[ -d "$TMP_DIR" ]]; then
        rm -rf "$TMP_DIR"
    fi
}

# Clean up only on successful exit (EXIT trap will be cleared on error)
trap cleanup EXIT

# Recreate fresh tmp directory
cleanup || true
mkdir -p "$TMP_DIR"

# Ensure codp is available
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

echo "Testing codp init/add/rm subcommands with yaml-cmp validation..."

# Test 1: init command
echo "  Testing init..."
cd "$TMP_DIR"

# Run init with fixed UUID for testing
codp init --uuid "11111111-1111-1111-1111-111111111111" testpkg "https://github.com/test/testpkg.git" main dev

# Validate with yaml-cmp using static expected file
yaml-cmp "$TEST_DIR/expected/init.yaml" CodProject.yaml || {
    echo "ERROR: init test failed - YAML mismatch"
    echo "Expected:"
    cat "$TEST_DIR/expected/init.yaml"
    echo "Actual:"
    cat CodProject.yaml
    exit 1
}

echo "  ✓ init test passed"

# Test 2: add command with UUID
echo "  Testing add with UUID..."
codp add --uuid "22222222-2222-2222-2222-222222222222" "https://github.com/test/dep.git" main feature

# Validate with yaml-cmp using static expected file
yaml-cmp "$TEST_DIR/expected/add_uuid.yaml" CodProject.yaml || {
    echo "ERROR: add test failed - YAML mismatch"
    echo "Expected:"
    cat "$TEST_DIR/expected/add_uuid.yaml"
    echo "Actual:"
    cat CodProject.yaml
    exit 1
}

echo "  ✓ add with UUID test passed"

# Test 3: add command with local path
echo "  Testing add with local path..."
mkdir -p local_dep
printf 'uuid: "33333333-3333-3333-3333-333333333333"\nname: localdep\nrepo_url: https://github.com/local/dep.git\nbranches:\n  - main\n' > local_dep/CodProject.yaml

codp add "local_dep" main

# Validate with yaml-cmp using static expected file
yaml-cmp "$TEST_DIR/expected/add_local.yaml" CodProject.yaml || {
    echo "ERROR: add local test failed - YAML mismatch"
    echo "Expected:"
    cat "$TEST_DIR/expected/add_local.yaml"
    echo "Actual:"
    cat CodProject.yaml
    exit 1
}

echo "  ✓ add with local path test passed"

# Test 4: rm command by UUID
echo "  Testing rm by UUID..."
codp rm "22222222-2222-2222-2222-222222222222"

# Validate with yaml-cmp using static expected file
yaml-cmp "$TEST_DIR/expected/rm_uuid.yaml" CodProject.yaml || {
    echo "ERROR: rm by UUID test failed - YAML mismatch"
    echo "Expected:"
    cat "$TEST_DIR/expected/rm_uuid.yaml"
    echo "Actual:"
    cat CodProject.yaml
    exit 1
}

echo "  ✓ rm by UUID test passed"

# Test 5: rm command by name
echo "  Testing rm by name..."

# Add a named dependency first
printf 'uuid: "44444444-4444-4444-4444-444444444444"\nname: testdep\nrepo_url: https://github.com/test/dep2.git\nbranches:\n  - main\n' > local_dep2/CodProject.yaml

codp add "local_dep2" main

# Now remove by name
codp rm "testdep"

# Validate with yaml-cmp using static expected file
yaml-cmp "$TEST_DIR/expected/rm_name.yaml" CodProject.yaml || {
    echo "ERROR: rm by name test failed - YAML mismatch"
    echo "Expected:"
    cat "$TEST_DIR/expected/rm_name.yaml"
    echo "Actual:"
    cat CodProject.yaml
    exit 1
}

echo "  ✓ rm by name test passed"

# Test 6: branches validation
echo "  Testing branches validation..."
cd "$TMP_DIR"
mkdir -p test_no_branches
cd test_no_branches

# Create invalid project without branches
cat > CodProject.yaml << 'EOF'
uuid: "11111111-1111-1111-1111-111111111111"
name: "test"
repo_url: "https://test.com"
EOF

if codp solve 2>/dev/null; then
    echo "ERROR: should fail when branches are missing"
    exit 1
fi

echo "  ✓ branches validation test passed"

echo "All init/add/rm tests passed!"
