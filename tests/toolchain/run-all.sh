#!/usr/bin/env bash
# Exercise the bundled native compiler and runtime without library-path overrides.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=tests/test-utils.sh
source "$SCRIPT_DIR/../test-utils.sh"
TOOLCHAIN_DIR="$COD_PROJECT_SRC_ROOT/$COD_TEST_TOOLCHAIN"
unset LD_LIBRARY_PATH LIBRARY_PATH CPATH CPLUS_INCLUDE_PATH C_INCLUDE_PATH
unset DYLD_LIBRARY_PATH DYLD_FALLBACK_LIBRARY_PATH DYLD_INSERT_LIBRARIES
SMOKE_DIR=$(mktemp -d)
trap 'rm -rf "$SMOKE_DIR"' EXIT

cat > "$SMOKE_DIR/smoke.c" <<'C'
#include <stdio.h>
int main(void) { return puts("C toolchain OK") < 0; }
C
"$TOOLCHAIN_DIR/bin/clang" "$SMOKE_DIR/smoke.c" -o "$SMOKE_DIR/c-smoke"
"$SMOKE_DIR/c-smoke"

cat > "$SMOKE_DIR/smoke.cc" <<'CPP'
#include <cxxabi.h>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#ifndef _LIBCPP_VERSION
#error The bundled C++ compiler must select libc++ by default.
#endif
extern "C" int runtime_check() {
  std::exception_ptr pending;
  try {
    throw std::runtime_error(std::format("C++ runtime {}", 42));
  } catch (...) {
    pending = std::current_exception();
  }
  try {
    std::rethrow_exception(pending);
  } catch (const std::exception &error) {
    std::cout << error.what() << '\n';
    if (std::string_view(error.what()) != "C++ runtime 42") return 1;
  }
  int status = 0;
  std::unique_ptr<char, decltype(&std::free)> name(
      abi::__cxa_demangle(typeid(std::runtime_error).name(), nullptr, nullptr, &status),
      &std::free);
  if (status != 0 || !name) return 2;
  std::cout << name.get() << '\n';
  return 0;
}
#ifndef COD_SMOKE_LIBRARY
int main() { return runtime_check(); }
#endif
CPP
"$TOOLCHAIN_DIR/bin/clang++" -std=c++20 -Werror -c "$SMOKE_DIR/smoke.cc" -o "$SMOKE_DIR/smoke.o"
"$TOOLCHAIN_DIR/bin/clang++" "$SMOKE_DIR/smoke.o" -o "$SMOKE_DIR/cxx-smoke"
"$SMOKE_DIR/cxx-smoke"

case "$(uname -s)" in
Linux)
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
;;
Darwin)
check_macos_runtime() {
  local mode="$1"
  shift
  local trace="$SMOKE_DIR/$mode-loader.log"
  if ! DYLD_PRINT_LIBRARIES=1 "$@" 2> "$trace"; then
    cat "$trace" >&2
    return 1
  fi
  python3 - "$TOOLCHAIN_DIR" "$trace" "$mode" "$1" <<'PY'
import pathlib
import re
import subprocess
import sys

prefix, trace, mode, executable = sys.argv[1:]
libdir = pathlib.Path(prefix, "lib").resolve()
required = {"libc++", "libc++abi"}
if mode == "asan":
    required.add("libclang_rt.asan_osx_dynamic")
found = set()
for line in pathlib.Path(trace).read_text().splitlines():
    match = re.search(r"(/.*\.dylib)\s*$", line)
    if not match:
        continue
    path = pathlib.Path(match.group(1)).resolve()
    for name in required:
        if path.name.startswith(name + "."):
            if libdir in path.parents:
                found.add(name)
                print(f"{mode}: {path}")
            elif path.parent != pathlib.Path("/usr/lib"):
                sys.exit(f"{name} loaded outside the selected toolchain: {path}")
if required - found:
    sys.exit(f"Bundled runtimes missing from {mode} loader trace: {required - found}")
commands = subprocess.check_output(["otool", "-l", executable], text=True)
if "LC_DYLD_CHAINED_FIXUPS" in commands:
    sys.exit(f"Chained fixups enabled in {executable}")
PY
}

check_macos_runtime cxx "$SMOKE_DIR/cxx-smoke"
"$TOOLCHAIN_DIR/bin/clang++" -std=c++20 -g -fsanitize=address \
  "$SMOKE_DIR/smoke.cc" -o "$SMOKE_DIR/cxx-asan"
check_macos_runtime asan "$SMOKE_DIR/cxx-asan"

# Load the system libc++ first, then exercise the bundled runtime through a dylib.
cat > "$SMOKE_DIR/coexist.c" <<'C'
#include <dlfcn.h>
#include <stdio.h>
int main(int argc, char **argv) {
  if (argc != 2) return 1;
  if (!dlopen("/usr/lib/libc++.1.dylib", RTLD_NOW | RTLD_GLOBAL)) return 2;
  void *plugin = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
  if (!plugin) { fprintf(stderr, "%s\n", dlerror()); return 3; }
  int (*check)(void) = (int (*)(void))dlsym(plugin, "runtime_check");
  return check ? check() : 4;
}
C
"$TOOLCHAIN_DIR/bin/clang++" -std=c++20 -DCOD_SMOKE_LIBRARY -dynamiclib \
  "$SMOKE_DIR/smoke.cc" -o "$SMOKE_DIR/runtime.dylib"
"$TOOLCHAIN_DIR/bin/clang" "$SMOKE_DIR/coexist.c" -o "$SMOKE_DIR/coexist"
check_macos_runtime coexist "$SMOKE_DIR/coexist" "$SMOKE_DIR/runtime.dylib"
echo "Bundled macOS toolchain smoke checks passed."
;;
*) echo "Unsupported toolchain smoke host: $(uname -s)" >&2; exit 1 ;;
esac
