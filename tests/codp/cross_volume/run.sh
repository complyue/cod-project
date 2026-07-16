#!/usr/bin/env bash

set -euo pipefail

case "$(uname -s)" in
  MINGW*|MSYS*) ;;
  *)
    echo "cross_volume test skipped outside Windows"
    exit 0
    ;;
esac

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
TOOLCHAIN_DIR="$PROJECT_ROOT/${COD_TEST_TOOLCHAIN:-build}"
export PATH="$TOOLCHAIN_DIR/bin:$PATH"

WORK_ROOT=$(mktemp -d "$SCRIPT_DIR/.tmp-cross-volume.XXXXXX")
TEMP_ROOT=$(cygpath -u "${TEMP:-${TMP:-C:/Temp}}")
DEP_ROOT=$(mktemp -d "$TEMP_ROOT/codp-cross-volume.XXXXXX")
trap 'rm -rf "$WORK_ROOT" "$DEP_ROOT"' EXIT

PROJECT_DRIVE=$(cygpath -w "$WORK_ROOT" | cut -c 1-2)
DEP_DRIVE=$(cygpath -w "$DEP_ROOT" | cut -c 1-2)
if [[ "${PROJECT_DRIVE,,}" == "${DEP_DRIVE,,}" ]]; then
  echo "cross_volume test skipped because no second Windows volume is available"
  exit 0
fi

PROJECT_DIR="$WORK_ROOT/project"
DEP_DIR="$DEP_ROOT/dependency"
mkdir -p "$PROJECT_DIR" "$DEP_DIR"
DEP_MANIFEST_PATH=$(cygpath -m "$DEP_DIR")

cat >"$DEP_DIR/CodProject.yaml" <<'EOF'
uuid: "77777777-7777-7777-7777-777777777777"
name: "cross-volume-dependency"
repo_url: "/cross/volume/dependency"
branches: [main]
EOF

cat >"$PROJECT_DIR/CodProject.yaml" <<EOF
uuid: "66666666-6666-6666-6666-666666666666"
name: "cross-volume-root"
repo_url: "/cross/volume/root"
branches: [main]
deps:
  - uuid: "77777777-7777-7777-7777-777777777777"
    name: "cross-volume-dependency"
    repo_url: "/cross/volume/dependency"
    branches: [main]
    path: "$DEP_MANIFEST_PATH"
EOF

codp solve --project "$PROJECT_DIR"

if ! grep -Fq "$DEP_MANIFEST_PATH" "$PROJECT_DIR/CodManifest.yaml"; then
  echo "codp did not preserve the absolute cross-volume dependency path" >&2
  exit 1
fi

echo "cross_volume test passed"
