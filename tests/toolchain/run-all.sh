#!/usr/bin/env bash
# Exercise the bundled Linux compiler and runtime without explicit library paths.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=tests/test-utils.sh
source "$SCRIPT_DIR/../test-utils.sh"
TOOLCHAIN_DIR="$COD_PROJECT_SRC_ROOT/$COD_TEST_TOOLCHAIN"
unset LD_LIBRARY_PATH LIBRARY_PATH CPATH CPLUS_INCLUDE_PATH C_INCLUDE_PATH
SMOKE_DIR=$(mktemp -d)
trap 'rm -rf "$SMOKE_DIR"' EXIT

cat > "$SMOKE_DIR/smoke.c" <<'C'
#include <stdio.h>
int main(void) { return puts("C toolchain OK") < 0; }
C
"$TOOLCHAIN_DIR/bin/clang" "$SMOKE_DIR/smoke.c" -o "$SMOKE_DIR/c-smoke"
"$SMOKE_DIR/c-smoke"

cat > "$SMOKE_DIR/smoke.cc" <<'CPP'
#include <format>
#include <iostream>
#include <stdexcept>
#include <string_view>
#ifndef _LIBCPP_VERSION
#error The bundled C++ compiler must select libc++ by default.
#endif
int main() {
  try {
    throw std::runtime_error(std::format("C++ runtime {}", 42));
  } catch (const std::exception &error) {
    std::cout << error.what() << '\n';
    return std::string_view(error.what()) == "C++ runtime 42" ? 0 : 1;
  }
}
CPP
"$TOOLCHAIN_DIR/bin/clang++" -std=c++20 -Werror -c "$SMOKE_DIR/smoke.cc" -o "$SMOKE_DIR/smoke.o"
"$TOOLCHAIN_DIR/bin/clang++" "$SMOKE_DIR/smoke.o" -o "$SMOKE_DIR/cxx-smoke"
"$SMOKE_DIR/cxx-smoke"

LOADER_OUTPUT=$(ldd "$SMOKE_DIR/cxx-smoke")
for RUNTIME in libc++.so libc++abi.so libunwind.so; do
  RUNTIME_PATH=$(awk -v name="$RUNTIME" 'index($1, name) == 1 {print $3}' <<< "$LOADER_OUTPUT")
  if [[ -z "$RUNTIME_PATH" || "$RUNTIME_PATH" == not ]]; then
    echo "$RUNTIME was not resolved by the loader" >&2
    exit 1
  fi
  case "$(realpath "$RUNTIME_PATH")" in
    "$(realpath "$TOOLCHAIN_DIR/lib")"/*) ;;
    *) echo "$RUNTIME was not loaded from the selected toolchain: $RUNTIME_PATH" >&2; exit 1 ;;
  esac
done
echo "Bundled Linux toolchain smoke checks passed."
