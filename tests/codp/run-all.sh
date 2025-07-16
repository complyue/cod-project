#!/usr/bin/env bash
# run-all.sh – orchestrate CoDP shell tests, making sure yaml-cmp is built first.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Ensure yaml-cmp is built and passing its self test
"$PROJECT_ROOT/tests/yaml-cmp/run-all.sh"

echo ""
echo ""
echo "🏃  Running codp shell tests..."

# Add yaml-cmp build dir to PATH so sub-tests can find it
export PATH="$PROJECT_ROOT/tests/yaml-cmp/build:$PATH"

# Run each sub-test script (directories containing run.sh)
for td in "$SCRIPT_DIR"/*/; do
  if [[ -f "$td/run.sh" ]]; then
    echo "→ $(basename "$td")"
    (cd "$td" && ./run.sh)
  fi
done

echo "✔ All codp tests passed."
