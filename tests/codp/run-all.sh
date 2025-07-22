#!/usr/bin/env bash
# run-all.sh – CoD Package Manager shell integration tests
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Source test utilities and setup toolchain
source "$SCRIPT_DIR/../test-utils.sh"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo "🔧 Using toolchain mode: $COD_TEST_TOOLCHAIN"

# Ensure yaml-cmp is built using shared utility functions
YAML_CMP_DIR="$PROJECT_ROOT/tests/yaml-cmp"
YAML_CMP_BUILD_DIR="$YAML_CMP_DIR/build"

setup_build_dir "$YAML_CMP_BUILD_DIR" "$YAML_CMP_DIR"
cmake --build "$YAML_CMP_BUILD_DIR"

# Add yaml-cmp build dir to PATH
export PATH="$YAML_CMP_BUILD_DIR:$PATH"

echo -e "${GREEN}✓ yaml-cmp built successfully${NC}"
echo

# Run sub-tests
echo -e "${GREEN}=== CoD Package Manager Shell Integration Tests ===${NC}"
for td in "$SCRIPT_DIR"/*/; do
  if [[ -f "$td/run.sh" ]]; then
    echo -e "${GREEN}→ $(basename "$td")${NC}"
    (cd "$td" && ./run.sh)
  fi
done

echo
echo -e "${GREEN}✓ All CoD Package Manager tests passed successfully${NC}"
exit 0
