#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
COD_TEST_TOOLCHAIN="${COD_TEST_TOOLCHAIN:-build}"
TOOLCHAIN_DIR="$PROJECT_ROOT/$COD_TEST_TOOLCHAIN"
export PATH="$TOOLCHAIN_DIR/bin:$PATH"

WORK_ROOT=$(mktemp -d "$SCRIPT_DIR/.tmp-update.XXXXXX")
WORK_DIR="$WORK_ROOT/path&shell"
mkdir -p "$WORK_DIR"
cleanup() {
  if ! rm -rf "$WORK_ROOT"; then
    sleep 1
    rm -rf "$WORK_ROOT"
  fi
}
trap cleanup EXIT

mkdir -p "$WORK_DIR/home" "$WORK_DIR/project"
export HOME="$WORK_DIR/home"
case "$(uname -s)" in
  MINGW*|MSYS*)
    USERPROFILE=$(cygpath -w "$WORK_DIR/home")
    export USERPROFILE
    ;;
esac

SOURCE_REPO="$WORK_DIR/source"
git init -q -b main "$SOURCE_REPO"
git -C "$SOURCE_REPO" config user.name "CoD Test"
git -C "$SOURCE_REPO" config user.email "cod-test@example.invalid"
printf 'test\n' >"$SOURCE_REPO/content.txt"
git -C "$SOURCE_REPO" add content.txt
git -C "$SOURCE_REPO" commit -q -m "initial"

REPO_URL="https://example.invalid/repo.git"
REPO_KEY="https___example_invalid_repo_git"
CACHE_ROOT="$WORK_DIR/home/.cod/pkgs/repos"
CANONICAL_CACHE="$CACHE_ROOT/$REPO_KEY.git"
mkdir -p "$CACHE_ROOT"
git clone -q --mirror "$SOURCE_REPO" "$CANONICAL_CACHE"
git config --global url."$SOURCE_REPO".insteadOf "$REPO_URL"

cp "$SCRIPT_DIR/CodManifest.yaml" "$WORK_DIR/project/CodManifest.yaml"
(cd "$WORK_DIR" && codp update --project "$WORK_DIR/project")

if [[ -e "$CACHE_ROOT/$REPO_KEY" ]]; then
  echo "codp update created a duplicate cache without the .git suffix" >&2
  exit 1
fi

EXPECTED_COMMIT=$(git -C "$SOURCE_REPO" rev-parse HEAD)
if ! grep -q "$EXPECTED_COMMIT" "$WORK_DIR/project/CodManifest.yaml"; then
  echo "codp update did not record the resolved commit" >&2
  exit 1
fi

cp "$SCRIPT_DIR/CodManifest.yaml" "$WORK_DIR/project/CodManifest.yaml"
git --git-dir="$CANONICAL_CACHE" remote set-url origin "$WORK_DIR/missing-origin"
UPDATE_OUTPUT=$(cd "$WORK_DIR" && codp update --project "$WORK_DIR/project" 2>&1)
if ! grep -q "Warning: Failed to update" <<<"$UPDATE_OUTPUT"; then
  echo "codp update did not report a failed fetch" >&2
  exit 1
fi
if grep -q "commit:" "$WORK_DIR/project/CodManifest.yaml"; then
  echo "codp update recorded a stale commit after fetch failed" >&2
  exit 1
fi

echo "update test passed"
